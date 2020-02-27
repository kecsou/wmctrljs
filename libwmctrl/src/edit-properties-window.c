#include "wmctrl.h"

//EDIT PROPERTIES SECTION
enum STATES  window_set_icon_name(Display *disp, Window win, const char *_net_wm_icon_name) {
    int res;
    bool displayProvided = true;

    if (!disp) {
        disp = XOpenDisplay(NULL);
        displayProvided = false;
    }

    if (!disp)
        return CAN_NOT_OPEN_DISPLAY;

    res =  XChangeProperty(disp, win, XInternAtom(disp, "_NET_WM_ICON_NAME", False), 
            XInternAtom(disp, "UTF8_STRING", False), 8, PropModeReplace,
            (const unsigned char *)_net_wm_icon_name, strlen(_net_wm_icon_name));

    if (!displayProvided)
        XCloseDisplay(disp);

    return res ? WINDOW_ICON_NAME_SET : CAN_NOT_SET_WINDOW_ICON_NAME;
}

enum STATES window_set_title(Display *disp, Window win, const char *_net_wm_name) {
    int res;
    bool displayProvided = true;

    if (!disp) {
        disp = XOpenDisplay(NULL);
        displayProvided = false;
    }

    if (!disp)
        return CAN_NOT_OPEN_DISPLAY;

    res = XChangeProperty(disp, win, XInternAtom(disp, "_NET_WM_NAME", False), 
            XInternAtom(disp, "UTF8_STRING", False), 8, PropModeReplace,
            (const unsigned char *)_net_wm_name, strlen(_net_wm_name));

    if (!displayProvided)
        XCloseDisplay(disp);

    return res ? WINDOW_TITLE_SET : CAN_NOT_SET_WINDOW_TITLE;
}

/**
 * action _NET_WM_STATE_REMOVE | _NET_WM_STATE_ADD | _NET_WM_STATE_TOGGLE
 * prop1, prop2: modal_sticky | maximized_vert | maximized_horz |
    shaded | skip_taskbar | skip_pager | hidden | fullscreen | above
**/
enum STATES window_state(Display *disp, Window win, unsigned long action, 
        char *prop1_str, char *prop2_str) {
    bool res;
    bool displayProvided = true;
    char *uppercase_prop1;
    char *uppercase_prop2;
    char *tmp_prop1;
    char *tmp_prop2;
    Atom prop1;
    Atom prop2;

    if (!disp) {
        disp = XOpenDisplay(NULL);
        displayProvided = false;
    }

    if (!disp)
        return CAN_NOT_OPEN_DISPLAY;

    uppercase_prop1 = strdup(prop1_str);
    for (size_t i = 0; uppercase_prop1[i]; i++)
        uppercase_prop1[i] = toupper(uppercase_prop1[i]);

    tmp_prop1 = malloc(sizeof(char) * (strlen(uppercase_prop1) + 64));
    sprintf(tmp_prop1, "_NET_WM_STATE_%s", uppercase_prop1);

    prop1 = XInternAtom(disp, tmp_prop1, False);
    free(uppercase_prop1);
    free(tmp_prop1);

    prop2 = 0;
    if (strcmp(prop2_str, "") != 0) {
        uppercase_prop2 = strdup(prop2_str);
        for (size_t i = 0; uppercase_prop2[i]; i++)
            uppercase_prop2[i] = toupper(uppercase_prop2[i]);
        tmp_prop2 = malloc(sizeof(char) * (strlen(prop2_str) + 64));
        sprintf(tmp_prop2, "_NET_WM_STATE_%s", uppercase_prop2);

        prop2 = XInternAtom(disp, tmp_prop2, False);
        free(uppercase_prop2);
        free(tmp_prop2);
    }

    res = client_msg(disp, win, "_NET_WM_STATE", 
        action, (unsigned long)prop1, (unsigned long)prop2, 0, 0);

    if (!displayProvided)
        XCloseDisplay(disp);

    return res ? WINDOW_STATE_SET : CAN_NOT_SET_WINDOW_STATE;
}

enum STATES change_viewport (Display *disp, unsigned long x, unsigned long y) {
    bool res;
    bool displayProvided = true;

    if (!disp) {
        disp = XOpenDisplay(NULL);
        displayProvided = false;
    }

    if (!disp)
        return CAN_NOT_OPEN_DISPLAY;

    res = client_msg(disp, DefaultRootWindow(disp), "_NET_DESKTOP_VIEWPORT", 
        x, y, 0, 0, 0);

    if (!displayProvided)
        XCloseDisplay(disp);

    return res ? VIEWPORT_CHANGED : CAN_NOT_CHANGE_VIEWPORT;
}

enum STATES change_geometry(Display *disp, unsigned long x, unsigned long y) {
    bool res;
    bool displayProvided = true;

    if (!disp) {
        disp = XOpenDisplay(NULL);
        displayProvided = false;
    }

    if (!disp)
        return CAN_NOT_OPEN_DISPLAY;

    res = client_msg(disp, DefaultRootWindow(disp), "_NET_DESKTOP_GEOMETRY", 
        x, y, 0, 0, 0);

    if (!displayProvided)
        XCloseDisplay(disp);

    return res ? GEOMETRY_CHANGED : CAN_NOT_CHANGE_GEOMETRY;
}

enum STATES window_allow_all_sizes(Display *disp, Window win) {
    XSizeHints *xsh;
    bool displayProvided = true;

    if (!disp) {
        disp = XOpenDisplay(NULL);
        displayProvided = false;
    }

    if (!disp)
        return CAN_NOT_OPEN_DISPLAY;

    xsh = XAllocSizeHints();
    if (!xsh)
        return CAN_NOT_ALLOCATE_MEMORY;

    xsh->min_width  = 0;
    xsh->min_height = 0;
    xsh->max_width  = 0;
    xsh->max_height = 0;

    XSetWMNormalHints(disp, win, xsh);

    if (!displayProvided)
        XCloseDisplay(disp);

    XFree(xsh);
    return WINDOW_ALLOWED_ALL_SIZE;
}

enum STATES window_raise(Display *disp, Window win) {
    bool displayProvided = true;
    if (!disp) {
        disp = XOpenDisplay(NULL);
        displayProvided = false;
    }

    if (!disp)
        return CAN_NOT_OPEN_DISPLAY;

    int res = XRaiseWindow(disp, win);

    if (displayProvided)
        XCloseDisplay(disp);

    return res ? WINDOW_RAISED : CAN_NOT_RAISE_WINDOW;
}