#include "wmctrl.h"

enum STATES close_window_by_id(Display *disp, Window win) {
    bool res;
    bool displayProvided = true;

    if (!disp) {
        disp = XOpenDisplay(NULL);
        displayProvided = false;
    }

    if (!disp)
        return CAN_NOT_OPEN_DISPLAY;

    res = client_msg(disp, win, "_NET_CLOSE_WINDOW", 
            0, 0, 0, 0, 0);

    if (!displayProvided)
        XCloseDisplay(disp);

    return res ? WINDOW_CLOSED : CAN_NOT_CLOSE_WINDOW;
}

static enum STATES close_windows_by(Display *disp, char mode, const void *data) {
    unsigned long size;
    unsigned long current_pid;
    unsigned long pid;
    const char *class_name;
    char *current_class_name;
    Window win;
    struct window_cache *wc;
    struct window_cache_list *wcl = init_window_list_cache();
    Window *windows = get_client_list(disp, &size);
    bool win_found = false;

    if (!wcl) {
        if (windows)
            free(windows);
        return CAN_NOT_ALLOCATE_MEMORY;
    }

    if (!windows) {
        free_window_cache_list(wcl);
        return CAN_NOT_GET_CLIENT_LIST;
    }

    if (!size) {
        free(windows);
        free_window_cache_list(wcl);
        return CAN_NOT_GET_CLIENT_LIST;
    }

    size /= sizeof(Window);

    switch (mode) {
        case 'p': {
            pid = *((unsigned long*) data);
            for (size_t i = 0; i < size; i++) {
                win = windows[i];
                wc = get_window_cache_by_id(wcl, win);
                if (wc && wc->win_pid)
                    current_pid = wc->win_pid;
                else {
                    current_pid = get_window_pid(disp, win);
                    if (current_pid)
                        add_window_cache(wcl, win, NULL, current_pid);
                    else
                        continue;
                }

                if (pid == current_pid) {
                    close_window_by_id(disp, win);
                    XSync(disp, False);
                    free(windows);
                    windows = get_client_list(disp, &size);
                    size /= sizeof(Window);

                    i = 0;
                    win_found = true;

                    if (!windows)
                        return CAN_NOT_GET_CLIENT_LIST;
                }
            }
            break;
        }
        case 'c': {
            class_name = data;
            for (size_t i = 0; i < size; i++) {
                win = windows[i];
                wc = get_window_cache_by_id(wcl, win);
                if (wc && wc->win_class)
                    current_class_name = strdup(wc->win_class);
                else {
                    current_class_name = get_window_class(disp, win);
                    if (current_class_name)
                        add_window_cache(wcl, win, current_class_name, 0);
                    else
                        continue;
                }

                if (strcmp(class_name, current_class_name) == 0) {
                    close_window_by_id(disp, win);
                    XSync(disp, False);
                    free(windows);
                    windows = get_client_list(disp, &size);
                    size /= sizeof(Window);

                    i = 0;
                    win_found = true;
                }
                free(current_class_name);
            }
            break;
        }
    }
    free(windows);
    free_window_cache_list(wcl);

    if (!win_found)
        return NO_WINDOW_FOUND;

    return WINDOWS_CLOSED;
}

enum STATES close_windows_by_pid(Display *disp, unsigned long pid) {
    bool displayProvided = true;
    enum STATES st;

    if (!disp) {
        disp = XOpenDisplay(NULL);
        displayProvided = false;
    }

    if (!disp)
        return CAN_NOT_OPEN_DISPLAY;

    st = close_windows_by(disp, 'p', &pid);

    if (!displayProvided)
        XCloseDisplay(disp);

    return st;
}

enum STATES close_windows_by_class_name(Display *disp, const char *class_name) {
    enum STATES st;
    bool displayProvided = true;

    if (!disp) {
        disp = XOpenDisplay(NULL);
        displayProvided = false;
    }

    if (!disp)
        return CAN_NOT_OPEN_DISPLAY;

    st = close_windows_by(disp, 'c',class_name);

    if (!displayProvided)
        XCloseDisplay(disp);

    return st;
}