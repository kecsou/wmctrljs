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
    struct window_list *wl = NULL;
    timer("getWindowList");
    wl = list_windows(NULL, &st);
    timer("getWindowList");
    free_window_list(wl);

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

    for (int i = 0; i < 25; i++) {
        if (fork() == 0) {
            execvp(winProcess, argv);
            return 1;
        }
    }
    sleep(5);

    timer("getWindowsByClassName");
    wl = get_windows_by_class_name(NULL, class_name, &st);
    timer("getWindowsByClassName");
    if (!wl) {
        char *error = get_error_message(st);
        if (error) {
            printf("%s", error);
            free(error);
        }
        return 1;
    }

    printf("%ld windows are open with the class_name %s\n", wl->client_list_size, class_name);

    timer("activeWindowsByClassName");
    st = active_windows_by_class_name(NULL, class_name);
    timer("activeWindowsByClassName");
    if (st != WINDOWS_ACTIVATED) {
        char *error = get_error_message(st);
        if (error) {
            printf("[activeWindowsByClassName] %s\n", error);
            free(error);
        }else
            printf("[activeWindowsByClassName] Unexpected error");
    }

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

    timer("getActiveWindow");
    struct window_info *wi = get_active_window(NULL, &st);
    timer("getActiveWindow");
    free_window_info(wi);
    if (fork() == 0) {
        execvp("xeyes", argv);
        return 1;
    }
    sleep(2);

    timer("activeWindowsByClassName");
    active_windows_by_class_name(NULL, "wmctrlTest.out.XEyes");
    timer("activeWindowsByClassName");

    timer("getActiveWindow");
    wi = get_active_window(NULL, &st);
    timer("getActiveWindow");
    if (st != CLIENT_LIST_GET) {
        char *error = get_error_message(st);
        if (error) {
            printf("%s", error);
            free(error);
        }
    }
    free_window_info(wi);

    timer("getWindowsByClassName");
    wl = get_windows_by_class_name(NULL, "wmctrlTest.out.XEyes", &st);
    timer("getWindowsByClassName");
    if (!wl || st != CLIENT_LIST_GET) {
        char *error = get_error_message(st);
        if (error) {
            printf("%s", error);
            free(error);
        }
        return 1;
    }
 
    wi = wl->client_list;
    timer("windowMoveResize");
    st = window_move_resize(NULL, wi->win_id, 0, 0, 0, 500, 500);
    timer("windowMoveResize");
    if (st != WINDOW_MOVED_RESIZED) {
        char *error = get_error_message(st);
        if (error) {
            printf("%s", error);
            free(error);
        }
        return 1;
    }

    sleep(2);

    timer("windowMoveResize");
    st = window_move_resize(NULL, wi->win_id, 0, 0, 0, 1000, 1000);
    timer("windowMoveResize");
    if (st != WINDOW_MOVED_RESIZED) {
        char *error = get_error_message(st);
        if (error) {
            printf("%s", error);
            free(error);
        }
        return 1;
    }
    sleep(5);
    free_window_list(wl);

    timer("closeWindowsByClassName");
    st = close_windows_by_class_name(NULL, "wmctrlTest.out.XEyes");
    timer("closeWindowsByClassName");
    if (st != WINDOWS_CLOSED) {
        char *error = get_error_message(st);
        if (error) {
            printf("%s", error);
            free(error);
        }
        return 1;
    }

    return 0;
}