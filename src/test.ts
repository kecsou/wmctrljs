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

closeWindowById(4);

const winProcess = "xclock";
const class_name = "xclock.XClock";

const exec = promisify(cp.exec);

function wait(time:number):Promise<void> {
    return new Promise((resolve) => {
        setTimeout(() => { resolve(); }, time * 1000);
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
    try {
        console.time("closeClock");
        closeWindowsByClassName(class_name);
        console.timeEnd("closeClock");
    }catch(e) {
        console.error(e);
    }

    for (let i = 0; i < 50; i++)
        exec(winProcess);
    await wait(5);

    console.time("getActiveWindow");
    getActiveWindow();
    console.timeEnd("getActiveWindow");

    console.time("activeWindowsByClassName");
    activeWindowsByClassName(class_name);
    console.timeEnd("activeWindowsByClassName");
    await wait(1);

    const xclockWindows = getWindowsByClassName(class_name);
    if (xclockWindows) {
        const winXclock = xclockWindows[0];
        console.time("getWindowsByPid");
        getWindowsByPid(winXclock.win_pid);
        console.timeEnd("getWindowsByPid");
        await wait(1);

        console.time("activeWindowById");
        activeWindowById(winXclock.win_id);
        console.timeEnd("activeWindowById");
        await wait(1);

        console.time("activeWindowsByPid");
        activeWindowsByPid(winXclock.win_pid);
        console.timeEnd("activeWindowsByPid");
        await wait(1);

        for (let i = 0; i < xclockWindows.length; i++) {
            const win = xclockWindows[i];
            if (i % 2 === 0) {
                try {
                    console.time("closeWindowById");
                    closeWindowById(win.win_id);
                    console.timeEnd("closeWindowById");
                } catch(e) {
                    console.error(e);
                }
            }
            else {
                try {
                    console.time("closeWindowsByPid");
                    closeWindowsByPid(win.win_pid);
                    console.timeEnd("closeWindowsByPid");
                } catch(e) {
                    console.error(e);
                }
            }
            await wait(0.5);
        }

        try {
            console.time("closeWindowsByClassName");
            closeWindowsByClassName(class_name);
            console.timeEnd("closeWindowsByClassName");
        }
        catch(e) {
            console.error(e);
        }

        exec("xeyes");
        await wait(2);

        try {
            console.time("closeWindowsByClassName");
            closeWindowsByClassName("xeyes.XEyes");
            console.timeEnd("closeWindowsByClassName");
        }
        catch(e) {
            console.error(e);
        }
    }
})();