#include "../src/wmctrl.h"
#include <unistd.h>

void test_list_window(Display *disp) {
    struct window_list *wl = list_windows(disp, NULL);
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
    char *winProcess = "xclock";
    char *class_name = "wmctrlTest.out.XClock";

    enum STATES st = close_windows_by_class_name(NULL, class_name);
    if (st != WINDOWS_CLOSED) {
        char *error = get_error_message(st);
        if (error) {
            printf("close of windows %s failed error:%s\n", class_name, error);
            free(error);
            if (st != NO_WINDOW_FOUND)
                return 1;
        }
        else
            printf("close of windows %s failed error:Unexpected", class_name);
    }

    for (int i = 0; i < 20; i++) {
        if (fork() == 0) {
            execvp(winProcess, argv);
            return 1;
        }
    }

    sleep(1);

    struct window_list *wl = get_windows_by_class_name(NULL, class_name, &st);
    if (!wl) {
        char *error = get_error_message(st);
        if (error) {
            printf("%s", error);
            free(error);
        }
        return 1;
    }


    printf("%ld windows are open with the class_name %s\n", wl->client_list_size, class_name);
    free_window_list(wl);

    close_windows_by_class_name(NULL, class_name);
    return 0;
}