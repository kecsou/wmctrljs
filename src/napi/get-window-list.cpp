#include "./wmctrl-napi.hpp"


class GetWindowListWorker : public AsyncWorker {
    public:
        GetWindowListWorker(Napi::Env &env, Promise::Deferred deferred) 
            : AsyncWorker(env), deferred(deferred), wl(NULL) {}
        ~GetWindowListWorker() {
            if (this->wl)
                free_window_list(this->wl);
        }

    void Execute() override {
        this->wl = list_windows(wmctrljs::disp, &this->st);
        wmctrljs::sync();
    }

    void OnOK() override {
        Napi::Env env = Env();
        String err_js;
        Array windows_js;
        char *err;
        struct window_info *wi;

        if (this->st != CLIENT_LIST_GET) {
            err = get_error_message(this->st);
            err_js = String::New(env, err);
            free(err);
            this->deferred.Reject(err_js);
        } else {
            windows_js = Array::New(env, this->wl->client_list_size);
            for (size_t i = 0; i < this->wl->client_list_size; i++) {
                wi = this->wl->client_list + i;
                windows_js[i] = create_window_js(env, wi);
            }
            this->deferred.Resolve(windows_js);
        }
    }

    private:
        Promise::Deferred deferred;
        struct window_list *wl;
        enum STATES st;
};

Promise getWindowListAsync(const CallbackInfo &info) {
    Napi::Env env = info.Env();
    Promise::Deferred deferred = Promise::Deferred::New(env);
    GetWindowListWorker *wk = new GetWindowListWorker(env, deferred);
    wk->Queue();
    return deferred.Promise();
}

Value getWindowListSync(const CallbackInfo &info) {
    Napi::Env env = info.Env();
    Array windows_js;
    enum STATES st;
    struct window_info *wi;
    struct window_list *wl = list_windows(wmctrljs::disp, &st);
    wmctrljs::sync();

    if (st != CLIENT_LIST_GET) {
        handling_libwmctrl_error(env, "getWindowListSync", st);
        return env.Null();
    }

    windows_js = Array::New(env, wl->client_list_size);
    for (size_t i = 0; i < wl->client_list_size; i++) {
        wi = wl->client_list + i;
        windows_js[i] = create_window_js(env, wi);
    }
    free_window_list(wl);

    return windows_js;
}