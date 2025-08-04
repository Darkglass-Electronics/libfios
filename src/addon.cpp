#include <napi.h>
#include "libfios.h"
#include "libfios-export.h"

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

void sleep_ms(int ms) {
#ifdef _WIN32
    Sleep(ms);
#else
    usleep(ms * 1000);
#endif
}

class TransferWorker : public Napi::AsyncWorker {
public:
    TransferWorker(
        const Napi::Function& callback,
        std::string mode,
        std::string devicePath,
        std::string filePath
    )
        : Napi::AsyncWorker(callback), mode_(mode), devicePath_(devicePath), filePath_(filePath)
    {}

    void Execute() override {
        bool sending;
        if (mode_ == "r")
            sending = false;
        else if (mode_ == "s")
            sending = true;
        else {
            SetError("Invalid mode");
            return;
        }

        const char* devpath = devicePath_ == "auto"
            ? (
#ifdef _WIN32
                "COM5"
#elif defined(__aarch64__)
                "/dev/ttyGS0"
#else
                "/dev/ttyACM0"
#endif
            )
            : devicePath_.c_str();

        fios_serial_t* s = fios_serial_open(devpath);
        if (s == NULL) {
            SetError("Failed to open device");
            return;
        }

        fios_file_t* f = sending
            ? fios_file_send(s, filePath_.c_str())
            : fios_file_receive(s, filePath_.c_str());
        if (f == NULL) {
            fios_serial_close(s);
            SetError("Failed to send/receive file");
            return;
        }

        float progress;
        // For demonstration, just update progress until finished.
        while (fios_file_idle(f, &progress) == fios_file_status_in_progress) {
            // In a real-world scenario, you may want to send progress to JS.
            sleep_ms(100);
        }

        fios_file_close(f);
        fios_serial_close(s);
        // Optionally: store success/progress
    }

    void OnOK() override {
        Napi::HandleScope scope(Env());
        Callback().Call({ Env().Null(), Napi::String::New(Env(), "ok") });
    }

    void OnError(const Napi::Error& e) override {
        Callback().Call({ e.Value(), Env().Undefined() });
    }

private:
    std::string mode_;
    std::string devicePath_;
    std::string filePath_;
};

Napi::Value transferFile(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    if (info.Length() < 4) {
        Napi::TypeError::New(env, "Expected mode, devicePath, filePath, callback").ThrowAsJavaScriptException();
        return env.Null();
    }

    std::string mode = info[0].As<Napi::String>();
    std::string devicePath = info[1].As<Napi::String>();
    std::string filePath = info[2].As<Napi::String>();
    Napi::Function cb = info[3].As<Napi::Function>();

    auto* worker = new TransferWorker(cb, mode, devicePath, filePath);
    worker->Queue();
    return env.Undefined();
}

Napi::Object Init(Napi::Env env, Napi::Object exports) {
    exports.Set("transferFile", Napi::Function::New(env, transferFile));
    return exports;
}

NODE_API_MODULE(libfios, Init)
