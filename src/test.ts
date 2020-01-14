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
import { promisify } from "util";
import * as cp from "child_process";

const winProcess = "xclock";
const class_name = "xclock.XClock"; //"wmctrlTest.out.XClock";

const exec = promisify(cp.exec);

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
    getWindowList();
    console.timeEnd("getWindowList");
    console.time("closeClock");
    try {
        closeWindowsByClassName(class_name)
    }catch(e) {

    }

    console.timeEnd("closeClock");
    for (let i = 0; i < 20; i++)
        exec(winProcess);
    await wait(1000)
    const xclockWindows = getWindowsByClassName(class_name);
    if (xclockWindows) {
        console.time("getActiveWindow");
        getActiveWindow();
        console.timeEnd("getActiveWindow");

        console.time("activeWindowById");
        activeWindowById(xclockWindows[0].win_id);
        console.timeEnd("activeWindowById");
        //checkSucces("activeWindowById", success);
        await wait(1000);

        console.time("activeWindowsByClassName");
        activeWindowsByClassName(class_name);
        console.timeEnd("activeWindowsByClassName");
        //checkSucces("activeWindowsByClassName", success);
        await wait(1000);

        console.time("activeWindowsByPid");
        activeWindowsByPid(xclockWindows[1].win_pid);
        console.timeEnd("activeWindowsByPid");
        //checkSucces("activeWindowsByPid", success);
        await wait(1000);

        //console.time("getWindowsByPid");
        //getWindowsByPid(xclockWindows[10].win_pid);
        //console.timeEnd("getWindowsByPid");
        for (let i = 0; i < xclockWindows.length; i++) {
            const win = xclockWindows[i];
            if (i % 2 === 0)
                closeWindowById(win.win_id);
            else
                closeWindowsByPid(win.win_pid);
        }
        closeWindowsByClassName(class_name);
    }
})();