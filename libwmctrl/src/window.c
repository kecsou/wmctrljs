#include "./wmctrl.h"

struct window_info *get_active_window(Display *disp, enum STATES *st) {
    char *prop;
    unsigned long size;
    Window ret = (Window)0;
    bool dispLocal;
    disp = create_display(disp, &dispLocal);
    if (!disp) {
        *st = CAN_NOT_OPEN_DISPLAY;
        return NULL;
    }

    prop = get_property(disp, DefaultRootWindow(disp), XA_WINDOW, 
                        "_NET_ACTIVE_WINDOW", &size);

    if (prop) {
        ret = *((Window*)prop);
        struct window_info *wi = NULL;
        if (ret) {
            wi = create_window_info(disp, ret);
            if (!wi)
                *st = CAN_NOT_ALLOCATE_MEMORY;
            *st = CLIENT_LIST_GET;
        }
        else
            *st = NO_WINDOW_ACTIVE_FOR_NOW;

        free(prop);
        free_local_display(disp, dispLocal);
        return wi;
    }
    else {
        *st = CAN_NOT_ACTIVATE_WINDOW;
        free_local_display(disp, dispLocal);
        return NULL;
    }
}

static struct window_list *get_windows_by(Display *disp, void *data, 
        int (*predicate)(struct window_info *wi, void *data), enum STATES *st) {
    enum STATES tmpState;
    struct window_list *wl = list_windows(disp, &tmpState);
    *st = tmpState;
    if (!wl)
        return NULL;
    struct window_info *newWinsInfos = malloc(sizeof(struct window_info) * wl->client_list_size);
    if (!newWinsInfos) {
        if (st)
            *st = CAN_NOT_ALLOCATE_MEMORY;
        return NULL;
    }
    unsigned long counter = 0;

    for (size_t i = 0; i < wl->client_list_size; i++) {
        struct window_info *wi = wl->client_list+i;
        if (predicate(wi, data)) {
            struct window_info *nwi = newWinsInfos + counter;
            initializeWindowInfo(nwi);
            copy_window_info(nwi, wi);
            counter++;
        }
    }

    if (counter == 0) {
        free(newWinsInfos);
        free_window_list(wl);
        return NULL;
    }

    for (size_t i = 0; i < wl->client_list_size; i++)
        free_window_info_properties(wl->client_list + i);
    free(wl->client_list);

    wl->client_list = newWinsInfos;
    wl->client_list_size = counter;
    return wl;
}

static int predicate_pid(struct window_info *wi, void *data) {
    unsigned long *pid = data;
    return wi->win_pid == *pid;
}
struct window_list *get_windows_by_pid(Display *disp, unsigned long pid, 
    enum STATES *st) {
    enum STATES tmpState;
    bool dispLocal;
    disp = create_display(disp, &dispLocal);
    if (!disp) {
        *st = CAN_NOT_OPEN_DISPLAY;
        return NULL;
    }
    struct window_list *wl = get_windows_by(disp, &pid, predicate_pid, &tmpState);
    if (st)
        *st = tmpState;
    free_local_display(disp, dispLocal);
    return wl;
}

static int predicate_class_name(struct window_info *wi, void *data) {
    char *class_name = data;
    return strcmp(wi->win_class, class_name) == 0;
}
struct window_list *get_windows_by_class_name(Display *disp, char *class_name, 
    enum STATES *st) {
    enum STATES tmpState;
    bool dispLocal;
    disp = create_display(disp, &dispLocal);
    if (!disp) {
        *st = CAN_NOT_OPEN_DISPLAY;
        return NULL;
    }
    struct window_list *wl = get_windows_by(disp, class_name, 
        predicate_class_name, &tmpState);
    if (st)
        *st = tmpState;
    free_local_display(disp, dispLocal);
    return wl; 
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