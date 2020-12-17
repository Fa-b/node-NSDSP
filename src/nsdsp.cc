#include <napi.h>
extern "C"
{
    #include "nsdsp.h"
}

using namespace Napi;

Napi::Value Enumerate(const Napi::CallbackInfo & info) {
    Napi::Env env = info.Env();
    
    if(info.Length() < 1){
        Napi::TypeError::New(env, "Wrong number of arguments").ThrowAsJavaScriptException();
        return env.Null();
    }
    
    if (!info[0].IsBuffer()) {
        Napi::TypeError::New(env, "Buffer expected").ThrowAsJavaScriptException();
        return env.Null();
    }
    
    Napi::Uint8Array arr = info[0].As<Napi::Uint8Array>();
    NSDSP_ENUM_DATA* metadataValue = reinterpret_cast<NSDSP_ENUM_DATA*>(arr.ArrayBuffer().Data()); 

    return Napi::Number::New(env, NSDSPEnumerate(metadataValue));
}

Napi::Value Connect(const Napi::CallbackInfo & info) {
    Napi::Env env = info.Env();
    NSDSP_CONN_HANDLE handle;
    
    if(info.Length() > 0) {
        
        if (!info[0].IsString()) {
            Napi::TypeError::New(env, "String expected").ThrowAsJavaScriptException();
            return env.Null();
        }
        
        std::string serial = info[0].As<Napi::String>();
        char* chr = &serial[0];
        
        handle = NSDSPConnect(chr);
    } else {
        handle = NSDSPConnect(nullptr);
    }
    
    if(handle == nullptr)
        return Napi::Value::Value();
    
    return Napi::Buffer<NSDSP_CONN_HANDLE>::Copy(env, &handle, 1);
}

Napi::Value Disconnect(const Napi::CallbackInfo & info) {
    Napi::Env env = info.Env();

    
    if(info.Length() < 1){
        Napi::TypeError::New(env, "Wrong number of arguments").ThrowAsJavaScriptException();
        return env.Null();
    }
    
    if (!info[0].IsBuffer()) {
        Napi::TypeError::New(env, "Buffer expected").ThrowAsJavaScriptException();
        return env.Null();
    }
    
    NSDSP_CONN_HANDLE handle = *info[0].As<Napi::Buffer<NSDSP_CONN_HANDLE>>().Data();
    
    if(!NSDSPDisconnect(handle))
        return Napi::Value::Value();
        
    
    return info[0];
}

Napi::Value GetSerial(const Napi::CallbackInfo & info) {
    Napi::Env env = info.Env();

    
    if(info.Length() < 1){
        Napi::TypeError::New(env, "Wrong number of arguments").ThrowAsJavaScriptException();
        return env.Null();
    }
    
    if (!info[0].IsBuffer()) {
        Napi::TypeError::New(env, "Buffer expected").ThrowAsJavaScriptException();
        return env.Null();
    }
    
    NSDSP_CONN_HANDLE handle = *info[0].As<Napi::Buffer<NSDSP_CONN_HANDLE>>().Data();
    
    return Napi::String::New(env, NSDSPGetSerial(handle));
}

Napi::Value GetVersion(const Napi::CallbackInfo & info) {
    Napi::Env env = info.Env();

    
    if(info.Length() < 1){
        Napi::TypeError::New(env, "Wrong number of arguments").ThrowAsJavaScriptException();
        return env.Null();
    }
    
    if (!info[0].IsBuffer()) {
        Napi::TypeError::New(env, "Buffer expected").ThrowAsJavaScriptException();
        return env.Null();
    }
    
    NSDSP_CONN_HANDLE handle = *info[0].As<Napi::Buffer<NSDSP_CONN_HANDLE>>().Data();
    
    return Napi::Number::New(env, NSDSPGetVersion(handle));
}

Napi::Value GetBaudRate(const Napi::CallbackInfo & info) {
    Napi::Env env = info.Env();

    
    if(info.Length() < 1){
        Napi::TypeError::New(env, "Wrong number of arguments").ThrowAsJavaScriptException();
        return env.Null();
    }
    
    if (!info[0].IsBuffer()) {
        Napi::TypeError::New(env, "Buffer expected").ThrowAsJavaScriptException();
        return env.Null();
    }
    
    NSDSP_CONN_HANDLE handle = *info[0].As<Napi::Buffer<NSDSP_CONN_HANDLE>>().Data();
    
    return Napi::Number::New(env, NSDSPGetBaudRate(handle));
}

Napi::Value GetFlowControl(const Napi::CallbackInfo & info) {
    Napi::Env env = info.Env();

    
    if(info.Length() < 1){
        Napi::TypeError::New(env, "Wrong number of arguments").ThrowAsJavaScriptException();
        return env.Null();
    }
    
    if (!info[0].IsBuffer()) {
        Napi::TypeError::New(env, "Buffer expected").ThrowAsJavaScriptException();
        return env.Null();
    }
    
    NSDSP_CONN_HANDLE handle = *info[0].As<Napi::Buffer<NSDSP_CONN_HANDLE>>().Data();
    
    return Napi::Number::New(env, NSDSPGetFlowControl(handle));
}

Napi::Value GetCTS(const Napi::CallbackInfo & info) {
    Napi::Env env = info.Env();

    
    if(info.Length() < 1){
        Napi::TypeError::New(env, "Wrong number of arguments").ThrowAsJavaScriptException();
        return env.Null();
    }
    
    if (!info[0].IsBuffer()) {
        Napi::TypeError::New(env, "Buffer expected").ThrowAsJavaScriptException();
        return env.Null();
    }
    
    NSDSP_CONN_HANDLE handle = *info[0].As<Napi::Buffer<NSDSP_CONN_HANDLE>>().Data();
    
    return Napi::Number::New(env, NSDSPGetCTS(handle));
}

Napi::Value GetRX(const Napi::CallbackInfo & info) {
    Napi::Env env = info.Env();

    
    if(info.Length() < 1){
        Napi::TypeError::New(env, "Wrong number of arguments").ThrowAsJavaScriptException();
        return env.Null();
    }
    
    if (!info[0].IsBuffer()) {
        Napi::TypeError::New(env, "Buffer expected").ThrowAsJavaScriptException();
        return env.Null();
    }
    
    NSDSP_CONN_HANDLE handle = *info[0].As<Napi::Buffer<NSDSP_CONN_HANDLE>>().Data();
    
    return Napi::Number::New(env, NSDSPGetRX(handle));
}

Napi::Value SetMode(const Napi::CallbackInfo & info) {
    Napi::Env env = info.Env();

    
    if(info.Length() < 2){
        Napi::TypeError::New(env, "Wrong number of arguments").ThrowAsJavaScriptException();
        return env.Null();
    }
    
    if (!info[0].IsBuffer()) {
        Napi::TypeError::New(env, "Buffer expected").ThrowAsJavaScriptException();
        return env.Null();
    }
    
    if (!info[1].IsBuffer()) {
        Napi::TypeError::New(env, "Buffer expected").ThrowAsJavaScriptException();
        return env.Null();
    }
    
    NSDSP_CONN_HANDLE handle = *info[0].As<Napi::Buffer<NSDSP_CONN_HANDLE>>().Data();
    
    unsigned char* mode = info[1].As<Napi::Buffer<unsigned char>>().Data();
    
    return Napi::Number::New(env, NSDSPSetMode(handle, mode));
}

Napi::Value SetTimeout(const Napi::CallbackInfo & info) {
    Napi::Env env = info.Env();

    
    if(info.Length() < 2){
        Napi::TypeError::New(env, "Wrong number of arguments").ThrowAsJavaScriptException();
        return env.Null();
    }
    
    if (!info[0].IsBuffer()) {
        Napi::TypeError::New(env, "Buffer expected").ThrowAsJavaScriptException();
        return env.Null();
    }
    
    if (!info[1].IsNumber()) {
        Napi::TypeError::New(env, "Number expected").ThrowAsJavaScriptException();
        return env.Null();
    }
    
    NSDSP_CONN_HANDLE handle = *info[0].As<Napi::Buffer<NSDSP_CONN_HANDLE>>().Data();
    
    unsigned int timeout = info[1].As<Napi::Number>();
    
    NSDSPSetTimeout(handle, timeout);
    
    return Napi::Number::New(env, timeout);
}

Napi::Value Write(const Napi::CallbackInfo & info) {
    Napi::Env env = info.Env();

    if(info.Length() < 3){
        Napi::TypeError::New(env, "Wrong number of arguments").ThrowAsJavaScriptException();
        return env.Null();
    }
    
    if (!info[0].IsBuffer()) {
        Napi::TypeError::New(env, "Buffer expected").ThrowAsJavaScriptException();
        return env.Null();
    }
    
    if (!info[1].IsBuffer()) {
        Napi::TypeError::New(env, "Buffer expected").ThrowAsJavaScriptException();
        return env.Null();
    }
    
    if (!info[2].IsNumber()) {
        Napi::TypeError::New(env, "Number expected").ThrowAsJavaScriptException();
        return env.Null();
    }
    
    NSDSP_CONN_HANDLE handle = *info[0].As<Napi::Buffer<NSDSP_CONN_HANDLE>>().Data();
    
    char* src = info[1].As<Napi::Buffer<char>>().Data();
    
    unsigned int size = info[2].As<Napi::Number>();
    
    return Napi::Number::New(env, NSDSPWrite(handle, src, size));
}

Napi::Value WriteCommand(const Napi::CallbackInfo & info) {
    Napi::Env env = info.Env();

    if(info.Length() < 3){
        Napi::TypeError::New(env, "Wrong number of arguments").ThrowAsJavaScriptException();
        return env.Null();
    }
    
    if (!info[0].IsBuffer()) {
        Napi::TypeError::New(env, "Buffer expected").ThrowAsJavaScriptException();
        return env.Null();
    }
    
    if (!info[1].IsNumber()) {
        Napi::TypeError::New(env, "Number expected").ThrowAsJavaScriptException();
        return env.Null();
    }
    
    if (!info[2].IsBuffer()) {
        Napi::TypeError::New(env, "Buffer expected").ThrowAsJavaScriptException();
        return env.Null();
    }
    
    if (!info[3].IsNumber()) {
        Napi::TypeError::New(env, "Number expected").ThrowAsJavaScriptException();
        return env.Null();
    }
    
    NSDSP_CONN_HANDLE handle = *info[0].As<Napi::Buffer<NSDSP_CONN_HANDLE>>().Data();
    
    char cmd = info[1].As<Napi::Number>().Uint32Value() & 0xFF;
    
    char* src = info[2].As<Napi::Buffer<char>>().Data();
    
    unsigned int size = info[3].As<Napi::Number>();
    
    return Napi::Number::New(env, NSDSPWriteCommand(handle, cmd, src, size));
}

Napi::Value WriteSPI(const Napi::CallbackInfo & info) {
    Napi::Env env = info.Env();

    if(info.Length() < 3){
        Napi::TypeError::New(env, "Wrong number of arguments").ThrowAsJavaScriptException();
        return env.Null();
    }
    
    if (!info[0].IsBuffer()) {
        Napi::TypeError::New(env, "Buffer expected").ThrowAsJavaScriptException();
        return env.Null();
    }
    
    if (!info[1].IsBuffer()) {
        Napi::TypeError::New(env, "Buffer expected").ThrowAsJavaScriptException();
        return env.Null();
    }
    
    if (!info[2].IsNumber()) {
        Napi::TypeError::New(env, "Number expected").ThrowAsJavaScriptException();
        return env.Null();
    }
    
    NSDSP_CONN_HANDLE handle = *info[0].As<Napi::Buffer<NSDSP_CONN_HANDLE>>().Data();
    
    char* src = info[1].As<Napi::Buffer<char>>().Data();
    
    unsigned int size = info[2].As<Napi::Number>();
    
    return Napi::Number::New(env, NSDSPWriteSPI(handle, src, size));
}

Napi::Value Delay(const Napi::CallbackInfo & info) {
    Napi::Env env = info.Env();

    
    if(info.Length() < 2){
        Napi::TypeError::New(env, "Wrong number of arguments").ThrowAsJavaScriptException();
        return env.Null();
    }
    
    if (!info[0].IsBuffer()) {
        Napi::TypeError::New(env, "Buffer expected").ThrowAsJavaScriptException();
        return env.Null();
    }
    
    if (!info[1].IsNumber()) {
        Napi::TypeError::New(env, "Number expected").ThrowAsJavaScriptException();
        return env.Null();
    }
    
    NSDSP_CONN_HANDLE handle = *info[0].As<Napi::Buffer<NSDSP_CONN_HANDLE>>().Data();
    
    unsigned int delay = info[1].As<Napi::Number>();
    
    return Napi::Number::New(env, NSDSPDelay(handle, delay));
}

Napi::Value Flush(const Napi::CallbackInfo & info) {
    Napi::Env env = info.Env();

    
    if(info.Length() < 1){
        Napi::TypeError::New(env, "Wrong number of arguments").ThrowAsJavaScriptException();
        return env.Null();
    }
    
    if (!info[0].IsBuffer()) {
        Napi::TypeError::New(env, "Buffer expected").ThrowAsJavaScriptException();
        return env.Null();
    }
    
    NSDSP_CONN_HANDLE handle = *info[0].As<Napi::Buffer<NSDSP_CONN_HANDLE>>().Data();
    
    return Napi::Number::New(env, NSDSPFlush(handle));
}

Napi::Value WaitForCompletion(const Napi::CallbackInfo & info) {
    Napi::Env env = info.Env();

    
    if(info.Length() < 1){
        Napi::TypeError::New(env, "Wrong number of arguments").ThrowAsJavaScriptException();
        return env.Null();
    }
    
    if (!info[0].IsBuffer()) {
        Napi::TypeError::New(env, "Buffer expected").ThrowAsJavaScriptException();
        return env.Null();
    }
    
    NSDSP_CONN_HANDLE handle = *info[0].As<Napi::Buffer<NSDSP_CONN_HANDLE>>().Data();
    
    return Napi::Number::New(env, NSDSPWaitForCompletion(handle));
}

Napi::Value AvailableData(const Napi::CallbackInfo & info) {
    Napi::Env env = info.Env();

    
    if(info.Length() < 1){
        Napi::TypeError::New(env, "Wrong number of arguments").ThrowAsJavaScriptException();
        return env.Null();
    }
    
    if (!info[0].IsBuffer()) {
        Napi::TypeError::New(env, "Buffer expected").ThrowAsJavaScriptException();
        return env.Null();
    }
    
    NSDSP_CONN_HANDLE handle = *info[0].As<Napi::Buffer<NSDSP_CONN_HANDLE>>().Data();
    
    return Napi::Number::New(env, NSDSPAvailableData(handle));
}

Napi::Value WaitForData(const Napi::CallbackInfo & info) {
    Napi::Env env = info.Env();

    
    if(info.Length() < 2){
        Napi::TypeError::New(env, "Wrong number of arguments").ThrowAsJavaScriptException();
        return env.Null();
    }
    
    if (!info[0].IsBuffer()) {
        Napi::TypeError::New(env, "Buffer expected").ThrowAsJavaScriptException();
        return env.Null();
    }
    
    if (!info[1].IsNumber()) {
        Napi::TypeError::New(env, "Number expected").ThrowAsJavaScriptException();
        return env.Null();
    }
    
    NSDSP_CONN_HANDLE handle = *info[0].As<Napi::Buffer<NSDSP_CONN_HANDLE>>().Data();
    
    unsigned int size = info[1].As<Napi::Number>();
    
    return Napi::Number::New(env, NSDSPWaitForData(handle, size));
}

Napi::Value WaitForDataForever(const Napi::CallbackInfo & info) {
    Napi::Env env = info.Env();

    
    if(info.Length() < 2){
        Napi::TypeError::New(env, "Wrong number of arguments").ThrowAsJavaScriptException();
        return env.Null();
    }
    
    if (!info[0].IsBuffer()) {
        Napi::TypeError::New(env, "Buffer expected").ThrowAsJavaScriptException();
        return env.Null();
    }
    
    if (!info[1].IsNumber()) {
        Napi::TypeError::New(env, "Number expected").ThrowAsJavaScriptException();
        return env.Null();
    }
    
    NSDSP_CONN_HANDLE handle = *info[0].As<Napi::Buffer<NSDSP_CONN_HANDLE>>().Data();
    
    unsigned int size = info[1].As<Napi::Number>();
    
    return Napi::Number::New(env, NSDSPWaitForDataForever(handle, size));
}

Napi::Value Read(const Napi::CallbackInfo & info) {
    Napi::Env env = info.Env();

    if(info.Length() < 3){
        Napi::TypeError::New(env, "Wrong number of arguments").ThrowAsJavaScriptException();
        return env.Null();
    }
    
    if (!info[0].IsBuffer()) {
        Napi::TypeError::New(env, "Buffer expected").ThrowAsJavaScriptException();
        return env.Null();
    }
    
    if (!info[1].IsBuffer()) {
        Napi::TypeError::New(env, "Buffer expected").ThrowAsJavaScriptException();
        return env.Null();
    }
    
    if (!info[2].IsNumber()) {
        Napi::TypeError::New(env, "Number expected").ThrowAsJavaScriptException();
        return env.Null();
    }
    
    NSDSP_CONN_HANDLE handle = *info[0].As<Napi::Buffer<NSDSP_CONN_HANDLE>>().Data();
    
    Napi::Uint8Array arr = info[0].As<Napi::Uint8Array>();
    char* metadataValue = reinterpret_cast<char*>(arr.ArrayBuffer().Data()); 
    
    unsigned int size = info[2].As<Napi::Number>();
    
    return Napi::Number::New(env, NSDSPRead(handle, metadataValue, size));
}

Napi::Object InitAll(Napi::Env env, Napi::Object exports) {
    //EnumData::Init(env, exports);
    exports.Set(Napi::String::New(env, "NSDSPEnumerate"), Napi::Function::New(env, Enumerate));
    exports.Set(Napi::String::New(env, "NSDSPConnect"), Napi::Function::New(env, Connect));
    exports.Set(Napi::String::New(env, "NSDSPDisconnect"), Napi::Function::New(env, Disconnect));
    exports.Set(Napi::String::New(env, "NSDSPGetSerial"), Napi::Function::New(env, GetSerial));
    exports.Set(Napi::String::New(env, "NSDSPGetVersion"), Napi::Function::New(env, GetVersion));
    exports.Set(Napi::String::New(env, "NSDSPGetBaudRate"), Napi::Function::New(env, GetBaudRate));
    exports.Set(Napi::String::New(env, "NSDSPGetFlowControl"), Napi::Function::New(env, GetFlowControl));
    exports.Set(Napi::String::New(env, "NSDSPGetCTS"), Napi::Function::New(env, GetCTS));
    exports.Set(Napi::String::New(env, "NSDSPGetRX"), Napi::Function::New(env, GetRX));
    exports.Set(Napi::String::New(env, "NSDSPSetMode"), Napi::Function::New(env, SetMode));
    exports.Set(Napi::String::New(env, "NSDSPSetTimeout"), Napi::Function::New(env, SetTimeout));
    exports.Set(Napi::String::New(env, "NSDSPWrite"), Napi::Function::New(env, Write));
    exports.Set(Napi::String::New(env, "NSDSPWriteCommand"), Napi::Function::New(env, WriteCommand));
    exports.Set(Napi::String::New(env, "NSDSPWriteSPI"), Napi::Function::New(env, WriteSPI));
    exports.Set(Napi::String::New(env, "NSDSPDelay"), Napi::Function::New(env, Delay));
    exports.Set(Napi::String::New(env, "NSDSPFlush"), Napi::Function::New(env, Flush));
    exports.Set(Napi::String::New(env, "NSDSPWaitForCompletion"), Napi::Function::New(env, WaitForCompletion));
    exports.Set(Napi::String::New(env, "NSDSPAvailableData"), Napi::Function::New(env, AvailableData));
    exports.Set(Napi::String::New(env, "NSDSPWaitForData"), Napi::Function::New(env, WaitForData));
    exports.Set(Napi::String::New(env, "NSDSPWaitForDataForever"), Napi::Function::New(env, WaitForDataForever));
    exports.Set(Napi::String::New(env, "NSDSPRead"), Napi::Function::New(env, Read));
    return exports;
}

NODE_API_MODULE(nsdsp, InitAll)
