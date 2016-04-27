
#include "unzip_node.h"
#include <iostream>

using namespace std;

using namespace v8;

v8::Persistent<v8::Function> UnZip_Wrap::constructor;

UnZip_Wrap::UnZip_Wrap( const Local<String>& inFilePath, const Local<String>& inTargetDir) {
  v8::String::Utf8Value filePathUtf8(inFilePath);
  v8::String::Utf8Value targetDirUtf8(inTargetDir);

  string file_name(*filePathUtf8,filePathUtf8.length());
  string target_dir(*targetDirUtf8,targetDirUtf8.length());

  m_Object = new UnZipObject(file_name, target_dir);
}

UnZip_Wrap::UnZip_Wrap() {
  m_Object = nullptr;
}

UnZip_Wrap::~UnZip_Wrap() {
  if(m_Object != nullptr)
    delete m_Object;
}


void UnZip_Wrap::New(const v8::FunctionCallbackInfo<v8::Value>& args) {
  Isolate* isolate = Isolate::GetCurrent();
  HandleScope scope(isolate);

  if (args.IsConstructCall()) {
    // Invoked as constructor: `new UnZip_Wrap(...)`
    UnZip_Wrap* obj = nullptr;

    if(args.Length()>1) {
      if(args[0]->IsString() && args[1]->IsString()) {
        Local<String> fileName = args[0]->ToString(),folder = args[1]->ToString();
        obj = new UnZip_Wrap(fileName,folder);
      }
    } else {
        //error goes here
    }

    if ( obj != nullptr ) {
      obj->Wrap(args.This());
      args.GetReturnValue().Set(args.This());
    }
  }
}

void UnZip_Wrap::Run(const v8::FunctionCallbackInfo<v8::Value>& args)
{
  UnZip_Wrap* obj = ObjectWrap::Unwrap<UnZip_Wrap>(args.Holder());

  if(obj == nullptr) return;

  Isolate* isolate = Isolate::GetCurrent();
  HandleScope scope(isolate);

   UnZipObject* unrarer = obj->m_Object;
  // obj.run(error,terminate,finished,progress,fail)

  //retrieve Callback functions
  if(args.Length() > 0) {
    if(args[0]->IsFunction()) {
      unrarer->On(UnZipObject::eError,Local<Function>::Cast(args[0]));
    }
    if(args.Length() > 1) {
      if(args[1]->IsFunction()) {
        unrarer->On(UnZipObject::eEnd,Local<Function>::Cast(args[1]));
      }
      if(args.Length() > 2) {
        if(args[2]->IsFunction()) {
          unrarer->On(UnZipObject::eProgress,Local<Function>::Cast(args[2]) );
        }
        
      }
    }
  }
 //increment reference so we keep Unrar_Wrap away from GC; an UnRef must be called after
  obj->Ref();
  obj->m_Request.data = obj;
  //process in a another thread
  uv_queue_work(uv_default_loop(),&obj->m_Request,[](uv_work_t* req)
      {
          UnZip_Wrap*   wrapper = reinterpret_cast<UnZip_Wrap*>(req->data); 
          UnZipObject* xobj = wrapper->m_Object;
          if(xobj != nullptr)
          {
             // xobj->StartIdler();
              xobj->Run();
          }
      },[](uv_work_t* req, int status)
      {
          UnZip_Wrap*   wrapper = reinterpret_cast<UnZip_Wrap*>(req->data); 
          UnZipObject* xobj = wrapper->m_Object;
          if(xobj != nullptr)
          {
              //xobj->StopIdler();
          }
          cerr << "Finish threaded function" << endl;
          wrapper->Unref();
      
      });

  args.GetReturnValue().Set(Undefined(isolate));
}

void  UnZip_Wrap::Init(v8::Handle<v8::Object> exports) {
  Isolate* isolate = Isolate::GetCurrent();

  // Prepare constructor template
  Local<FunctionTemplate> tpl = FunctionTemplate::New(isolate, New);
  tpl->SetClassName(String::NewFromUtf8(isolate, "Unzipper"));
  tpl->InstanceTemplate()->SetInternalFieldCount(1);

  // Prototype
  NODE_SET_PROTOTYPE_METHOD(tpl, "run", Run);

  //add other if exists..
  constructor.Reset(isolate, tpl->GetFunction());
  exports->Set(String::NewFromUtf8(isolate, "Unzipper"), tpl->GetFunction());
}


void init(v8::Handle<v8::Object> exports) {
  UnZip_Wrap::Init(exports);
}

NODE_MODULE(unzipper, init)