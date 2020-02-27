#include "wmctrl-napi.hpp"

using namespace Napi;

class ActiveByIdWorker : public AsyncWorker {
    public:
        ActiveByIdWorker(Napi::Env &env, Promise::Deferred deferred, Window win_id)
        : AsyncWorker(env), deferred(deferred), win_id(win_id) {}
        ~ActiveByIdWorker() {}

    void Execute() override {
        st = active_window_by_id(wmctrljs::disp, this->win_id);
        wmctrljs::sync();
    }

    void OnOK() override {
        Napi::Env env = Env();
        char *err;
        String err_js;

        if (st != WINDOW_ACTIVATED) {
            err = get_libwmctrl_error("activeWindowById", st);
            err_js = String::New(env, err);
            free(err);
            this->deferred.Reject(err_js);
        } else {
            this->deferred.Resolve(Boolean::New(env, true));
        }
    }

    private:
        Promise::Deferred deferred;
        Window win_id;
        enum STATES st;
};

Boolean activeWindowByIdSync(const CallbackInfo &info) {
    checkId(info, "activeWindowByIdSync");
    int32_t win_id;
    enum STATES st;
    Env env = info.Env();
    win_id = info[0].As<Number>().Int32Value();
    st = active_window_by_id(wmctrljs::disp, win_id);
    wmctrljs::sync();

    if (st != WINDOW_ACTIVATED) {
        handling_libwmctrl_error(env, "activeWindowByIdSync", st);
        return Boolean::New(env, false);
    }
    return Boolean::New(env, true);
}

Promise activeWindowByIdAsync(const CallbackInfo &info) {
    checkId(info, "activeWindowByIdAsync");
    int32_t win_id;
    Env env = info.Env();    
    win_id = info[0].As<Number>().Int32Value();
    Promise::Deferred deferred = Promise::Deferred::New(env);
    ActiveByIdWorker *wk = new ActiveByIdWorker(env, deferred, win_id);
    wk->Queue();
    return deferred.Promise();
}