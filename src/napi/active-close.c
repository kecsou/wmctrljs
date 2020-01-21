#include "wmctrl-napi.h"

napi_value activeWindowById(napi_env env, napi_callback_info info) {
    size_t argc = 1;
    napi_value args[1];
    Window win;
    enum STATES st;
    napi_value success, failure;

    napi_create_int32(env, 1, &success);
    napi_create_int32(env, 0, &failure);

    if (napi_get_cb_info(env, info, &argc, args, NULL, NULL) != napi_ok) {
        napi_throw_error(env, NULL, "Can't get function parameters");
        return failure;
    }

    if (argc != 1) {
        napi_throw_error(env, "EINVAL", "Must have one arg wid_js [activeWindowById]");
        return failure;
    }

    if (napi_get_value_int32(env, args[0], (int32_t *)&win) != napi_ok) {
        napi_throw_error(env, NULL, "Can't get wid_js [activeWindowById]");
        return failure;
    }

    st = active_window_by_id(NULL, win);
    if (st != WINDOW_ACTIVATED) {
        handling_libwmctrl_error(env, "activeWindowById", st);
        return failure;
    }

    return success;
}

napi_value activeWindowsByClassName(napi_env env, napi_callback_info  info) {
    size_t argc = 1;
    napi_value args[1];
    char class_name[1024];
    size_t size;
    enum STATES st;
    napi_value success, failure;

    napi_create_int32(env, 1, &success);
    napi_create_int32(env, 0, &failure);
    if (napi_get_cb_info(env, info, &argc, args, NULL, NULL) != napi_ok) {
        napi_throw_error(env, NULL, "Can't get function parameters");
        return failure;
    }

    if (argc != 1) {
        napi_throw_error(env, "EINVAL", "Must have one arg class_name_js [activeWindowsByClassName]");
        return failure;
    }

    if (napi_get_value_string_utf8(env, args[0], (char *)&class_name, 1024, &size)  != napi_ok) {
        napi_throw_error(env, NULL, "Can't get class_name_js [activeWindowsByClassName]");
        return failure;
    }

    st = active_windows_by_class_name(NULL, class_name);
    if (st != WINDOWS_ACTIVATED) {
        handling_libwmctrl_error(env, "activeWindowsByClassName", st);
        return failure;
    }

    return success;
}

napi_value activeWindowsByPid(napi_env env, napi_callback_info info) {
    size_t argc = 1;
    napi_value args[1];
    enum STATES st;
    int32_t pid;
    napi_value success, failure;

    napi_create_int32(env, 1, &success);
    napi_create_int32(env, 0, &failure);

    if (napi_get_cb_info(env, info, &argc, args, NULL, NULL) != napi_ok) {
        napi_throw_error(env, NULL, "Can't get function parameters");
        return failure;
    }

    if (argc != 1) {
        napi_throw_error(env, "EINVAL", "Must have one arg pid_js [activeWindowsByPid]");
        return failure;
    }

    if (napi_get_value_int32(env, args[0], &pid) != napi_ok) {
        napi_throw_error(env, NULL, "Can't get pid_js [activeWindowsByPid]");
        return failure;
    }

    st = active_windows_by_pid(NULL, pid);
    if (st != WINDOWS_ACTIVATED) {
        handling_libwmctrl_error(env, "activeWindowsByPid", st);
        return failure;
    }

    return success;
}

napi_value closeWindowById(napi_env env, napi_callback_info info) {
    size_t argc = 1;
    napi_value args[1];
    Window win;
    enum STATES st;
    napi_value success, failure;

    napi_create_int32(env, 1, &success);
    napi_create_int32(env, 0, &failure);

    if (napi_get_cb_info(env, info, &argc, args, NULL, NULL) != napi_ok) {
        napi_throw_error(env, NULL, "Can't get function parameters");
        return failure;
    }

    if (argc != 1) {
        napi_throw_error(env, "EINVAL", "Must have one arg wid_js [closeWindowById]");
        return failure;
    }

    if (napi_get_value_int32(env, args[0], (int32_t *)&win) != napi_ok) {
        napi_throw_error(env, NULL, "Can't get wid_js [closeWindowById]");
        return failure;
    }

    st = close_window_by_id(NULL, win);
    if (st != WINDOW_CLOSED) {
        handling_libwmctrl_error(env, "closeWindowById", st);
        return failure;
    }

    return success;
}

napi_value closeWindowsByClassName(napi_env env, napi_callback_info info) {
    size_t argc = 1;
    napi_value args[1];
    char class_name[1024];
    size_t size;
    enum STATES st;
    napi_value success, failure;

    napi_create_int32(env, 1, &success);
    napi_create_int32(env, 0, &failure);
    if (napi_get_cb_info(env, info, &argc, args, NULL, NULL) != napi_ok) {
        napi_throw_error(env, NULL, "Can't get function parameters");
        return failure;
    }
    if (argc != 1) {
        napi_throw_error(env, "EINVAL", "Must have one arg class_name_js [closeWindowsByClassName]");
        return failure;
    }

    if (napi_get_value_string_utf8(env, args[0], (char *)&class_name, 1024, &size)  != napi_ok) {
        napi_throw_error(env, NULL, "Can't get class_name_js [closeWindowsByClassName]");
        return failure;
    }

    st = close_windows_by_class_name(NULL, class_name);
    if (st != WINDOWS_CLOSED) {
        handling_libwmctrl_error(env, "closeWindowsByClassName", st);
        return failure;
    }

    return success;
}

napi_value closeWindowsByPid(napi_env env, napi_callback_info info) {
    size_t argc = 1;
    napi_value args[1];
    int32_t pid;
    enum STATES st;
    napi_value success, failure;

    napi_create_int32(env, 1, &success);
    napi_create_int32(env, 0, &failure);

    if (napi_get_cb_info(env, info, &argc, args, NULL, NULL) != napi_ok) {
        napi_throw_error(env, NULL, "Can't get function parameters");
        return failure;
    }

    if (argc != 1) {
        napi_throw_error(env, "EINVAL", "Must have one arg pid_js [closeWindowsByPid]");
        return failure;
    }

    if (napi_get_value_int32(env, args[0], &pid) != napi_ok) {
        napi_throw_error(env, NULL, "Can't get pid_js [closeWindowsByPid]");
        return failure;
    }

    st = close_windows_by_pid(NULL, pid);
    if (st != WINDOWS_CLOSED) {
        handling_libwmctrl_error(env, "closeWindowsByPid", st);
        return failure;
    }

    return success;
}

/*
int window_state (Display *disp, Window win, unsigned long action, 
        char *prop1_str, char *prop2_str);
int window_set_title(Display *disp, Window win,
        const char *_net_wm_name);
int window_set_icon_name(Display *disp, Window win, 
        const char *_net_wm_icon_name);
Window Select_Window(Display *dpy);*/