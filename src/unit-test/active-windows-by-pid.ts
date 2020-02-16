import {
    activeWindowsByPid,
    activeWindowsByPidSync,
    closeWindowsByPid,
    getWindowsByClassName
} from "../index";
import { createXeyes } from "./utils";

export const run = (async () => {
    try {
        activeWindowsByPidSync(0);
    } catch(e) {
        console.log("[activeWindowsByPidSync] Volountary error");
    }

    try {
        await activeWindowsByPid(0);
    } catch(e) {
        console.log("[activeWindowsByPid] Volountary error");
    }

    await createXeyes(1);
    const windows = await getWindowsByClassName("xeyes.XEyes");
    if (windows.length === 0) {
        console.error("Can't create xeyes windows");
        return;
    }

    const valid_win_pid = windows[0].win_id;
    const nbrTest = 100;
    console.time(`[activeWindowsByPidSync] duration for ${nbrTest}`);
    try {
        for (let i = 0; i < nbrTest; i++)
            activeWindowsByPidSync(valid_win_pid);
    } catch(e) {
        console.log("[activeWindowsByPidSync] Unexpected error", e);
    }
    console.timeEnd(`[activeWindowsByPidSync] duration for ${nbrTest}`);

    console.time(`[activeWindowsByPid] duration for ${nbrTest}`);
    try {
        const promises = [];
        for (let i = 0; i < nbrTest; i++)
            promises.push(activeWindowsByPid(valid_win_pid));
        await Promise.all(promises);
    } catch(e) {
        console.log("[activeWindowsByPid] Unexpected error", e);
    }
    console.timeEnd(`[activeWindowsByPid] duration for ${nbrTest}`);
    await closeWindowsByPid(valid_win_pid);
})();