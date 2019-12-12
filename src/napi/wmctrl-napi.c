#include "wmctrl-napi.h"
#include <stdio.h>

napi_value getWindowList(napi_env env, napi_callback_info info) {
    Display *disp;
    if (! (disp = XOpenDisplay(NULL))) {
        napi_throw_error(env, NULL, "Can't open dispay");
        return NULL;
    }

    struct window_list *wl = list_windows(disp);
    napi_value wins_js;

    if (napi_create_array_with_length(env, wl->client_list_size, &wins_js) != napi_ok) {
        napi_throw_error(env, "ERROR", "Can't create array");
        return NULL;
    }

    for (uint32_t i = 0; i < wl->client_list_size; i++) {
        napi_value win_js; 
        if (!create_window(env, &win_js, wl->client_list + i)) {
            napi_throw_error(env, NULL, "Can't create win_js");
            return NULL;
        }
        napi_set_element(env, wins_js, i, win_js);
    }
    free_window_list(wl);
    XCloseDisplay(disp);
    return wins_js;
}

napi_value getActiveWindow(napi_env env, napi_callback_info info) {
    Display *disp;
    if (! (disp = XOpenDisplay(NULL))) {
        napi_throw_error(env, NULL, "Can't open dispay");
        return NULL;
    }

    struct window_info *wi = get_active_window(disp);
    if (!wi) {
        napi_throw_error(env, NULL, "Unexpected error can't get active window");
        return NULL;
    }

    napi_value win_js; 
    if (!create_window(env, &win_js, wi)) {
        napi_throw_error(env, NULL, "Can't create window_js");
        return NULL;
    }

    free_window_info(wi);
    XFree(disp);
    return win_js;
}

napi_value getWindowsByPid(napi_env env, napi_callback_info info) {
    napi_value argv[1];
    size_t argc = 1;

    if (napi_get_cb_info(env, info, &argc, argv, NULL, NULL) != napi_ok) {
        napi_throw_error(env, NULL, "Can't get function parameters");
        return NULL;
    }

    if (argc != 1) {
        napi_throw_error(env, "EINVAL", "Must have one arg pid_js [getWindowsByPid]");
        return NULL;
    }

    int32_t pid;
    if (napi_get_value_int32(env, argv[0], &pid) != napi_ok) {
        napi_throw_error(env, NULL, "Can't set input pid_js to C pid");
        return NULL;
    }
    Display *disp;
    if (! (disp = XOpenDisplay(NULL))) {
        napi_throw_error(env, NULL, "Can't open dispay");        
        return NULL;
    }

    struct window_list*wl = get_windows_by_pid(disp, pid);
    if (!wl) {
        napi_throw_error(env, NULL, "Unexpected error can't windows for this pid");
        return NULL;
    }

    napi_value wins_js;
    if (napi_create_array_with_length(env, wl->client_list_size, &wins_js) != napi_ok) {
        napi_throw_error(env, "ERROR", "Can't create array");
        return NULL;
    }

    for (size_t i = 0; i < wl->client_list_size; i++) {
        napi_value win_js; 
        if (!create_window(env, &win_js, wl->client_list + i)) {
            napi_throw_error(env, NULL, "Can't create win_js");
            return NULL;
        }
        napi_set_element(env, wins_js, i, win_js);
    }
    free_window_list(wl);
    XFree(disp);
    return wins_js;
}

napi_value getWindowsByClassName(napi_env env, napi_callback_info info) {
    napi_value argv[1];
    size_t argc = 1;    

    if (napi_get_cb_info(env, info, &argc, argv, NULL, NULL) != napi_ok) {
        napi_throw_error(env, NULL, "Can't get function parameters");
        return NULL;
    }

    if (argc != 1) {
        napi_throw_error(env, "EINVAL", "Must have one arg class_name_js [getWindowsByClassName]");
        return NULL;
    }

    char class_name[1024];
    size_t result;
    if (napi_get_value_string_utf8(env, argv[0], (char *)&class_name, 1024, &result) != napi_ok) {
        napi_throw_error(env, NULL, "Can't set input class_name_js to C class_name");
        return NULL;
    }

    Display *disp;
    if (! (disp = XOpenDisplay(NULL))) {
        napi_throw_error(env, NULL, "Can't open dispay");        
        return NULL;
    }

    struct window_list*wl = get_windows_by_class_name(disp, class_name);
    if (!wl) {
        napi_throw_error(env, NULL, "Unexpected error can't windows for this class_name");
        return NULL;
    }

    napi_value wins_js;
    if (napi_create_array_with_length(env, wl->client_list_size, &wins_js) != napi_ok) {
        napi_throw_error(env, "ERROR", "Can't create array");
        return NULL;
    }

    for (size_t i = 0; i < wl->client_list_size; i++) {
        napi_value win_js; 
        if (!create_window(env, &win_js, wl->client_list + i)) {
            napi_throw_error(env, NULL, "Can't create win_js");
            return NULL;
        }
        napi_set_element(env, wins_js, i, win_js);
    }
    free_window_list(wl);
    XFree(disp);
    return wins_js;
}

napi_value init_all (napi_env env, napi_value exports) {
    napi_value getWindowListFn;
    napi_value getActiveWindowFn;
    napi_value getWindowsByPidFn;
    napi_value getWindowsByClassNameFn;

    napi_create_function(env, NULL, 0, getWindowList, NULL, &getWindowListFn);
    napi_set_named_property(env, exports, "getWindowList", getWindowListFn);

    napi_create_function(env, NULL, 0, getActiveWindow, NULL, &getActiveWindowFn);
    napi_set_named_property(env, exports, "getActiveWindow", getActiveWindowFn);

    napi_create_function(env, NULL, 0, getWindowsByPid, NULL, &getWindowsByPidFn);
    napi_set_named_property(env, exports, "getWindowsByPid", getWindowsByPidFn);

    napi_create_function(env, NULL, 0, getWindowsByClassName, NULL, &getWindowsByClassNameFn);
    napi_set_named_property(env, exports, "getWindowsByClassName", getWindowsByClassNameFn);
    return exports;
}

NAPI_MODULE(NODE_GYP_MODULE_NAME, init_all)