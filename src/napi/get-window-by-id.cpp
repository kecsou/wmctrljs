#include "wmctrl-napi.hpp"

class GetByIdWorker : public AsyncWorker {
    public:
        GetByIdWorker(Napi::Env &env, Promise::Deferred deferred, Window id) 
            :AsyncWorker(env), deferred(deferred), id(id), wi(NULL), error(NULL) {}
        ~GetByIdWorker() {

            if (this->wi)
                free_window_info(this->wi);

            if (this->error)
                free(this->error);
        }

    void Execute() override {
        Display *disp = XOpenDisplay(NULL);
        this->wi = create_window_info(disp, this->id);
        XCloseDisplay(disp);
    }

    void OnOK() override {
        Napi::Env env = Env();
        Object wi_js;
        String error_js;
        if (!this->wi) {
            this->error = get_error_message(NO_WINDOW_FOUND);
            error_js = String::New(env, this->error);
            this->deferred.Reject(error_js);
        } else {
            wi_js = create_window_js(env, this->wi);
            this->deferred.Resolve(wi_js);
        }    
    }

    private:
        Promise::Deferred deferred;
        Window id;
        struct window_info *wi;
        char *error;
};

Promise getWindowByIdAsync(const CallbackInfo &info) {
    checkId(info, "getWindowById");
    Env env = info.Env();

    int id = info[0].As<Number>();
    Promise::Deferred deferred = Promise::Deferred::New(env);
    GetByIdWorker *wk = new GetByIdWorker(env, deferred, (Window)id);
    wk->Queue();

    return deferred.Promise();
}