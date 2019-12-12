#include "wmctrl-napi.h"

napi_value getWindowList(napi_env env, napi_callback_info info) {
    struct window_list *wl = list_windows(disp_client);
    if (!wl)
        return NULL;
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
        if (napi_set_element(env, wins_js, i, win_js) != napi_ok) {
            napi_throw_error(env, NULL, "Can't set element to win_js");
            return NULL;
        }
    }
    free_window_list(wl);
    return wins_js;
}

napi_value getActiveWindow(napi_env env, napi_callback_info info) {
    struct window_info *wi = get_active_window(disp_client);
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

    struct window_list*wl = get_windows_by_pid(disp_client, pid);
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
        if (napi_set_element(env, wins_js, i, win_js) != napi_ok) {
            napi_throw_error(env, NULL, "Can't set element to win_js");
            return NULL;
        }
    }
    free_window_list(wl);
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

    struct window_list*wl = get_windows_by_class_name(disp_client, class_name);
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
    return wins_js;
}