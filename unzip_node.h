/*
    @author Ayoub Serti
    @email  ayb.serti@gmail.com
    @file   unzip_node.h
*/

#ifndef __UNZIP_NODE_H__
#define __UNZIP_NODE_H__

#include <node.h>
#include <node_object_wrap.h>
#include <v8.h>
#include <uv.h>

#include <string>
#include <vector>
#include <iostream>


using namespace std;

class UnZipObject
{
private:
    
    std::string m_filename;
    std::string m_filepath;
    std::string m_target;
    
public:

    enum EventEnums
    {
        eEnd =1,
        eError,
        eProgress,
    };
    
    UnZipObject    (const string inFile, const string& inTarget);
    
    ~UnZipObject();
    void Run(void);
    
    void On(EventEnums inEvent, const v8::Local<v8::Function>& inCallback);
    
//asynchronous callback

class AsyncCallBack
{
private: 
	uv_loop_t* m_UVLoop;
	uv_async_t* m_AsyncHandler;
	UnZipObject* m_Object;
	
	string      m_FileName;
	int         m_percent;
	
	EventEnums  m_EventType;

	struct CallbackData
	{
	    AsyncCallBack* m_AsynchCallback;
	    uv_async_t*    m_AsyncHandler;
	    v8::Isolate*   m_Isolate;
	};
	

public:
	explicit AsyncCallBack(uv_loop_t* inUVLoop,EventEnums inEventType,UnZipObject* inObject ):
		m_UVLoop(inUVLoop)
	{
		m_AsyncHandler = new uv_async_t;
		v8::Isolate* isolate = v8::Isolate::GetCurrent();
		uv_async_init(m_UVLoop, m_AsyncHandler, WakeUpCB);
		uv_unref(reinterpret_cast<uv_handle_t*>(m_AsyncHandler));
		
		CallbackData* data = new CallbackData;
		data->m_AsynchCallback = this;
		data->m_Isolate = isolate;
		data->m_AsyncHandler = m_AsyncHandler;
		m_AsyncHandler->data = data;
		
		m_EventType = inEventType;
		
		m_Object = inObject;
	}
	~AsyncCallBack(){
		
		
		uv_close(reinterpret_cast<uv_handle_t*>(m_AsyncHandler), [](uv_handle_t* hndl){
			CallbackData* cbData = reinterpret_cast<CallbackData*>(hndl->data);
			
			if(cbData!= nullptr)
			{
				delete cbData->m_AsyncHandler;
				delete cbData;	
			}
			
		});
		
	}

	static void WakeUpCB(uv_async_t* inAsyncHandler){
		
		// WakeUpCB will be called into uv loop thread
		CallbackData* cbData = reinterpret_cast<CallbackData*>(inAsyncHandler->data);
		AsyncCallBack* cbObject = cbData->m_AsynchCallback;
		UnZipObject* unzipperObject = cbObject->m_Object;
		
		switch (cbObject->m_EventType)
		{
		    case eEnd:
		        unzipperObject->install_end();
		        break;
		    case eError:
		        unzipperObject->install_error(cbObject->m_FileName);
		        break;
		    case eProgress:
		        unzipperObject->install_progress(cbObject->m_percent,cbObject->m_FileName);
		        break;
		    default:
		        cerr << "Unknow event type" << endl;
		    break;
		}
		
		
		
	};
	void Send(){
		uv_async_send(m_AsyncHandler);
	}
	
	void SetFileName(const string& inFileName){m_FileName = inFileName;}
	void SetPercent (int inPercent) {m_percent = inPercent;}

};
    
private: 
    //function for signal
     void install_end();
     void install_error(const string&  filename);
     void install_progress( int percent, const string& filename);
    
    v8::Persistent<v8::Function, v8::CopyablePersistentTraits<v8::Function> >    m_EndCallback;
    v8::Persistent<v8::Function, v8::CopyablePersistentTraits<v8::Function> >    m_ErrorCallback;
    v8::Persistent<v8::Function, v8::CopyablePersistentTraits<v8::Function> >    m_ProgressCallback;
  
    friend class AsyncCallBack;
    
    //asynchronous callback
    UnZipObject::AsyncCallBack *m_ErrorAsynch, *m_EndAsynch,  *m_ProgressAsynch;
	UnZipObject();

};

class UnZip_Wrap : public  node::ObjectWrap
{
public:
    static void Init(v8::Handle<v8::Object> exports);
    
private:
    
    //internal Object
    UnZipObject* m_Object;
    uv_work_t    m_Request;
    
    explicit UnZip_Wrap( const v8::Local<v8::String>& inFilePath, const v8::Local<v8::String>& inTargetDir);
    UnZip_Wrap();
    ~UnZip_Wrap();

    //v8 constructor function object
    static v8::Persistent<v8::Function> constructor;
    
    // new constructor
    static void New(const v8::FunctionCallbackInfo<v8::Value>& args);
    
    static void Run(const v8::FunctionCallbackInfo<v8::Value>& args);
};



#endif //__UNZIP_NODE_H__