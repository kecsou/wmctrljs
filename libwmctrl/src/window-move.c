#include "./wmctrl.h"

int window_to_desktop (Display *disp, Window win, int desktop) {
    unsigned long *cur_desktop = NULL;
    Window root = DefaultRootWindow(disp);

    if (desktop == -1) {
        if (! (cur_desktop = (unsigned long *)get_property(disp, root,
                XA_CARDINAL, "_NET_CURRENT_DESKTOP", NULL))) {
            if (! (cur_desktop = (unsigned long *)get_property(disp, root,
                    XA_CARDINAL, "_WIN_WORKSPACE", NULL))) {
                fputs("Cannot get current desktop properties. "
                      "(_NET_CURRENT_DESKTOP or _WIN_WORKSPACE property)"
                      "\n", stderr);
                return EXIT_FAILURE;
            }
        }
        desktop = *cur_desktop;
    }
    g_free(cur_desktop);

    return client_msg(disp, win, "_NET_WM_DESKTOP", (unsigned long)desktop,
            0, 0, 0, 0);
}

int window_to_current_desktop(Display *disp, Window win) {
    if (window_to_desktop(disp, win, -1) == EXIT_SUCCESS) {
        usleep(100000); // 100 ms - make sure the WM has enough
        //time to move the window, before we activate it /
        return active_window_by_id(disp, win);
    }
    return EXIT_FAILURE;
}

//_NET_MOVERESIZE_WINDOW
int window_move_resize (Display *disp, Window win, unsigned long grav, 
    unsigned long x, unsigned long y, 
    unsigned long w, unsigned long h) {
    unsigned long grflags;

    if (grav < 0)
        return EXIT_FAILURE;

    grflags = grav;
    if (x != -1) grflags |= (1 << 8);
    if (y != -1) grflags |= (1 << 9);
    if (w != -1) grflags |= (1 << 10);
    if (h != -1) grflags |= (1 << 11);

    if (wm_supports(disp, "_NET_MOVERESIZE_WINDOW")){
        return client_msg(disp, win, "_NET_MOVERESIZE_WINDOW", grflags, x, y, w, h);
    }
    else {
        if ((w < 1 || h < 1) && (x >= 0 && y >= 0)) {
            XMoveWindow(disp, win, x, y);
        }
        else if ((x < 0 || y < 0) && (w >= 1 && h >= -1)) {
            XResizeWindow(disp, win, w, h);
        }
        else if (x >= 0 && y >= 0 && w >= 1 && h >= 1) {
            XMoveResizeWindow(disp, win, x, y, w, h);
        }
        return EXIT_SUCCESS;
    }
}