import {
    getScreenSync,
    getWindowListSync,
    getActiveWindowSync,
    getWindowsByPidSync,
    getWindowsByClassNameSync,
    activeWindowByIdSync,
    activeWindowsByPidSync,
    activeWindowsByClassNameSync,
    closeWindowByIdSync,
    closeWindowsByPidSync,
    closeWindowsByClassNameSync,
    windowMoveResizeSync,
    windowStateSync,
    windowMinimize
} from "./index";

import { promisify } from "util";
import * as cp from "child_process";

const winProcess = "xclock";
const class_name = "xclock.XClock";

const exec = promisify(cp.exec);

function wait(time:number):Promise<void> {
    return new Promise((resolve) => {
        setTimeout(() => { resolve(); }, time * 1000);
    });
}

(async () => {
    for(let i = 0; i < 100; i++) {
        console.time("closeWindowById");
        closeWindowByIdSync(4);
        console.timeEnd("closeWindowById");
    }
 
    for(let i = 0; i < 100; i++) {
        console.time("getScreen");
        const screen = getScreenSync();
        console.timeEnd("getScreen");
        console.log(screen);
    }

    for(let i = 0; i < 100; i++) {
        console.time("getWindowList");
        getWindowListSync();
        console.timeEnd("getWindowList");
    }

    for(let i = 0; i < 100; i++) {    
        try {
            closeWindowsByClassNameSync(class_name);
        }catch(e) {
            //Voluntary error [Must failed]
            console.error("Voluntary error [Must failed]", e.message);
        }
    }

    for (let i = 0; i < 50; i++) {
        exec(winProcess);
        await wait(0.1);
    }
    await wait(5);

    console.time("getWindowsByClassName");
    const xclockWindows = getWindowsByClassNameSync(class_name);
    console.timeEnd("getWindowsByClassName");
    if (!xclockWindows)
        throw("Unexpected error xclockWinsows");

    console.log(`${xclockWindows.length} windows are open with the class_name ${class_name}`);

    try {
        console.time("activeWindowsByClassName");
        activeWindowsByClassNameSync(class_name);
        console.timeEnd("activeWindowsByClassName");
    } catch (e) {
        console.log(`Voluntary error [Must failed][activeWindowsByClassName] ${e.message}`);
        getWindowsByClassNameSync(class_name)
            .forEach(win => activeWindowByIdSync(win.win_id));
    }
    await wait(1);

    for (const winXclock of xclockWindows) {
        console.time("closeWindowsByPid");
        closeWindowsByPidSync(winXclock.win_pid);
        console.timeEnd("closeWindowsByPid");
        await wait(0.1);
    }
    await wait(1);

    try {
        closeWindowsByClassNameSync(class_name);
    }
    catch(e) {
        //Voluntary error [Must failed]
        console.error("Voluntary error [Must failed]", e.message);
    }
    await wait(1);

    for (let i = 0; i < 100; i++) {
        try {
            console.time("getActiveWindow");
            getActiveWindowSync();
            console.timeEnd("getActiveWindow");
        } catch (e) {
            //Voluntary error [Must failed]
            console.error("Voluntary error [Must failed]", e.message);
            console.timeEnd("getActiveWindow");  
        }
    }

    exec("xeyes");
    await wait(2);

    for (let i = 0; i < 100; i++) {
        console.time("activeWindowsByClassName");
        activeWindowsByClassNameSync("xeyes.XEyes");
        console.timeEnd("activeWindowsByClassName");
    }
    await wait(1);

    for (let i = 0; i < 100; i++) {
        try {
            getActiveWindowSync();
        } catch(e) {
            //Voluntary error [Must failed]
            console.error("Voluntary error [Must failed]", e.message);
        }
    }
    const winsXeyes = getWindowsByClassNameSync("xeyes.XEyes");
    console.timeEnd('getWindowsByClassName');
    if (!winsXeyes || !winsXeyes.length)
        throw new Error("Unexpected empty array");
    await wait(1);

    console.time("windowMoveResize");
    windowMoveResizeSync(winsXeyes[0].win_id, 0, 0, 0, 500, 500);
    console.timeEnd("windowMoveResize");
    await wait(1);

    console.time("windowMoveResize");
    windowMoveResizeSync(winsXeyes[0].win_id, 0, 0, 0, 1000, 1000);
    console.timeEnd("windowMoveResize");
    await wait(1);

    console.time("windowState");
    windowStateSync(winsXeyes[0].win_id, "add", "maximized_vert", "maximized_horz");
    console.timeEnd("windowState");
    await wait(1);

    console.time("windowState");
    windowStateSync(winsXeyes[0].win_id, "remove", "maximized_vert", "maximized_horz");
    console.timeEnd("windowState");
    await wait(1);

    console.time("windowState");
    windowStateSync(winsXeyes[0].win_id, "add", "fullscreen");
    console.timeEnd("windowState");
    await wait(1);

    console.time("windowState");
    windowStateSync(winsXeyes[0].win_id, "remove", "fullscreen");
    console.timeEnd("windowState");
    await wait(1);

    console.time("windowMinimize");
    windowMinimize(winsXeyes[0].win_id);
    console.timeEnd("windowMinimize");
    await wait(1);

    console.time("activeWindowById");
    activeWindowByIdSync(winsXeyes[0].win_id);
    console.timeEnd("activeWindowById");

    console.time("windowState");
    windowStateSync(winsXeyes[0].win_id, "add", "fullscreen");
    console.timeEnd("windowState");
    await wait(1);

    console.time("closeWindowsByClassName");
    closeWindowsByClassNameSync("xeyes.XEyes");
    console.timeEnd("closeWindowsByClassName");
})()
.then(() => {
    console.log("ALL TEST ENDED WITH SUCESS");
})
.catch(e => {
    console.error("Unexpected error during tests", e);
})
.finally(() => {
    console.log("DONE!");
});