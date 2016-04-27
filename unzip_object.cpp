#include "unzip.h"

#include "unzip_node.h"

#include <locale>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <functional>
#include <cstring>

#define dir_delimter '/'
#define MAX_FILENAME 512
#define READ_SIZE 8192


using namespace std;
using v8::Isolate;
using v8::Local;
using v8::String;
using v8::HandleScope;
using v8::Function;
using v8::Value;
using v8::Integer;

std::ifstream::pos_type _filesize(const char* filename)
{
    std::ifstream in(filename, std::ifstream::ate | std::ifstream::binary);
    return in.tellg(); 
}


UnZipObject::UnZipObject(const string inFile, const string& inTarget):
m_filepath(inFile),
m_target(inTarget)
{
  //m_filepath
}

UnZipObject::~UnZipObject() {
  delete m_ErrorAsynch;
  delete m_EndAsynch;
  delete m_ProgressAsynch;
  
  //free up persistent value
  m_ErrorCallback.Reset();
  m_EndCallback.Reset();
  m_ProgressCallback.Reset();
}

void UnZipObject::On(EventEnums inEvent, const Local<Function>& inCallback) {
  if( inEvent == eEnd) {
    m_EndCallback.Reset(Isolate::GetCurrent(),inCallback);
    m_EndAsynch = new AsyncCallBack(uv_default_loop(),eEnd,this);
  } else if (inEvent == eError) {
    m_ErrorCallback.Reset(Isolate::GetCurrent(),inCallback);
    m_ErrorAsynch = new AsyncCallBack(uv_default_loop(),eError,this);
  } else if (inEvent == eProgress) {
    m_ProgressCallback.Reset(Isolate::GetCurrent(),inCallback);
    m_ProgressAsynch = new AsyncCallBack(uv_default_loop(),eProgress,this);
  } else {
    cerr << "Error: invalid" << endl;
  }
}

void UnZipObject::Run(void) {

  int last_percent=0,current_percent=0;
  
    
  if(!m_target.empty())
  {
      if(chdir(m_target.c_str()))
      {
          #ifdef WIN32
             mkdir(m_target.c_str());
        #else
             mkdir (m_target.c_str(),0775);
        #endif
      }
      chdir(m_target.c_str());
  }

  if(m_filepath.empty())
  {
    if(m_ErrorAsynch != nullptr)
    {
        m_ErrorAsynch->SetFileName(m_filepath);
        m_ErrorAsynch->Send();
        return;
    }
  }
  
  // Open the zip file
  unzFile zipfile = unzOpen( m_filepath.c_str() );
  if ( zipfile == NULL )
  {
    if(m_ErrorAsynch != nullptr)
    {
        m_ErrorAsynch->SetFileName(m_filepath);
        m_ErrorAsynch->Send();
        return;
    }
  }

  // Get info about the zip file
  unz_global_info global_info;
  if ( unzGetGlobalInfo( zipfile, &global_info ) != UNZ_OK )
  {
#if DEBUG
    cerr <<  "could not read file global info" << endl; ;
#endif

    unzClose( zipfile );
    if(m_ErrorAsynch != nullptr)
    {
        m_ErrorAsynch->SetFileName(m_filepath);
        m_ErrorAsynch->Send();
    return;
    } 
  }

  // Buffer to hold data read from the zip file.
  char read_buffer[ READ_SIZE ];
  

  // Loop to extract all files
  uLong i;
  uLong bytes_read =0;
  uLong file_length =0;
  unzGoToFirstFile(zipfile);
  for ( i = 0; i < global_info.number_entry; ++i )
  {
      // Get info about current file.
      unz_file_info file_info;
      char filename[ MAX_FILENAME ];
      if ( unzGetCurrentFileInfo(
          zipfile,
          &file_info,
          filename,
          MAX_FILENAME,
          NULL, 0, NULL, 0 ) != UNZ_OK )
      {
          cerr <<  "could not read file info" << endl ;
          unzClose( zipfile );
          return;
      }
      
      file_length+=file_info.uncompressed_size;
      
      unzCloseCurrentFile(zipfile);
      unzGoToNextFile(zipfile);
  }
  
  unzGoToFirstFile(zipfile);
  for ( i = 0; i < global_info.number_entry; ++i )
  {
      // Get info about current file.
      unz_file_info file_info;
      char filename[ MAX_FILENAME ];
      if ( unzGetCurrentFileInfo(
          zipfile,
          &file_info,
          filename,
          MAX_FILENAME,
          NULL, 0, NULL, 0 ) != UNZ_OK )
      {
          cerr <<  "could not read file info" << endl ;
          unzClose( zipfile );
          return;
      }

      // Check if this entry is a directory or file.
      const size_t filename_length = strlen( filename );
      if ( filename[ filename_length-1 ] == dir_delimter )
      {
          // Entry is a directory, so create it.
        #if DEBUG
            cerr <<  "dir:" <<  filename << endl;
        #endif
        
        #ifdef WIN32
             mkdir(filename);
        #else
             mkdir (filename,0775);
        #endif
      }
      else
      {
          // Entry is a file, so extract it.
          #if DEBUG
          cerr <<  "file: " << filename << endl;
          #endif
          if ( unzOpenCurrentFile( zipfile ) != UNZ_OK )
          {
              //printf( "could not open file\n" );
              unzClose( zipfile );
              return ;
          }

          // Open a file to write out the data.
          FILE *out = fopen( filename, "wb" );
          if ( out == NULL )
          {
              //printf( "could not open destination file\n" );
              unzCloseCurrentFile( zipfile );
              unzClose( zipfile );
              return ;
          }

          uint64_t len_read = 0;
          do    
          {
              len_read = unzReadCurrentFile( zipfile, read_buffer, READ_SIZE );
              if ( len_read < 0 )
              {
                  //printf( "error %d\n", error );
                  unzCloseCurrentFile( zipfile );
                  unzClose( zipfile );
                  return ;
              }
              bytes_read+=len_read;
              // Write data to file.
              if ( len_read > 0 )
              {
                  fwrite( read_buffer, len_read, 1, out ); 
                  current_percent = ((double)bytes_read/(double)file_length)*100;
                  
                  if(m_ProgressAsynch != nullptr && current_percent> last_percent)
                  {
                      last_percent = current_percent;
                      m_ProgressAsynch->SetFileName(m_filename);
                      m_ProgressAsynch->SetPercent(current_percent);
                      m_ProgressAsynch->Send();
                  }
              }
              
          } while ( len_read > 0 );

          fclose( out );
      }

      unzCloseCurrentFile( zipfile );

      // Go the the next entry listed in the zip file.
      if ( ( i+1 ) < global_info.number_entry )
      {
          if ( unzGoToNextFile( zipfile ) != UNZ_OK )
          {
              //printf( "cound not read next file\n" );
              unzClose( zipfile );
              return ;
          }
      }
  }

  unzClose( zipfile );
  if(m_EndAsynch!=nullptr)
  {
      m_EndAsynch->SetFileName(m_filename);
      m_EndAsynch->Send();
  }
}

void UnZipObject::install_end() {
  Isolate* isolate=Isolate::GetCurrent();
  HandleScope scope(isolate);
  Local<String> strFileName = String::NewFromUtf8(isolate,m_filename.c_str());
  Local< Value > 	argv[] ={strFileName};

  Local<Function> callback = Local<Function>::New(isolate,m_EndCallback);
  callback->Call(isolate->GetCurrentContext()->Global(),1,argv);
}


void UnZipObject::install_error(const string&  filename) {
  cout << "Error callback " << filename << endl;
}

void UnZipObject::install_progress( int percent, const string& filename) {
  Isolate* isolate=Isolate::GetCurrent();
  HandleScope scope(isolate);
  Local<Integer> valPercent = Integer::New(isolate,(uint32_t)percent);
  Local<String> strFileName = String::NewFromUtf8(isolate,filename.c_str());
  Local< Value > 	argv[] ={valPercent,strFileName};

  Local<Function> callback = Local<Function>::New(isolate,m_ProgressCallback);
  callback->Call(isolate->GetCurrentContext()->Global(),2,argv);
}