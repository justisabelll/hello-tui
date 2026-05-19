#include <napi.h>
#include <cstdint>
#include <string>

// internal state
// nothing here is visible to JS/TS, values only cross if wrapped in a Napi::Value and returned to JS/TS

static int32_t s_frame_count = 0;

//
// Every exported function has the same signature: 
// Napi::Value FunctionName(const Napi::CallbackInfo& info)
// `info` is how you access arguments. `info.Env()` gives you the handle to the runtime environment, which you need to create new JS values to return.
//

Napi::Value GetGreeting(const Napi::CallbackInfo &info)
{
    Napi::Env env = info.Env();

    if (info.Length() < 1)
    {
        Napi::TypeError::New(env, "GetGreeting expects 1 argument: string").ThrowAsJavaScriptException();

        // after exception, always return immediately, do not continue executing C++ code

        return env.Null();
    }

    if (!info[0].IsString())
    {
        Napi::TypeError::New(env, "GetGreeting expects 1 argument: string").ThrowAsJavaScriptException();

        return env.Null();
    }
    
    // we copy the JS string into std, std then owns it independently 
    std::string name = info[0].As<Napi::String>().Utf8Value();
    std::string greeting = "Hello from C++, " + name + "!";

    // the env is is needed so the runtime can track the lifetime of the returned value, and free it when no longer needed
    return Napi::String::New(env,greeting); 
}

// increments the frame count and returns the new value
Napi::Value TickFrame(const Napi::CallbackInfo& info){
    Napi::Env env = info.Env();

    ++s_frame_count;

    // Napi::Number::New() wraps a C++ numeric type in a JS Number value.
    // use Int32Value(), DoubleValue(), etc. to go the other direction
    // (JS Number → C++ type) when reading arguments.
    return Napi::Number::New(env, s_frame_count);
}

// this func is called once when the addon is loaded with require
// `exports` is the object that require() returns to the JS caller
// attach all public functions here using exports.Set().
//
// the string passed to Set() is the name the JS caller uses to access it:
// const { getGreeting } = require("./build/Release/hello.node");
Napi::Object Init(Napi::Env env, Napi::Object exports){

    exports.Set(
        Napi::String::New(env, "getGreeting"),
        Napi::Function::New(env, GetGreeting)
    );

    exports.Set(
        Napi::String::New(env, "tickFrame"),
        Napi::Function::New(env,TickFrame)
    );


    return exports;
}

// macro that registers `Init` as the module intiializer
// first arg is internal identifier, does not affect the symbol require() uses, should use the project name
NODE_API_MODULE(hello, Init)