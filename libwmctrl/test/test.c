#include "../src/wmctrl.h"
#include <unistd.h>
#include <stdio.h>
#include <time.h>
#include <string.h>

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

struct codeInfos {
    clock_t begin;
    char *id;
};

struct codeInfos **times = NULL;
size_t size_times = 512;

void timer(char *str) {
    if (!times) {
        times = calloc(sizeof(struct codeInfos *), size_times);
    }

    if (!str)
        return;

    size_t i = 0;
    for (; i < size_times; i++) {
        if (times[i] && strcmp(times[i]->id, str) == 0) {
            clock_t t = clock() - times[i]->begin; 
            double time_taken = ((double)t)/CLOCKS_PER_SEC;
            printf("%s: %fms\n", times[i]->id, time_taken * 1000);
            free(times[i]->id);
            free(times[i]);
            times[i] = NULL;
            break;
        }
    }

    //CREATE IF NOT EXIST
    if (i == size_times) {
        i = 0;
        for (; i < size_times; i++) {
            if (!times[i]) {
                times[i] = malloc(sizeof(struct codeInfos));
                times[i]->begin = clock();
                times[i]->id = strdup(str);
                break;
            }
        }
    }
}

int main(int argc, char **argv) { 
    char *winProcess = "xclock";
    char *class_name = "wmctrlTest.out.XClock";//"xclock.XClock";
    enum STATES st;
    timer("getWindowList");
    list_windows(NULL, &st);
    timer("getWindowList");

    timer("closeClock");
    st = close_windows_by_class_name(NULL, class_name);
    timer("closeClock");

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

    for (int i = 0; i < 40; i++) {
        if (fork() == 0) {
            execvp(winProcess, argv);
            return 1;
        }
    }

    sleep(1);

    timer("getWindowsByClassName");
    struct window_list *wl = get_windows_by_class_name(NULL, class_name, &st);
    timer("getWindowsByClassName");
    /*if (!wl) {
        char *error = get_error_message(st);
        if (error) {
            printf("%s", error);
            free(error);
        }
        return 1;
    }*/


    //printf("%ld windows are open with the class_name %s\n", wl->client_list_size, class_name);

    timer("getActiveWindow");
    struct window_info *wi = get_active_window(NULL, &st);
    timer("getActiveWindow");
    free_window_info(wi);

    timer("activeWindowsByClassName");
    active_windows_by_class_name(NULL, class_name);
    timer("activeWindowsByClassName");

    for (size_t i = 0; i < wl->client_list_size; i++) {
        struct window_info *wi = wl->client_list + i;
        if (i % 2 == 0) {
            timer("closeWindowById");
            close_window_by_id(NULL, wi->win_id);
            timer("closeWindowById");
        }
        else {
            timer("closeWindowsByPid");
            close_windows_by_pid(NULL, wi->win_pid);
            timer("closeWindowsByPid");
        }
    }
    timer("closeWindowsByClassName");
    close_windows_by_class_name(NULL, class_name);
    timer("closeWindowsByClassName");
    free_window_list(wl);
    return 0;
}