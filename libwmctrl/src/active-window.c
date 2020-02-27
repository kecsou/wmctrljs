#include "./wmctrl.h"

static bool activate_window(Display *disp, Window win,
        bool switch_desktop) {
    unsigned long *desktop;
    /* desktop ID */
    if ((desktop = (unsigned long *)get_property(disp, win,
            XA_CARDINAL, "_NET_WM_DESKTOP", NULL)) == NULL) {
        if ((desktop = (unsigned long *)get_property(disp, win,
                XA_CARDINAL, "_WIN_WORKSPACE", NULL)) == NULL) {
           return false;
        }
    }

    if (switch_desktop && desktop) {
        if (client_msg(disp, DefaultRootWindow(disp), 
                    "_NET_CURRENT_DESKTOP", 
                    *desktop, 0, 0, 0, 0) != True) {
            return false;
        }
    }

    XMapRaised(disp, win);
    if (desktop)
        free(desktop);

    return client_msg(disp, win, "_NET_ACTIVE_WINDOW", 
            0, 0, 0, 0, 0);
}

//ACTIVE SECTION
enum STATES active_window_by_id(Display *disp, Window wid) {
    bool res;
    bool displayProvided = true;

    if (!disp) {
        disp = XOpenDisplay(NULL);
        displayProvided = false;
    }

    if (!disp)
        return CAN_NOT_OPEN_DISPLAY;

    res = activate_window(disp, wid, false);

    if (!displayProvided)
        XCloseDisplay(disp);

    return res ? WINDOW_ACTIVATED : CAN_NOT_ACTIVATE_WINDOW;
}

enum STATES active_windows_by_pid(Display *disp, unsigned long pid) {
    size_t size;
    Window win;
    unsigned long current_pid;
    struct window_cache *wc;
    enum STATES st;
    Window *windows;
    struct window_cache_list *wcl = init_window_list_cache();
    bool displayProvided = true;
    bool window_found = false;

    if (!disp) {
        disp = XOpenDisplay(NULL);
        displayProvided = false;
    }

    if (!disp)
        return CAN_NOT_OPEN_DISPLAY;

    windows = get_client_list(disp, &size);

    if (!windows)
        return CAN_NOT_GET_CLIENT_LIST;

    size /= sizeof(Window);
    if (!windows) {
        if (!displayProvided)
            XCloseDisplay(disp);
        return CAN_NOT_ALLOCATE_MEMORY;
    }

    for (size_t i = 0; i < size; i++) {
        win = windows[i];
        wc = get_window_cache_by_id(wcl, win);

        if (wc && wc->win_pid) {
            current_pid = wc->win_pid;
        }
        else {
            current_pid = get_window_pid(disp, win);
            if (current_pid) {
                add_window_cache(wcl, win, NULL, current_pid);
            }
            else {
                continue;
            }
        }

        if (current_pid == pid) {
            st = active_window_by_id(disp, win);
            if (st != WINDOW_ACTIVATED)
                return CAN_NOT_ACTIVATE_WINDOW;
            XSync(disp, False);
            window_found = true;
        }
    }

    free(windows);
    free_window_cache_list(wcl);
    if (!displayProvided)
        XCloseDisplay(disp);

    if (!window_found)
        return NO_WINDOW_FOUND;

    return WINDOWS_ACTIVATED;
}

enum STATES active_windows_by_class_name(Display *disp, const char *class_name) {
    size_t size;
    Window win;
    Window *windows;
    char *current_class_name;
    struct window_cache *wc;
    enum STATES st;
    struct window_cache_list *wcl = init_window_list_cache();
    bool displayProvided = true;
    bool window_found = false;

    if (!disp) {
        disp = XOpenDisplay(NULL);
        displayProvided = false;
    }

    if (!disp)
        return CAN_NOT_OPEN_DISPLAY;

    windows = get_client_list(disp, &size);

    if (!windows)
        return CAN_NOT_GET_CLIENT_LIST;

    if (!wcl)
        return CAN_NOT_ALLOCATE_MEMORY;

    size /= sizeof(Window);
    if (!windows) {
        if (!displayProvided)
            XCloseDisplay(disp);
        return CAN_NOT_ALLOCATE_MEMORY;
    }

    for (size_t i = 0; i < size; i++) {
        win = windows[i];
        wc = get_window_cache_by_id(wcl, win);
        if (wc && wc->win_class)
            current_class_name = strdup(wc->win_class);
        else {
            current_class_name = get_window_class(disp, win);
            if (current_class_name)
                add_window_cache(wcl, win, current_class_name, 0);
            else {
                continue;
            }
        }

        if (strcmp(current_class_name, class_name) == 0) {
            st = active_window_by_id(disp, win);
            if (st != WINDOW_ACTIVATED)
                return CAN_NOT_ACTIVATE_WINDOW;
            XSync(disp, False);
            window_found = true;
        }

        free(current_class_name);
    }

    free(windows);
    free_window_cache_list(wcl);
    if (!displayProvided)
        XCloseDisplay(disp);

    if (!window_found)
        return NO_WINDOW_FOUND;

    return WINDOWS_ACTIVATED;
}