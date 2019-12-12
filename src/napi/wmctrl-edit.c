#include "wmctrl-napi.h"

void activeWindowById(napi_env env, napi_callback_info info) {
    Display *disp;
    if (! (disp = XOpenDisplay(NULL))) {
        napi_throw_error(env, NULL, "Can't open dispay");
        return;
    }
    napi_value args[1];
    size_t argc = 1;

    if (napi_get_cb_info(env, info, &argc, args, NULL, NULL) != napi_ok) {
        napi_throw_error(env, NULL, "Can't get function parameters");
        return;
    }

    if (argc != 1) {
        napi_throw_error(env, "EINVAL", "Must have one arg wid_js [activeWindowById]");
        return;
    }

    Window win;
    if (napi_get_value_int32(env, args[0], (int32_t *)&win) != napi_ok) {
        napi_throw_error(env, NULL, "Can't get wid_js [activeWindowById]");
        return;
    }

    active_window_by_id(disp, win);
}

/*
int active_windows_by_pid(Display *disp, unsigned long pid);
int active_windows_by_class_name(Display *disp, char *class_name);
int close_window_by_id(Display *disp, Window win);
int close_windows_by_pid(Display *disp, unsigned long pid);
int close_windows_by_class_name(Display *disp, char *class_name);
int window_state (Display *disp, Window win, unsigned long action, 
        char *prop1_str, char *prop2_str);
int window_set_title(Display *disp, Window win,
        const char *_net_wm_name);
int window_set_icon_name(Display *disp, Window win, 
        const char *_net_wm_icon_name);
Window Select_Window(Display *dpy);*/