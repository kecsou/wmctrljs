#include "../src/wmctrl.h"

static void print_window(Display *disp, Window *win) {
    gchar *title_utf8 = get_window_title(disp, *win);
    if (title_utf8) {
        printf("| Title: %s\n", title_utf8);
        g_free(title_utf8);
    }

    gchar *window_class = get_window_class(disp, *win);
    if (window_class) {
        printf("| Class: %s\n", window_class);
        g_free(window_class);
    }
}

void test_list_window(Display *disp) {
    struct window_list *wl = list_windows(disp);
    printf("Window number %d\n", wl->client_list_size);
    for (int i = 0; i < wl->client_list_size; i++) {
        printf("\n");
        Window *win = wl->client_list + i;
        print_window(disp, win);
        printf("\n");
    }

    free_window_list(wl);
}

void debugend(char *test_name) {
    printf("\n-------TEST------ %s ended\n\n", test_name);
}

int main (int argc, char **argv) { 
    Display *disp;
    if (! (disp = XOpenDisplay(NULL))) {
        fputs("Cannot open display.\n", stderr);
        return 1;
    }

    test_list_window(disp);
    debugend("list_window");

    struct wm_info *wi = get_wm_info(disp);
    if (wi)
        free_wm_info(wi);

    debugend("wm_info");

    Window win = get_active_window(disp);
    print_window(disp, &win);
    debugend("get_active_window");

    active_window_by_id(disp, 0x03a015c0);
    debugend("active_window_by_id");

    close_window_by_id(disp, 0x03a015c0);
    debugend("close_window_by_id");
    XCloseDisplay(disp);
    return 0;
}
