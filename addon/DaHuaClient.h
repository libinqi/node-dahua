#ifndef DaHuaClient_H
#define DaHuaClient_H

#include <uv.h>
#include <node.h>
#include <node_buffer.h>
#include <node_object_wrap.h>
#include "dhnetsdk.h"

class DaHuaClient : public node::ObjectWrap {
public:
	static void Init(v8::Handle<v8::Object> exports);

private:
	explicit DaHuaClient();
	~DaHuaClient();

	static void New(const v8::FunctionCallbackInfo<v8::Value>& args);
	static void Init_Device(const v8::FunctionCallbackInfo<v8::Value>& args);
	static void Login_Device(const v8::FunctionCallbackInfo<v8::Value>& args);
	static void StartRealPlay(const v8::FunctionCallbackInfo<v8::Value>& args);
	static void StopRealPlay(const v8::FunctionCallbackInfo<v8::Value>& args);
	static void SubscribeCarNumber(const v8::FunctionCallbackInfo<v8::Value>& args);
	static v8::Persistent<v8::Function> constructor;
};

#endif