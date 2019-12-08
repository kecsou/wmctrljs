#include "../src/wmctrl.h"

void test_list_window(Display *disp) {
    struct window_list *wl = list_windows(disp);
    printf("Window number %ld\n", wl->client_list_size);
    for (int i = 0; i < wl->client_list_size; i++) {
        print_window_info(wl->client_list + i);
    }
    free_window_list(wl);
}

void debugend(char *test_name) {
    printf("\n------- %s ok\n\n", test_name);
}

int main(int argc, char **argv) { 
    Display *disp;
    
    if (! (disp = XOpenDisplay(NULL))) {
        fputs("Cannot open display. %s\n", stderr);
        return 1;
    }

    test_list_window(disp);
    debugend("list_window");

    struct window_info *wi = get_active_window(disp);
    unsigned long pid_active_window = wi->win_pid;
    debugend("get_active_window");

    struct window_list *wl = get_windows_by_pid(disp, pid_active_window);
    debugend("get_windows_by_pid");
    if (wl) {
        printf("Window number %ld\n", wl->client_list_size);
        for (size_t i = 0; i < wl->client_list_size; i++) {
            print_window_info(wl->client_list + i);
        }
    }
    free_window_list(wl);
    debugend("free_window_list");

    window_state(disp, wi->win_id, _NET_WM_STATE_REMOVE, "maximized_vert", "maximized_horz");
    debugend("window_state (REMOVE vert+horz)");
    window_set_title(disp, wi->win_id, "MY CUSTOM TITLE");
    active_windows_by_pid(disp, wi->win_pid);
    free_window_info(wi);
//    active_window_by_pid(disp, 20340);
//    debugend("active_window_by_pid");
    
//    sleep(5);

//    close_window_by_pid(disp, 20340);
//    debugend("close_window_by_pid");
/*
    struct window_info *wi = get_wm_info(disp);
    if (wi)
        free_window_info(wi);
    debugend("wm_info");*/
    /*
    char *class_name = "Navigator.Firefox";
    wl = get_windows_by_class_name(disp, class_name);
    if (wl) {
        for (size_t i = 0; i < wl->client_list_size; i++) {
            wi = wl->client_list + i;
            active_window_by_id(disp, wi->win_id);
            window_state(disp, wi->win_id, _NET_WM_STATE_REMOVE, "maximized_vert", "maximized_horz");
            window_move_resize(disp, wi->win_id, 0, 100, 100, 700, 700);
            //close_window_by_id(disp, wi->win_id);
        }
        free_window_list(wl);
    }else {
        printf("No window found for class_name [%s]\n", class_name);
    }*/
    //sleep(5);
//    close_window_by_class_name(disp, "Navigator.Firefox");
/*
    Window win = get_active_window(disp);
    print_window(disp, &win);
    debugend("get_active_window");

    active_window_by_id(disp, 0x03a015c0);
    debugend("active_window_by_id");

    close_window_by_id(disp, 0x03a015c0);
    debugend("close_window_by_id");*/
    XCloseDisplay(disp);
    return 0;
}