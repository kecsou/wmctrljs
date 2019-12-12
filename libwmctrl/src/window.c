#include "./wmctrl.h"

struct window_info *get_active_window(Display *disp) {
    char *prop;
    unsigned long size;
    Window ret = (Window)0;

    prop = get_property(disp, DefaultRootWindow(disp), XA_WINDOW, 
                        "_NET_ACTIVE_WINDOW", &size);
    if (prop) {
        ret = *((Window*)prop);
        free(prop);
    }

    return create_window_info(disp, ret);
}

static struct window_list *get_windows_by(Display *disp, void *data, 
        int (*predicate)(struct window_info *wi, void *data)) {
    struct window_list *wl = list_windows(disp);
    struct window_info *newWinInfos = malloc(sizeof(struct window_info) * wl->client_list_size);
    unsigned long counter = 0;

    for (size_t i = 0; i < wl->client_list_size; i++) {
        struct window_info *wi = wl->client_list+i;
        if (predicate(wi, data)) {
            copy_window_info(newWinInfos + counter, wi);
            counter++;
        }
    }

    if (counter == 0) {
        free(newWinInfos);
        free_window_list(wl);
        return NULL;
    }

    for (size_t i = 0; i < wl->client_list_size; i++)
        free_window_info_properties(wl->client_list + i);
    free(wl->client_list);

    wl->client_list = newWinInfos;
    wl->client_list_size = counter;
    return wl;
}

static int predicate_pid(struct window_info *wi, void *data) {
    unsigned long *pid = data;
    return wi->win_pid == *pid;
}
struct window_list *get_windows_by_pid(Display *disp, unsigned long pid) {
    return get_windows_by(disp, &pid, predicate_pid);
}

static int predicate_class_name(struct window_info *wi, void *data) {
    char *class_name = data;
    return strcmp(wi->win_class, class_name) == 0;
}
struct window_list *get_windows_by_class_name(Display *disp, char *class_name) {
    return get_windows_by(disp, class_name, predicate_class_name);
}

Window Select_Window(Display *dpy) {
    /*
     * Routine to let user select a window using the mouse
     * Taken from xfree86.
     */
    int status;
    Cursor cursor;
    XEvent event;
    Window target_win = None, root = DefaultRootWindow(dpy);
    int buttons = 0;
    int dummyi;
    unsigned int dummy;

    /* Make the target cursor */
    cursor = XCreateFontCursor(dpy, XC_crosshair);

    /* Grab the pointer using target cursor, letting it room all over */
    status = XGrabPointer(dpy, root, False,
            ButtonPressMask|ButtonReleaseMask, GrabModeSync,
            GrabModeAsync, root, cursor, CurrentTime);
    if (status != GrabSuccess) {
        fputs("ERROR: Cannot grab mouse.\n", stderr);
        return 0;
    }

    /* Let the user select a window... */
    while ((target_win == None) || (buttons != 0)) {
        /* allow one more event */
        XAllowEvents(dpy, SyncPointer, CurrentTime);
        XWindowEvent(dpy, root, ButtonPressMask|ButtonReleaseMask, &event);
        switch (event.type) {
            case ButtonPress:
                if (target_win == None) {
                    target_win = event.xbutton.subwindow; /* window selected */
                    if (target_win == None) target_win = root;
                }
                buttons++;
                break;
            case ButtonRelease:
                if (buttons > 0) /* there may have been some down before we started */
                    buttons--;
                break;
        }
    }

    XUngrabPointer(dpy, CurrentTime);      /* Done with pointer */

    if (XGetGeometry (dpy, target_win, &root, &dummyi, &dummyi,
                &dummy, &dummy, &dummy, &dummy) && target_win != root) {
        target_win = XmuClientWindow (dpy, target_win);
    }

    return(target_win);
}