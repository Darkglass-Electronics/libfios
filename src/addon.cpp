// src/addon.cpp
#include <napi.h>

// Make sure we link the C symbols without C++ name mangling
extern "C"
{
#include "libfios.h"        // declares fios_serial_* / fios_file_* / enums
#include "libfios-export.h" // declares new_float_ptr / get_float_ptr_value / delete_float_ptr
}

// ---- helpers ---------------------------------------------------------------

template <typename T>
static T *expectExternal(const Napi::Env &env, const Napi::Value &v, const char *what)
{
    if (!v.IsExternal())
    {
        Napi::TypeError::New(env, std::string(what) + " must be an External").ThrowAsJavaScriptException();
        return nullptr;
    }
    return v.As<Napi::External<T>>().Data();
}

// ---- bindings --------------------------------------------------------------

static Napi::Value JS_fios_serial_open(const Napi::CallbackInfo &info)
{
    Napi::Env env = info.Env();
    if (info.Length() < 1 || !info[0].IsString())
    {
        Napi::TypeError::New(env, "Expected devicePath: string").ThrowAsJavaScriptException();
        return env.Null();
    }

    std::string dev = info[0].As<Napi::String>();
    fios_serial_t *s = fios_serial_open(dev.c_str());
    if (s == nullptr)
        return env.Null();

    return Napi::External<fios_serial_t>::New(env, s);
}

static Napi::Value JS_fios_serial_close(const Napi::CallbackInfo &info)
{
    Napi::Env env = info.Env();
    if (info.Length() < 1)
    {
        Napi::TypeError::New(env, "Expected (serial)").ThrowAsJavaScriptException();
        return env.Undefined();
    }
    fios_serial_t *s = expectExternal<fios_serial_t>(env, info[0], "serial");
    if (!s)
        return env.Undefined();

    fios_serial_close(s);
    return env.Undefined();
}

static Napi::Value JS_fios_file_send(const Napi::CallbackInfo &info)
{
    Napi::Env env = info.Env();
    if (info.Length() < 2)
    {
        Napi::TypeError::New(env, "Expected (serial, inPath: string)").ThrowAsJavaScriptException();
        return env.Null();
    }
    fios_serial_t *s = expectExternal<fios_serial_t>(env, info[0], "serial");
    if (!s)
        return env.Null();
    if (!info[1].IsString())
    {
        Napi::TypeError::New(env, "inPath must be a string").ThrowAsJavaScriptException();
        return env.Null();
    }

    std::string inPath = info[1].As<Napi::String>();
    fios_file_t *f = fios_file_send(s, inPath.c_str());
    if (f == nullptr)
        return env.Null();

    return Napi::External<fios_file_t>::New(env, f);
}

static Napi::Value JS_fios_file_receive(const Napi::CallbackInfo &info)
{
    Napi::Env env = info.Env();
    if (info.Length() < 2)
    {
        Napi::TypeError::New(env, "Expected (serial, outPath: string)").ThrowAsJavaScriptException();
        return env.Null();
    }
    fios_serial_t *s = expectExternal<fios_serial_t>(env, info[0], "serial");
    if (!s)
        return env.Null();
    if (!info[1].IsString())
    {
        Napi::TypeError::New(env, "outPath must be a string").ThrowAsJavaScriptException();
        return env.Null();
    }

    std::string outPath = info[1].As<Napi::String>();
    fios_file_t *f = fios_file_receive(s, outPath.c_str());
    if (f == nullptr)
        return env.Null();

    return Napi::External<fios_file_t>::New(env, f);
}

static Napi::Value JS_fios_file_idle(const Napi::CallbackInfo &info)
{
    Napi::Env env = info.Env();
    if (info.Length() < 1)
    {
        Napi::TypeError::New(env, "Expected (file[, progressPtr])").ThrowAsJavaScriptException();
        return env.Null();
    }

    fios_file_t *f = expectExternal<fios_file_t>(env, info[0], "file");
    if (!f)
        return env.Null();

    float *progressPtr = nullptr;
    if (info.Length() >= 2 && !info[1].IsUndefined() && !info[1].IsNull())
    {
        progressPtr = expectExternal<float>(env, info[1], "progressPtr");
        if (!progressPtr)
            return env.Null();
    }

    const int status = static_cast<int>(fios_file_idle(f, progressPtr));
    return Napi::Number::New(env, status);
}

static Napi::Value JS_fios_file_close(const Napi::CallbackInfo &info)
{
    Napi::Env env = info.Env();
    if (info.Length() < 1)
    {
        Napi::TypeError::New(env, "Expected (file)").ThrowAsJavaScriptException();
        return env.Undefined();
    }
    fios_file_t *f = expectExternal<fios_file_t>(env, info[0], "file");
    if (!f)
        return env.Undefined();

    fios_file_close(f);
    return env.Undefined();
}

static Napi::Value JS_new_float_ptr(const Napi::CallbackInfo &info)
{
    Napi::Env env = info.Env();
    float *p = new_float_ptr();
    if (p == nullptr)
        return env.Null();
    return Napi::External<float>::New(env, p);
}

static Napi::Value JS_get_float_ptr_value(const Napi::CallbackInfo &info)
{
    Napi::Env env = info.Env();
    if (info.Length() < 1)
    {
        Napi::TypeError::New(env, "Expected (progressPtr)").ThrowAsJavaScriptException();
        return env.Null();
    }
    float *p = expectExternal<float>(env, info[0], "progressPtr");
    if (!p)
        return env.Null();

    float v = get_float_ptr_value(p);
    return Napi::Number::New(env, v);
}

static Napi::Value JS_delete_float_ptr(const Napi::CallbackInfo &info)
{
    Napi::Env env = info.Env();
    if (info.Length() < 1)
    {
        Napi::TypeError::New(env, "Expected (progressPtr)").ThrowAsJavaScriptException();
        return env.Undefined();
    }
    float *p = expectExternal<float>(env, info[0], "progressPtr");
    if (!p)
        return env.Undefined();

    delete_float_ptr(p);
    return env.Undefined();
}

// ---- module init -----------------------------------------------------------

Napi::Object Init(Napi::Env env, Napi::Object exports)
{
    exports.Set("fios_serial_open", Napi::Function::New(env, JS_fios_serial_open));
    exports.Set("fios_serial_close", Napi::Function::New(env, JS_fios_serial_close));
    exports.Set("fios_file_receive", Napi::Function::New(env, JS_fios_file_receive));
    exports.Set("fios_file_send", Napi::Function::New(env, JS_fios_file_send));
    exports.Set("fios_file_idle", Napi::Function::New(env, JS_fios_file_idle));
    exports.Set("fios_file_close", Napi::Function::New(env, JS_fios_file_close));
    exports.Set("new_float_ptr", Napi::Function::New(env, JS_new_float_ptr));
    exports.Set("get_float_ptr_value", Napi::Function::New(env, JS_get_float_ptr_value));
    exports.Set("delete_float_ptr", Napi::Function::New(env, JS_delete_float_ptr));

    // (Optional but handy) expose enum values if libfios.h declares them
#ifdef fios_file_status_in_progress
    exports.Set("FILE_STATUS_IN_PROGRESS", Napi::Number::New(env, (int)fios_file_status_in_progress));
#endif
#ifdef fios_file_status_completed
    exports.Set("FILE_STATUS_COMPLETED", Napi::Number::New(env, (int)fios_file_status_completed));
#endif
#ifdef fios_file_status_error
    exports.Set("FILE_STATUS_ERROR", Napi::Number::New(env, (int)fios_file_status_error));
#endif

    return exports;
}

// Make sure target_name matches your .node filename (node-gyp does this via macro)
NODE_API_MODULE(NODE_GYP_MODULE_NAME, Init)
