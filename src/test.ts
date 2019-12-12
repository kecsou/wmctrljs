import {
    getWindowList,
    getActiveWindow,
    getWindowsByPid,
    getWindowsByClassName,
    activeWindowById,
    activeWindowsByPid,
    activeWindowsByClassName,
    closeWindowById,
    closeWindowsByPid,
    closeWindowsByClassName
} from "./index";

function wait(time:number):Promise<void> {
    return new Promise((resolve) => {
        setTimeout(() => { resolve(); }, time);
    });
}

function checkSucces(name:string, res:boolean) {
    console.log(`${name}: ${res ? "success" : "failed"}`);
}

(async () => {
    let success:boolean;
    console.time("getWindowList");
    const window_list = getWindowList();
    console.timeEnd("getWindowList");
    if (window_list) {

        console.time("getActiveWindow");
        getActiveWindow();
        console.timeEnd("getActiveWindow");
        const win = window_list[2];

        console.time("activeWindowById");
        activeWindowById(win.win_id);
        console.timeEnd("activeWindowById");
        //checkSucces("activeWindowById", success);
        await wait(1000);

        console.time("activeWindowsByClassName");
        activeWindowsByClassName("code.Code");
        console.timeEnd("activeWindowsByClassName");
        //checkSucces("activeWindowsByClassName", success);
        await wait(1000);

        console.time("activeWindowsByPid");
        activeWindowsByPid(win.win_pid);
        console.timeEnd("activeWindowsByPid");
        //checkSucces("activeWindowsByPid", success);
        await wait(1000);

        console.time("activeWindowsByClassName");
        activeWindowsByClassName("code.Code");
        console.timeEnd("activeWindowsByClassName");
        //checkSucces("activeWindowsByClassName", success);
        await wait(1000);

        console.time("getWindowsByClassName");
        getWindowsByClassName("code.Code");
        console.timeEnd("getWindowsByClassName");

        console.time("getWindowsByPid");
        getWindowsByPid(0);
        console.timeEnd("getWindowsByPid");
    }
})();