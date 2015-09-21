#include <node.h>
#include "DaHuaClient.h"

using namespace v8;

void InitAll(Handle<Object> exports) {
	DaHuaClient::Init(exports);
}

NODE_MODULE(dahua, InitAll)