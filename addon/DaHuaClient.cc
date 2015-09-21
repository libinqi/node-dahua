#include "DaHuaClient.h"
#include "H264Decoder.h"
#include <iostream>
#include <string>
#include <stdio.h>

using namespace v8;

Persistent<Function> DaHuaClient::constructor;

static uv_async_t s_async = { 0 };
static Persistent<Function> cbRealDataCallback;
static uv_async_t s_async2 = { 0 };
static Persistent<Function> cbCarNumberCallback;
static DEV_EVENT_TRAFFIC_TRAFFICCAR_INFO stTrafficCar;
static Persistent<Function> cbDisConnectCallback;
static char* buf;
static int size = 0;
static H264Decoder* decoder;

DaHuaClient::DaHuaClient(){
}

DaHuaClient::~DaHuaClient() {
}

void DaHuaClient::Init(Handle<Object> exports) {
	Isolate* isolate = Isolate::GetCurrent();

	// Prepare constructor template
	Local<FunctionTemplate> tpl = FunctionTemplate::New(isolate, New);
	tpl->SetClassName(String::NewFromUtf8(isolate, "DaHuaClient"));
	tpl->InstanceTemplate()->SetInternalFieldCount(1);

	// Prototype
	NODE_SET_PROTOTYPE_METHOD(tpl, "Init_Device", Init_Device);
	NODE_SET_PROTOTYPE_METHOD(tpl, "Login_Device", Login_Device);
	NODE_SET_PROTOTYPE_METHOD(tpl, "StartRealPlay", StartRealPlay);
	NODE_SET_PROTOTYPE_METHOD(tpl, "StopRealPlay", StopRealPlay);
	NODE_SET_PROTOTYPE_METHOD(tpl, "SubscribeCarNumber", SubscribeCarNumber);


	constructor.Reset(isolate, tpl->GetFunction());
	exports->Set(String::NewFromUtf8(isolate, "DaHuaClient"),
		tpl->GetFunction());
}

void DaHuaClient::New(const FunctionCallbackInfo<Value>& args) {
	Isolate* isolate = Isolate::GetCurrent();
	HandleScope scope(isolate);

	if (args.IsConstructCall()) {
		// Invoked as constructor: `new MyObject(...)`
		DaHuaClient* obj = new DaHuaClient();
		obj->Wrap(args.This());
		args.GetReturnValue().Set(args.This());
	}
	else {
		// Invoked as plain function `MyObject(...)`, turn into construct call.
		const int argc = 1;
		Local<Value> argv[argc] = { args[0] };
		Local<Function> cons = Local<Function>::New(isolate, constructor);
		args.GetReturnValue().Set(cons->NewInstance(argc, argv));
	}
}

void uv_close(uv_handle_t* handle){
	Isolate* isolate = Isolate::GetCurrent();
	HandleScope scope(isolate);

	if (!cbDisConnectCallback.IsEmpty()){
		Local<Function> cb = Local<Function>::New(isolate, cbDisConnectCallback);

		const unsigned argc = 1;
		Local<Value> argv[argc] = {
			Number::New(isolate, 0)
		};

		cb->Call(isolate->GetCurrentContext()->Global(), argc, argv);
	}
}

void CALLBACK DisConnectFunc(LONG lLoginID, char *pchDVRIP, LONG nDVRPort, DWORD dwUser)
{
	CLIENT_Logout(lLoginID);
	uv_close((uv_handle_t*)&s_async, uv_close);
	uv_close((uv_handle_t*)&s_async2, uv_close);
	delete decoder;
}

void DaHuaClient::Init_Device(const FunctionCallbackInfo<Value>& args) {
	Isolate* isolate = Isolate::GetCurrent();
	HandleScope scope(isolate);

	cbDisConnectCallback.Reset(isolate, Persistent<Function>::Persistent(isolate, Local<Function>::Cast(args[0])));

	bool result = CLIENT_Init(DisConnectFunc, 0);
	args.GetReturnValue().Set(Boolean::New(isolate, result));
}

static char* TO_CHAR(Handle<Value> val) {
	String::Utf8Value utf8(val->ToString());

	int len = utf8.length() + 1;
	char *str = (char *)calloc(sizeof(char), len);
	strncpy(str, *utf8, len);

	return str;
}


static std::string UTF8ToGBK(const std::string& strUTF8)
{
	int len = MultiByteToWideChar(CP_UTF8, 0, strUTF8.c_str(), -1, NULL, 0);
	wchar_t * wszGBK = new WCHAR[len + 1]; memset(wszGBK, 0, len * 2 + 2);
	MultiByteToWideChar(CP_UTF8, 0, (char*)strUTF8.c_str(), -1, wszGBK, len);

	len = WideCharToMultiByte(CP_ACP, 0, wszGBK, -1, NULL, 0, NULL, NULL);
	char *szGBK = new char[len + 1];
	memset(szGBK, 0, len + 1);
	WideCharToMultiByte(CP_ACP, 0, wszGBK, -1, szGBK, len, NULL, NULL); //strUTF8 = szGBK; 
	std::string strTemp(szGBK);
	delete[]szGBK;
	delete[]wszGBK;
	return strTemp;
}

static std::string GBKToUTF8(const std::string& strGBK)
{
	static std::string strOutUTF8 = "";
	WCHAR * str1;
	int n = MultiByteToWideChar(CP_ACP, 0, strGBK.c_str(), -1, NULL, 0);
	str1 = new WCHAR[n];
	MultiByteToWideChar(CP_ACP, 0, strGBK.c_str(), -1, str1, n); n = WideCharToMultiByte(CP_UTF8, 0, str1, -1, NULL, 0, NULL, NULL);
	char * str2 = new char[n];
	WideCharToMultiByte(CP_UTF8, 0, str1, -1, str2, n, NULL, NULL);
	strOutUTF8 = str2;
	delete[]str1;
	str1 = NULL;
	delete[]str2;
	str2 = NULL;
	return strOutUTF8;
}

//static std::string GBKToUTF8(char *pText, int pLen)
//{
//	char buf[4];
//	memset(buf, 0, 4);
//
//	std::string(pOut);
//	pOut.clear();
//
//	int i = 0;
//	while (i < pLen)
//	{
//		//如果是英文直接复制就可以  
//		if (pText[i] >= 0)
//		{
//			char asciistr[2] = { 0 };
//			asciistr[0] = (pText[i++]);
//			pOut.append(asciistr);
//		}
//		else
//		{
//			WCHAR pbuffer;
//			MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, pText + i, 2, &pbuffer, 1);
//
//			// 注意 WCHAR高低字的顺序,低字节在前，高字节在后  
//			char* pchar = (char *)pbuffer;
//
//			buf[0] = (0xE0 | ((pchar[1] & 0xF0) >> 4));
//			buf[1] = (0x80 | ((pchar[1] & 0x0F) << 2)) + ((pchar[0] & 0xC0) >> 6);
//			buf[2] = (0x80 | (pchar[0] & 0x3F));
//
//			pOut.append(buf);
//
//			i += 2;
//		}
//	}
//
//	return pOut;
//}

void DaHuaClient::Login_Device(const FunctionCallbackInfo<Value>& args) {
	Isolate* isolate = Isolate::GetCurrent();
	HandleScope scope(isolate);

	char* m_DvrIPAddr = TO_CHAR(args[0]);
	WORD m_DvrPort = args[1]->Int32Value();
	char* m_DvrUserName = TO_CHAR(args[2]);
	char* m_DvrPassword = TO_CHAR(args[3]);
	//char szIP[16] = "61.234.234.164";
	//int nPort = 37777;

	NET_DEVICEINFO stDevInfo = { 0 };
	int error = 0;

	LONG result = CLIENT_Login(m_DvrIPAddr, m_DvrPort, m_DvrUserName, m_DvrPassword, &stDevInfo, &error);
	//LONG result = CLIENT_Login(szIP, nPort, "admin", "admin", &stDevInfo, &error);
	args.GetReturnValue().Set(Number::New(isolate, result));
}

static void rgb24_to_bmp(uint8_t *pRGBBuffer, int width, int height, int bpp)
{
	BITMAPFILEHEADER bmpheader;
	BITMAPINFO bmpinfo;
	FILE *fp;

	bmpheader.bfType = ('M' << 8) | 'B';
	bmpheader.bfReserved1 = 0;
	bmpheader.bfReserved2 = 0;
	bmpheader.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
	bmpheader.bfSize = 3 * width*height;

	bmpinfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bmpinfo.bmiHeader.biWidth = width;
	bmpinfo.bmiHeader.biHeight = height;
	bmpinfo.bmiHeader.biPlanes = 1;
	bmpinfo.bmiHeader.biBitCount = bpp;
	bmpinfo.bmiHeader.biCompression = 0;
	bmpinfo.bmiHeader.biSizeImage = 0;
	bmpinfo.bmiHeader.biXPelsPerMeter = 100;
	bmpinfo.bmiHeader.biYPelsPerMeter = 100;
	bmpinfo.bmiHeader.biClrUsed = 0;
	bmpinfo.bmiHeader.biClrImportant = 0;

	memcpy(pRGBBuffer, &bmpheader, sizeof(BITMAPFILEHEADER));
	memcpy(pRGBBuffer + sizeof(BITMAPFILEHEADER), &bmpinfo.bmiHeader, sizeof(BITMAPINFOHEADER));

	fp = fopen("test.bmp", "wb");
	fwrite(pRGBBuffer, (3 * width*height) + (sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER)), 1, fp);
	fclose(fp);
}

void onCallback(uv_async_t* handle, int status){
	Isolate* isolate = Isolate::GetCurrent();
	HandleScope scope(isolate);

	if (!cbRealDataCallback.IsEmpty()){
		Local<Function> cb = Local<Function>::New(isolate, cbRealDataCallback);

		AVFrame* frame = decoder->Decoder(size, buf);

		if (frame)
		{
			//rgb24_to_bmp(frame->data[0], decoder->width, decoder->height, 24);
			int bmpSize = (3 * decoder->width * decoder->height) + (sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER));
			const unsigned argc = 2;
			Local<Value> argv[argc] = {
				Number::New(isolate, bmpSize),
				node::Buffer::New(isolate, (char*)frame->data[0], bmpSize)
			};

			cb->Call(isolate->GetCurrentContext()->Global(), argc, argv);
		}
	}
}


void  CALLBACK cbRealData(LLONG lRealHandle, DWORD dwDataType, BYTE *pBuffer, DWORD dwBufSize, LDWORD dwUser){
	buf = (char*)pBuffer;
	size = (int)dwBufSize;
	uv_async_send(&s_async);
}

void DaHuaClient::StartRealPlay(const FunctionCallbackInfo<Value>& args) {
	Isolate* isolate = Isolate::GetCurrent();
	HandleScope scope(isolate);
	LONG loginId = args[0]->Int32Value();
	LONG realHandle = CLIENT_RealPlay(loginId, 0, 0);

	if (realHandle)
	{
		//DaHuaClient* client = ObjectWrap::Unwrap<DaHuaClient>(args.Holder());
		//client->cbRealDataCallback.Reset(isolate, Persistent<Function>::Persistent(isolate, cb));
		//cbRealDataCallback.Reset(isolate, Persistent<Function>::Persistent(isolate, cb));
		int width = args[1]->Int32Value();
		int height = args[2]->Int32Value();

		cbRealDataCallback.Reset(isolate, Persistent<Function>::Persistent(isolate, Local<Function>::Cast(args[3])));

		decoder = new H264Decoder(width, height);
		BOOL result = CLIENT_SetRealDataCallBack(realHandle, cbRealData, 0);

		if (result)
		{
			uv_async_init(uv_default_loop(), &s_async, (uv_async_cb)onCallback);
			args.GetReturnValue().Set(Number::New(isolate, realHandle));
		}
		else{
			uv_async_init(uv_default_loop(), &s_async, (uv_async_cb)onCallback);
			args.GetReturnValue().Set(Number::New(isolate, 0));
		}
	}
	else
	{
		args.GetReturnValue().Set(Boolean::New(isolate, false));
	}
}

void DaHuaClient::StopRealPlay(const FunctionCallbackInfo<Value>& args) {
	Isolate* isolate = Isolate::GetCurrent();
	HandleScope scope(isolate);
	LONG realHandle = args[0]->Int32Value();

	BOOL result = CLIENT_StopRealPlay(realHandle);

	uv_close((uv_handle_t*)&s_async, NULL);
	delete decoder;

	args.GetReturnValue().Set(Boolean::New(isolate, result));
}

void onCallback2(uv_async_t* handle, int status){
	Isolate* isolate = Isolate::GetCurrent();
	HandleScope scope(isolate);

	if (!cbRealDataCallback.IsEmpty()){
		Local<Function> cb = Local<Function>::New(isolate, cbCarNumberCallback);

		std::string carNumber = GBKToUTF8(stTrafficCar.szPlateNumber);

		const unsigned argc = 1;
		Local<Value> argv[argc] = {
			String::NewFromUtf8(isolate, (char*)carNumber.c_str())
		};

		cb->Call(isolate->GetCurrentContext()->Global(), argc, argv);
	}
}

int CALLBACK cbAnalyzerData(LLONG lAnalyzerHandle, DWORD dwAlarmType, void* pAlarmInfo, BYTE *pBuffer, DWORD dwBufSize, LDWORD dwUser, int nSequence, void *reserved){
	if (dwAlarmType == EVENT_IVS_TRAFFIC_RUNREDLIGHT)
	{
		DEV_EVENT_TRAFFIC_RUNREDLIGHT_INFO* pInfo = (DEV_EVENT_TRAFFIC_RUNREDLIGHT_INFO*)pAlarmInfo;
		stTrafficCar = pInfo->stTrafficCar;
	}
	else if (dwAlarmType == EVENT_IVS_TRAFFICJUNCTION)
	{
		DEV_EVENT_TRAFFICJUNCTION_INFO* pInfo = (DEV_EVENT_TRAFFICJUNCTION_INFO*)pAlarmInfo;
		stTrafficCar = pInfo->stTrafficCar;
	}
	else if (dwAlarmType == EVENT_IVS_TRAFFIC_TURNLEFT)
	{
		DEV_EVENT_TRAFFIC_TURNLEFT_INFO* pInfo = (DEV_EVENT_TRAFFIC_TURNLEFT_INFO*)pAlarmInfo;
		stTrafficCar = pInfo->stTrafficCar;
	}
	else if (dwAlarmType == EVENT_IVS_TRAFFIC_TURNRIGHT)
	{
		DEV_EVENT_TRAFFIC_TURNRIGHT_INFO* pInfo = (DEV_EVENT_TRAFFIC_TURNRIGHT_INFO*)pAlarmInfo;
		stTrafficCar = pInfo->stTrafficCar;
	}
	else if (dwAlarmType == EVENT_IVS_TRAFFIC_OVERSPEED)
	{
		DEV_EVENT_TRAFFIC_OVERSPEED_INFO* pInfo = (DEV_EVENT_TRAFFIC_OVERSPEED_INFO*)pAlarmInfo;
		stTrafficCar = pInfo->stTrafficCar;
	}
	else if (dwAlarmType == EVENT_IVS_TRAFFIC_UNDERSPEED)
	{
		DEV_EVENT_TRAFFIC_UNDERSPEED_INFO* pInfo = (DEV_EVENT_TRAFFIC_UNDERSPEED_INFO*)pAlarmInfo;
		stTrafficCar = pInfo->stTrafficCar;
	}
	else if (dwAlarmType == EVENT_IVS_TRAFFIC_MANUALSNAP)
	{
		DEV_EVENT_TRAFFIC_MANUALSNAP_INFO* pInfo = (DEV_EVENT_TRAFFIC_MANUALSNAP_INFO*)pAlarmInfo;
		stTrafficCar = pInfo->stTrafficCar;
	}

	uv_async_send(&s_async2);
	return lAnalyzerHandle;
}

void DaHuaClient::SubscribeCarNumber(const FunctionCallbackInfo<Value>& args) {
	Isolate* isolate = Isolate::GetCurrent();
	HandleScope scope(isolate);
	LONG loginId = args[0]->Int32Value();

	cbCarNumberCallback.Reset(isolate, Persistent<Function>::Persistent(isolate, Local<Function>::Cast(args[1])));

	LONG result = CLIENT_RealLoadPictureEx(loginId, 0, EVENT_IVS_ALL, false, cbAnalyzerData, 0, 0);
	uv_async_init(uv_default_loop(), &s_async2, (uv_async_cb)onCallback2);

	args.GetReturnValue().Set(Number::New(isolate, result));
}