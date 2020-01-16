#include "./wmctrl.h"

static bool activate_window(Display *disp, Window win,
        bool switch_desktop) {
    unsigned long *desktop;
    /* desktop ID */
    if ((desktop = (unsigned long *)get_property(disp, win,
            XA_CARDINAL, "_NET_WM_DESKTOP", NULL)) == NULL) {
        if ((desktop = (unsigned long *)get_property(disp, win,
                XA_CARDINAL, "_WIN_WORKSPACE", NULL)) == NULL) {
            //printf("Cannot find desktop ID of the window.\n");
        }
    }

    if (switch_desktop && desktop) {
        if (client_msg(disp, DefaultRootWindow(disp), 
                    "_NET_CURRENT_DESKTOP", 
                    *desktop, 0, 0, 0, 0) != True) {
            printf("Cannot switch desktop.\n");
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
    bool dispLocal;
    disp = create_display(disp, &dispLocal);
    if (!disp)
        return CAN_NOT_OPEN_DISPLAY;

    bool res = activate_window(disp, wid, false);
    free_local_display(disp, dispLocal);
    return res ? WINDOW_ACTIVATED : CAN_NOT_ACTIVATE_WINDOW;
}

enum STATES active_windows_by_pid(Display *disp, unsigned long pid) {
    bool dispLocal;
    disp = create_display(disp, &dispLocal);
    if (!disp)
        return CAN_NOT_OPEN_DISPLAY;

    size_t size = 0;
    Window *windows = get_client_list(disp, &size);
    size /= 8;
    if (!windows) {
        free_local_display(disp, dispLocal);
        return CAN_NOT_ALLOCATE_MEMORY;
    }

    if (size > 50) {
        free(windows);
        free_local_display(disp, dispLocal);
        return TOO_MANY_WINDOW_OPENED;
    }

    bool window_found = false;
    for (size_t i = 0; i < size; i++) {
        Window win = windows[i];
        size_t current_pid = get_window_pid(disp, win);

        if (current_pid == pid) {
            active_window_by_id(disp, win);
            window_found = true;
        }
    }

    free(windows);
    free_local_display(disp, dispLocal);

    if (!window_found)
        return NO_WINDOW_FOUND;

    return WINDOWS_ACTIVATED;
}

enum STATES active_windows_by_class_name(Display *disp, char *class_name) {
    bool dispLocal;
    disp = create_display(disp, &dispLocal);
    if (!disp)
        return CAN_NOT_OPEN_DISPLAY;

    size_t size = 0;
    Window *windows = get_client_list(disp, &size);
    size /= 8;
    if (!windows) {
        free_local_display(disp, dispLocal);
        return CAN_NOT_ALLOCATE_MEMORY;
    }

    if (size > 50) {
        free(windows);
        free_local_display(disp, dispLocal);
        return TOO_MANY_WINDOW_OPENED;
    }

    bool window_found = false;
    for (size_t i = 0; i < size; i++) {
        Window win = windows[i];
        char *current_class_name = get_window_class(disp, win);
        if (!current_class_name)
            continue;

        if (strcmp(current_class_name, class_name) == 0) {
            active_window_by_id(disp, win);
            window_found = true;
        }
        free(current_class_name);
    }

    free(windows);
    free_local_display(disp, dispLocal);

    if (!window_found)
        return NO_WINDOW_FOUND;

    return WINDOWS_ACTIVATED;
}

//CLOSE SECTION
enum STATES close_window_by_id(Display *disp, Window win) {
    bool dispLocal;
    disp = create_display(disp, &dispLocal);
    if (!disp)
        return CAN_NOT_OPEN_DISPLAY;

    bool res = client_msg(disp, win, "_NET_CLOSE_WINDOW", 
            0, 0, 0, 0, 0);
    free_local_display(disp, dispLocal);
    return res ? WINDOW_CLOSED : CAN_NOT_CLOSE_WINDOW;
}

static enum STATES close_windows_by(Display *disp, char mode, void *data) {
    size_t size = 0;
    Window *windows = get_client_list(disp, &size);
    size /= 8;
    if (!windows)
        return CAN_NOT_GET_CLIENT_LIST;

    if (size > 50) {
        free(windows);
        return TOO_MANY_WINDOW_OPENED;
    }

    bool win_found = false;
    switch (mode) {
        case 'p': {
            size_t pid = *((size_t*) data);
            for (size_t i = 0; i < size; i++) {
                Window win = windows[i];
                size_t current_pid = get_window_pid(disp, win);
                if (pid == current_pid) {
                    close_window_by_id(disp, win);
                    win_found = true;
                }
            }
            break;
        }
        case 'c': {
            char *class_name = data;
            for (size_t i = 0; i < size; i++) {
                Window win = windows[i];
                char *current_class_name = get_window_class(disp, win);
                if (!current_class_name)
                    continue;
                if (strcmp(class_name, current_class_name) == 0) {
                    close_window_by_id(disp, win);
                    win_found = true;
                }
                free(current_class_name);
            }
            break;
        }
    }

    free(windows);
    if (!win_found)
        return NO_WINDOW_FOUND;
    return WINDOWS_CLOSED;
}

enum STATES close_windows_by_pid(Display *disp, unsigned long pid) {
    bool dispLocal;
    enum STATES st;
    disp = create_display(disp, &dispLocal);
    if (!disp)
        return CAN_NOT_OPEN_DISPLAY;
    st = close_windows_by(disp, 'p', &pid);
    free_local_display(disp, dispLocal);
    return st;
}

enum STATES close_windows_by_class_name(Display *disp, char *class_name) {
    bool dispLocal;
    enum STATES st;
    disp = create_display(disp, &dispLocal);
    if (!disp)
        return CAN_NOT_OPEN_DISPLAY;

    st = close_windows_by(disp, 'c',class_name);
    free_local_display(disp, dispLocal);
    return st;
}

//EDIT PROPERTIES SECTION
enum STATES  window_set_icon_name(Display *disp, Window win, 
    const char *_net_wm_icon_name) {
    bool dispLocal;
    disp = create_display(disp, &dispLocal);
    if (!disp)
        return CAN_NOT_OPEN_DISPLAY;
    int res =  XChangeProperty(disp, win, XInternAtom(disp, "_NET_WM_ICON_NAME", False), 
            XInternAtom(disp, "UTF8_STRING", False), 8, PropModeReplace,
            (const unsigned char *)_net_wm_icon_name, strlen(_net_wm_icon_name));
    free_local_display(disp, dispLocal);
    return res ? WINDOW_ICON_NAME_SET : CAN_NOT_SET_WINDOW_ICON_NAME;
}

enum STATES window_set_title(Display *disp, Window win,
    const char *_net_wm_name) {
    bool dispLocal;
    disp = create_display(disp, &dispLocal);
    if (!disp)
        return CAN_NOT_OPEN_DISPLAY;
    int res = XChangeProperty(disp, win, XInternAtom(disp, "_NET_WM_NAME", False), 
            XInternAtom(disp, "UTF8_STRING", False), 8, PropModeReplace,
            (const unsigned char *)_net_wm_name, strlen(_net_wm_name));
    free_local_display(disp, dispLocal);
    return res ? WINDOW_TITLE_SET : CAN_NOT_SET_WINDOW_TITLE;
}

/**
 * action _NET_WM_STATE_REMOVE | _NET_WM_STATE_ADD | _NET_WM_STATE_TOGGLE
 * prop1, prop2: modal_sticky | maximized_vert | maximized_horz |
    shaded | skip_taskbar | skip_pager | hidden | fullscreen | above
**/
enum STATES window_state(Display *disp, Window win, unsigned long action, 
        char *prop1_str, char *prop2_str) {
    bool dispLocal;
    disp = create_display(disp, &dispLocal);
    if (!disp)
        return CAN_NOT_OPEN_DISPLAY;

    char *uppercase_prop1 = strdup(prop1_str);
    for (size_t i = 0; uppercase_prop1[i]; i++)
        uppercase_prop1[i] = toupper(uppercase_prop1[i]);

    char *tmp_prop1 = malloc(sizeof(char) * (strlen(uppercase_prop1) + 64));
    sprintf(tmp_prop1, "_NET_WM_STATE_%s", uppercase_prop1);

    Atom prop1 = XInternAtom(disp, tmp_prop1, False);
    free(uppercase_prop1);
    free(tmp_prop1);

    Atom prop2 = 0;
    if (prop2_str) {
        char *uppercase_prop2 = strdup(prop2_str);
        for (size_t i = 0; uppercase_prop2[i]; i++)
            uppercase_prop2[i] = toupper(uppercase_prop2[i]);
        char *tmp_prop2 = malloc(sizeof(char) * (strlen(prop2_str) + 64));
        sprintf(tmp_prop2, "_NET_WM_STATE_%s", uppercase_prop2);

        prop2 = XInternAtom(disp, tmp_prop2, False);
        free(uppercase_prop2);
        free(tmp_prop2);
    }

    bool res = client_msg(disp, win, "_NET_WM_STATE", 
        action, (unsigned long)prop1, (unsigned long)prop2, 0, 0);

    free_local_display(disp, dispLocal);
    return res ? WINDOW_STATE_SET : CAN_NOT_SET_WINDOW_STATE;
}

enum STATES change_viewport (Display *disp, unsigned long x, unsigned long y) {
    bool dispLocal;
    disp = create_display(disp, &dispLocal);
    if (!disp)
        return CAN_NOT_OPEN_DISPLAY;

    bool res = client_msg(disp, DefaultRootWindow(disp), "_NET_DESKTOP_VIEWPORT", 
        x, y, 0, 0, 0);

    free_local_display(disp, dispLocal);
    return res ? VIEWPORT_CHANGED : CAN_NOT_CHANGE_VIEWPORT;
}

enum STATES change_geometry (Display *disp, unsigned long x, unsigned long y) {
    bool dispLocal;
    disp = create_display(disp, &dispLocal);
    if (!disp)
        return CAN_NOT_OPEN_DISPLAY;
    bool res = client_msg(disp, DefaultRootWindow(disp), "_NET_DESKTOP_GEOMETRY", 
        x, y, 0, 0, 0);
    return res ? GEOMETRY_CHANGED : CAN_NOT_CHANGE_GEOMETRY;
}

/*
int action_window (Display *disp, Window win, char mode) {

        
        case 't':
            // move the window to the specified desktop => -r -t /
            return window_to_desktop(disp, win, atoi(options.param));
        
        case 'R':
            // move the window to the current desktop and activate it => -r /
            if (window_to_desktop(disp, win, -1) == EXIT_SUCCESS) {
                usleep(100000); // 100 ms - make sure the WM has enough
                    //time to move the window, before we activate it /
                return activate_window(disp, win, FALSE);
            }
            else {
                return EXIT_FAILURE;
            }

        case 'N': case 'I': case 'T':
            window_set_title(disp, win, options.param, mode);
            return EXIT_SUCCESS;

        default:
            fprintf(stderr, "Unknown action: '%c'\n", mode);
            return EXIT_FAILURE;
    }
}

static int action_window_pid (Display *disp, char mode) {
    unsigned long wid;

    if (sscanf(options.param_window, "0x%lx", &wid) != 1 &&
            sscanf(options.param_window, "0X%lx", &wid) != 1 &&
            sscanf(options.param_window, "%lu", &wid) != 1) {
        fputs("Cannot convert argument to number.\n", stderr);
        return EXIT_FAILURE;
    }

    return action_window(disp, (Window)wid, mode);
}

static int action_window_str (Display *disp, char mode) {
    Window activate = 0;
    Window *client_list;
    unsigned long client_list_size;
    int i;
    
    if (strcmp(SELECT_WINDOW_MAGIC, options.param_window) == 0) {
        activate = Select_Window(disp);
        if (activate) {
            return action_window(disp, activate, mode);
        }
        else {
            return EXIT_FAILURE;
        }
    }
    if (strcmp(ACTIVE_WINDOW_MAGIC, options.param_window) == 0) {
        activate = get_active_window(disp);
        if (activate)
        {
            return action_window(disp, activate, mode);
        }
        else
        {
            return EXIT_FAILURE;
        }
    }
    else {
        if ((client_list = get_client_list(disp, &client_list_size)) == NULL) {
            return EXIT_FAILURE; 
        }
        
        for (i = 0; i < client_list_size / sizeof(Window); i++) {
 			char *match_utf8;
 			if (options.show_class) {
 	            match_utf8 = get_window_class(disp, client_list[i]); // UTF8 /
 			}
 			else {
 				match_utf8 = get_window_title(disp, client_list[i]); // UTF8 /
 			}
            if (match_utf8) {
                char *match;
                char *match_cf;
                char *match_utf8_cf = NULL;
                if (envir_utf8) {
                    match = g_strdup(options.param_window);
                    match_cf = g_utf8_casefold(options.param_window, -1);
                }
                else {
                    if (! (match = g_locale_to_utf8(options.param_window, -1, NULL, NULL, NULL))) {
                        match = g_strdup(options.param_window);
                    }
                    match_cf = g_utf8_casefold(match, -1);
                }
                
                if (!match || !match_cf) {
                    continue;
                }

                match_utf8_cf = g_utf8_casefold(match_utf8, -1);

                if ((options.full_window_title_match && strcmp(match_utf8, match) == 0) ||
                        (!options.full_window_title_match && strstr(match_utf8_cf, match_cf))) {
                    activate = client_list[i];
                    g_free(match);
                    g_free(match_cf);
                    g_free(match_utf8);
                    g_free(match_utf8_cf);
                    break;
                }
                g_free(match);
                g_free(match_cf);
                g_free(match_utf8);
                g_free(match_utf8_cf);
            }
        }
        g_free(client_list);

        if (activate) {
            return action_window(disp, activate, mode);
        }
        else {
            return EXIT_FAILURE;
        }
    }
}*/