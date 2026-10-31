// Behaviour tests against a real X server with an EWMH window manager (CI: Xvfb + openbox).
// Skipped when DISPLAY is unset. Only ever acts on the fixture windows it spawns.
import { test, describe, before, after } from "node:test";
import * as assert from "node:assert/strict";
import { spawn, ChildProcess } from "node:child_process";
import { once } from "node:events";
import { createInterface } from "node:readline";
import * as path from "node:path";
import { setTimeout as sleep } from "node:timers/promises";
import * as wm from "../index";

const FIXTURE = process.env.WMCTRLJS_FIXTURE ?? path.join(__dirname, "../../native/target/release/examples/fixture");
const skip = !process.env.DISPLAY && "DISPLAY is not set";
// Screen 1 of the same X server, managed by its own window manager.
const skipScreen1 = !process.env.WMCTRLJS_SCREEN1 && "WMCTRLJS_SCREEN1 is not set";
const spawned:ChildProcess[] = [];

interface Fixture {
    ids:number[]
    id:number
    pid:number
    cls:string
    exited:Promise<unknown>
}

let serial = 0;
async function fixture(opts:{ count?:number, legacy?:boolean, instance?:string, screen1?:boolean } = {}):Promise<Fixture> {
    const instance = opts.instance ?? `wmctrljs-${process.pid}-${serial++}`;
    const args = [instance, `title ${instance}`, String(opts.count ?? 1), opts.legacy ? "legacy" : ""];
    const env = opts.screen1 ? { ...process.env, DISPLAY: process.env.WMCTRLJS_SCREEN1 } : process.env;
    const proc = spawn(FIXTURE, args, { stdio: ["ignore", "pipe", "inherit"], env });
    spawned.push(proc);
    const exited = once(proc, "exit");
    const ids:number[] = [];
    for await (const line of createInterface({ input: proc.stdout! })) {
        ids.push(Number(line));
        if (ids.length === (opts.count ?? 1))
            break;
    }
    // _NET_CLIENT_LIST only covers the default screen: on screen 1, wait for the WM's frame instead.
    await waitFor(() => ids.every(id => opts.screen1 ? wm.getWindowByIdSync(id).frame_extents : wm.getWindowListSync().some(w => w.win_id === id)));
    return { ids, id: ids[0], pid: proc.pid!, cls: `${instance}.Fixture`, exited };
}

async function waitFor<T>(fn:() => T, ms = 20000):Promise<T> {
    const end = Date.now() + ms;
    for (;;) {
        const v = fn();
        if (v)
            return v;
        if (Date.now() > end)
            throw new Error(`timed out waiting for ${fn}`);
        await sleep(20);
    }
}

const states = (id:number) => wm.getWindowByIdSync(id).win_states.map(s => s.flag);
const geometry = (id:number) => wm.getWindowByIdSync(id).win_geometry!;
// _NET_ACTIVE_WINDOW is briefly 0 while the WM hands focus over.
const active = () => { try { return wm.getActiveWindowSync().win_id; } catch { return 0; } };
const MISSING = 1; // never a client window id: X servers hand out ids from 0x200000

// A WM publishes _NET_CLIENT_LIST once it manages its screen; it starts slowly under emulation.
const wmReady = () => { try { return wm.getWindowListSync(); } catch { return false; } };
before(async () => {
    if (skip)
        return;
    await waitFor(wmReady, 60000);
    if (skipScreen1)
        return;
    // X has one input focus per display: the screen 1 WM takes it when it starts, so it must be up
    // before any test that expects a window of screen 0 to stay active.
    const display = process.env.DISPLAY;
    process.env.DISPLAY = process.env.WMCTRLJS_SCREEN1;
    try {
        await waitFor(wmReady, 60000);
    } finally {
        process.env.DISPLAY = display;
    }
});
after(() => spawned.forEach(p => p.kill()));

describe("X11", { skip }, () => {
    describe("screen", () => {
        test("sync and async agree on the screen", async () => {
            const screen = wm.getScreenSync();
            assert.deepEqual(Object.keys(screen), [
                "backing_store", "black_pixel", "cmap", "height", "max_maps", "mheight", "min_maps",
                "mwidth", "ndepths", "root_depth", "root_input_mask", "save_unders", "white_pixel", "width"
            ]);
            assert.ok(Object.values(screen).every(v => Number.isInteger(v) && v >= 0));
            assert.ok(screen.width > 0 && screen.height > 0 && screen.root_depth > 0 && screen.ndepths > 0);
            assert.notEqual(screen.white_pixel, screen.black_pixel);
            assert.deepEqual(await wm.getScreen(), screen);
        });
    });

    describe("reading windows", () => {
        test("getWindowById reports every fixture property", async () => {
            const f = await fixture();
            const win = wm.getWindowByIdSync(f.id);
            assert.deepEqual(Object.keys(win), [
                "win_id", "win_pid", "desktop_number", "showing_desktop", "win_client_machine", "win_class",
                "win_types", "win_actions", "win_states", "win_name", "win_visible_name", "win_icon_name",
                "win_visible_icon_name", "wm_normal_hints_supplied", "wm_hints_supplied", "win_geometry",
                "WM_NORMAL_HINTS", "WM_HINTS", "frame_extents"
            ]);
            assert.equal(win.win_id, f.id);
            assert.equal(win.win_pid, f.pid);
            assert.equal(win.win_class, f.cls);
            assert.equal(win.win_name, `title ${f.cls.replace(".Fixture", "")} ✓`);
            assert.ok(win.win_visible_name.startsWith(win.win_name));
            assert.equal(win.win_icon_name, "fixture-icon");
            assert.ok(win.win_visible_icon_name.startsWith("fixture-icon"));
            assert.equal(win.win_client_machine, "fixture-host");
            assert.equal(win.desktop_number, 0);
            assert.equal(win.showing_desktop, 0);
            assert.deepEqual(win.win_types.map(t => t.flag), ["_NET_WM_WINDOW_TYPE_NORMAL"]);
            assert.ok(win.win_types[0].number > 0);
            assert.ok(win.win_actions.some(a => a.flag === "_NET_WM_ACTION_CLOSE"));
            assert.ok(win.win_actions.every(a => a.flag.startsWith("_NET_WM_ACTION_") && a.number > 0));
            assert.deepEqual(win.win_states, []);
            assert.equal(win.wm_normal_hints_supplied, 1023);
            assert.deepEqual(win.WM_NORMAL_HINTS, {
                flags: 816, x: 0, y: 0, width: 0, height: 0,
                min_width: 100, min_height: 80, max_width: 1000, max_height: 800,
                width_inc: 0, height_inc: 0,
                min_aspect: { numerator: 0, denominator: 0 },
                max_aspect: { numerator: 0, denominator: 0 },
                base_width: 10, base_height: 20, win_gravity: 1
            });
            assert.equal(win.wm_hints_supplied, 0);
            assert.ok(Object.values(win.WM_HINTS!).every(v => typeof v === "object" || v === 0));
            assert.equal(win.win_geometry!.width, 300);
            assert.equal(win.win_geometry!.height, 200);
            const fe = win.frame_extents!;
            assert.ok([fe.left, fe.right, fe.top, fe.bottom].every(v => Number.isInteger(v) && v >= 0));
            // Client area sits inside the frame.
            assert.equal(win.win_geometry!.x, fe.left);
            assert.equal(win.win_geometry!.y, fe.top);
            assert.deepEqual(await wm.getWindowById(f.id), win);
        });

        test("legacy ICCCM-only clients", async () => {
            const f = await fixture({ legacy: true });
            const win = wm.getWindowByIdSync(f.id);
            assert.equal(win.win_name, `title ${f.cls.replace(".Fixture", "")}`); // from WM_NAME
            assert.equal(win.win_pid, 0);
            assert.equal(win.win_client_machine, "");
            assert.equal(win.win_icon_name, "");
            assert.deepEqual(win.win_types, []);
            assert.equal(win.wm_normal_hints_supplied, 255); // 15-long hints: no base size, no gravity
            assert.equal(win.WM_NORMAL_HINTS!.flags, 16);
            assert.equal(win.WM_NORMAL_HINTS!.min_width, 120);
            assert.equal(win.WM_NORMAL_HINTS!.min_height, 90);
            assert.equal(win.WM_NORMAL_HINTS!.base_width, 0);
            // Windows without _NET_WM_PID never match a pid lookup.
            assert.ok(!wm.getWindowsByPidSync(0).some(w => w.win_id === f.id));
            assert.deepEqual(wm.getWindowsByClassNameSync(f.cls).map(w => w.win_id), [f.id]);
        });

        test("a window that does not exist reads as empty", async () => {
            const win = wm.getWindowByIdSync(MISSING);
            assert.equal(win.win_id, MISSING);
            assert.equal(win.win_pid, 0);
            assert.equal(win.win_class, "");
            assert.equal(win.win_name, "");
            assert.deepEqual([win.win_types, win.win_actions, win.win_states], [[], [], []]);
            assert.equal(win.wm_normal_hints_supplied, 0);
            assert.equal(win.win_geometry, undefined);
            assert.equal(win.frame_extents, undefined);
            assert.deepEqual(await wm.getWindowById(MISSING), win);
        });

        test("getWindowList lists clients in mapping order", async () => {
            const a = await fixture();
            const b = await fixture();
            const list = wm.getWindowListSync();
            assert.deepEqual(await wm.getWindowList(), list);
            const ids = list.map(w => w.win_id);
            assert.ok(ids.indexOf(a.id) >= 0 && ids.indexOf(a.id) < ids.indexOf(b.id));
            assert.equal(new Set(ids).size, ids.length);
        });

        test("getWindowsByPid / getWindowsByClassName", async () => {
            const a = await fixture();
            const b = await fixture({ count: 3 });
            assert.deepEqual(wm.getWindowsByPidSync(a.pid).map(w => w.win_id), [a.id]);
            assert.deepEqual((await wm.getWindowsByPid(b.pid)).map(w => w.win_id), b.ids);
            assert.deepEqual(wm.getWindowsByClassNameSync(b.cls).map(w => w.win_id), b.ids);
            assert.deepEqual((await wm.getWindowsByClassName(a.cls)).map(w => w.win_id), [a.id]);
            // Each match is a full window object.
            assert.deepEqual(wm.getWindowsByPidSync(a.pid)[0], wm.getWindowByIdSync(a.id));
        });

        test("no match is an empty list", async () => {
            assert.deepEqual(wm.getWindowsByPidSync(0), []);
            assert.deepEqual(wm.getWindowsByPidSync(2 ** 31), []);
            assert.deepEqual(await wm.getWindowsByPid(0), []);
            assert.deepEqual(wm.getWindowsByClassNameSync("nope.Nope"), []);
            assert.deepEqual(wm.getWindowsByClassNameSync(""), []);
            assert.deepEqual(await wm.getWindowsByClassName("nope.Nope"), []);
        });

        test("class names match exactly", async () => {
            const f = await fixture();
            const instance = f.cls.replace(".Fixture", "");
            assert.deepEqual(wm.getWindowsByClassNameSync(instance), []);
            assert.deepEqual(wm.getWindowsByClassNameSync(f.cls.toUpperCase()), []);
            assert.deepEqual(wm.getWindowsByClassNameSync(`${f.cls} `), []);
        });

        test("many concurrent calls stay consistent", async () => {
            const f = await fixture();
            const one = wm.getWindowByIdSync(f.id);
            const results = await Promise.all(Array.from({ length: 200 }, (_, i) =>
                i % 3 === 0 ? wm.getWindowById(f.id) : i % 3 === 1 ? wm.getWindowList() : wm.getScreen()));
            results.forEach((r, i) => {
                if (i % 3 === 0)
                    assert.deepEqual(r, one);
                else if (i % 3 === 1)
                    assert.ok((r as wm.Window[]).some(w => w.win_id === f.id));
            });
            for (let i = 0; i < 200; i++)
                assert.equal(wm.getWindowByIdSync(f.id).win_id, f.id);
        });
    });

    describe("activation", () => {
        test("by id, pid and class name; getActiveWindow follows", async () => {
            const a = await fixture();
            const b = await fixture();
            assert.equal(wm.activeWindowByIdSync(a.id), true);
            await waitFor(() => active() === a.id);
            assert.equal(await wm.activeWindowById(b.id), true);
            await waitFor(() => active() === b.id);
            assert.equal(wm.activeWindowsByClassNameSync(a.cls), true);
            await waitFor(() => active() === a.id);
            assert.equal(await wm.activeWindowsByPid(b.pid), true);
            await waitFor(() => active() === b.id);
            assert.equal(wm.activeWindowsByPidSync(a.pid), true);
            await waitFor(() => active() === a.id);
            assert.equal(await wm.activeWindowsByClassName(b.cls), true);
            await waitFor(() => active() === b.id);
            assert.equal((await wm.getActiveWindow()).win_id, b.id);
            assert.deepEqual(wm.getActiveWindowSync(), wm.getWindowByIdSync(b.id));
        });

        test("every window of a pid gets activated", async () => {
            const f = await fixture({ count: 3 });
            assert.equal(wm.activeWindowsByPidSync(f.pid), true);
            await waitFor(() => f.ids.includes(active()));
        });

        test("activating a minimized window restores it", async () => {
            const f = await fixture();
            wm.windowMinimizeSync(f.id);
            await waitFor(() => states(f.id).includes("_NET_WM_STATE_HIDDEN"));
            wm.activeWindowByIdSync(f.id);
            await waitFor(() => !states(f.id).includes("_NET_WM_STATE_HIDDEN") && active() === f.id);
        });

        test("errors", async () => {
            const legacy = await fixture({ legacy: true });
            assert.throws(() => wm.activeWindowByIdSync(0), { message: "[activeWindowByIdSync] CAN_NOT_ACTIVATE_WINDOW (10)" });
            assert.throws(() => wm.activeWindowByIdSync(MISSING), { message: "[activeWindowByIdSync] CAN_NOT_ACTIVATE_WINDOW (10)" });
            await assert.rejects(wm.activeWindowById(0), { message: "[activeWindowById] CAN_NOT_ACTIVATE_WINDOW (10)" });
            assert.throws(() => wm.activeWindowsByPidSync(0), { message: "[activeWindowsByPidSync] NO_WINDOW_FOUND (24)" });
            await assert.rejects(wm.activeWindowsByPid(2 ** 31), { message: "[activeWindowsByPid] NO_WINDOW_FOUND (24)" });
            assert.throws(() => wm.activeWindowsByClassNameSync("nope.Nope"), { message: "[activeWindowsByClassNameSync] NO_WINDOW_FOUND (24)" });
            await assert.rejects(wm.activeWindowsByClassName("nope.Nope"), { message: "[activeWindowsByClassName] NO_WINDOW_FOUND (24)" });
            // Managed windows always carry _NET_WM_DESKTOP, even legacy ones.
            assert.equal(wm.activeWindowByIdSync(legacy.id), true);
        });
    });

    describe("move and resize", () => {
        test("windowMoveResize places the frame and sizes the client", async () => {
            const f = await fixture();
            assert.equal(wm.windowMoveResizeSync(f.id, 0, 50, 60, 400, 300), true);
            await waitFor(() => geometry(f.id).width === 400 && geometry(f.id).height === 300);
            const { left, top } = wm.getWindowByIdSync(f.id).frame_extents!;
            assert.equal(geometry(f.id).abs_x, 50 + left);
            assert.equal(geometry(f.id).abs_y, 60 + top);

            assert.equal(await wm.windowMoveResize(f.id, 0, 70, 80, 410, 310), true);
            await waitFor(() => geometry(f.id).width === 410 && geometry(f.id).abs_x === 70 + left);
        });

        test("-1 keeps the current value", async () => {
            const f = await fixture();
            wm.windowMoveResizeSync(f.id, 0, 100, 100, 300, 200);
            await waitFor(() => geometry(f.id).abs_x > 100);
            const before = geometry(f.id);
            wm.windowMoveResizeSync(f.id, 0, -1, -1, 500, -1);
            await waitFor(() => geometry(f.id).width === 500);
            assert.equal(geometry(f.id).height, before.height);
            assert.equal(geometry(f.id).abs_x, before.abs_x);
            wm.windowMoveResizeSync(f.id, 0, 150, -1, -1, -1);
            await waitFor(() => geometry(f.id).abs_x === before.abs_x + 50);
            assert.equal(geometry(f.id).abs_y, before.abs_y);
            assert.equal(geometry(f.id).width, 500);
        });

        test("every gravity is accepted", async () => {
            const f = await fixture();
            for (let gravity = 0; gravity <= 10; gravity++) {
                assert.equal(wm.windowMoveResizeSync(f.id, gravity, 100, 100, 200 + gravity, 150), true);
                await waitFor(() => geometry(f.id).width === 200 + gravity);
            }
        });

        test("size hints are enforced until windowAllowAllSizes", async () => {
            const f = await fixture();
            wm.windowMoveResizeSync(f.id, 0, 10, 10, 50, 40);
            await waitFor(() => geometry(f.id).abs_x < 50);
            assert.deepEqual([geometry(f.id).width, geometry(f.id).height], [100, 80]); // min size

            assert.equal(wm.windowAllowAllSizesSync(f.id), true);
            const hints = wm.getWindowByIdSync(f.id).WM_NORMAL_HINTS!;
            assert.equal(hints.flags, 0);
            assert.equal(hints.min_width, 0);
            assert.equal(hints.max_width, 0);
            wm.windowMoveResizeSync(f.id, 0, 10, 10, 50, 40);
            await waitFor(() => geometry(f.id).width === 50 && geometry(f.id).height === 40);
            assert.equal(await wm.windowAllowAllSizes(f.id), true);
        });

        test("windowMoveResizeTransition steps to the target", async () => {
            const f = await fixture();
            wm.windowMoveResizeSync(f.id, 0, 0, 0, 300, 200);
            await waitFor(() => geometry(f.id).width === 300);
            const start = Date.now();
            assert.equal(await wm.windowMoveResizeTransition(f.id, 0, 100, 100, 350, 250, 20, 50), true);
            assert.ok(Date.now() - start >= 40, "at least 2 steps of 20ms");
            await waitFor(() => geometry(f.id).width === 350 && geometry(f.id).height === 250);
            // Defaults: 1ms per step, 1px per step.
            assert.equal(await wm.windowMoveResizeTransition(f.id, 0, -1, -1, 360, -1), true);
            await waitFor(() => geometry(f.id).width === 360);
            assert.equal(geometry(f.id).height, 250);
            // pad 0 used to hang the C addon.
            assert.equal(await wm.windowMoveResizeTransition(f.id, 0, -1, -1, 362, -1, 0, 0), true);
            await waitFor(() => geometry(f.id).width === 362);
        });

        test("errors", async () => {
            const f = await fixture();
            assert.throws(() => wm.windowMoveResizeSync(f.id, -1, 0, 0, 10, 10), { message: "[windowMoveResizeSync] CAN_NOT_MOVE_RESIZE_WINDOW (27)" });
            await assert.rejects(wm.windowMoveResize(f.id, -1, 0, 0, 10, 10), { message: "[windowMoveResize] CAN_NOT_MOVE_RESIZE_WINDOW (27)" });
            await assert.rejects(wm.windowMoveResizeTransition(f.id, -1, 0, 0, 10, 10), { message: "[windowMoveResizeTransition] CAN_NOT_MOVE_RESIZE_WINDOW (27)" });
            await assert.rejects(wm.windowMoveResizeTransition(MISSING, 0, 0, 0, 10, 10), /^Error: \[windowMoveResizeTransition\] /);
        });
    });

    describe("states", () => {
        // openbox keeps sticky windows on a desktop rather than flagging them, so sticky is only sent.
        const honoured = ["modal", "maximized_vert", "maximized_horz", "shaded", "skip_taskbar", "skip_pager",
            "hidden", "fullscreen", "above", "below"];

        for (const prop of honoured)
            test(`add and remove ${prop}`, async () => {
                const f = await fixture();
                const flag = `_NET_WM_STATE_${prop.toUpperCase()}`;
                assert.equal(wm.windowStateSync(f.id, "add", prop), true);
                await waitFor(() => states(f.id).includes(flag));
                assert.equal(await wm.windowState(f.id, "remove", prop), true);
                await waitFor(() => !states(f.id).includes(flag));
            });

        test("sticky is accepted", async () => {
            const f = await fixture();
            assert.equal(wm.windowStateSync(f.id, "toggle", "sticky"), true);
            assert.equal(await wm.windowState(f.id, "toggle", "sticky"), true);
        });

        test("two properties at once", async () => {
            const f = await fixture();
            const maxed = ["_NET_WM_STATE_MAXIMIZED_VERT", "_NET_WM_STATE_MAXIMIZED_HORZ"];
            assert.equal(await wm.windowState(f.id, "add", "maximized_vert", "maximized_horz"), true);
            await waitFor(() => maxed.every(s => states(f.id).includes(s)));
            const screen = wm.getScreenSync();
            assert.ok(geometry(f.id).width > screen.width / 2);
            assert.equal(wm.windowStateSync(f.id, "remove", "maximized_vert", "maximized_horz"), true);
            await waitFor(() => !maxed.some(s => states(f.id).includes(s)));
        });

        test("toggle flips the state back and forth", async () => {
            const f = await fixture();
            assert.equal(wm.windowStateSync(f.id, "toggle", "above"), true);
            await waitFor(() => states(f.id).includes("_NET_WM_STATE_ABOVE"));
            assert.equal(wm.windowStateSync(f.id, "toggle", "above"), true);
            await waitFor(() => !states(f.id).includes("_NET_WM_STATE_ABOVE"));
        });

        test("actions and properties are case-insensitive", async () => {
            const f = await fixture();
            assert.equal(wm.windowStateSync(f.id, "Add", "SHADED"), true);
            await waitFor(() => states(f.id).includes("_NET_WM_STATE_SHADED"));
            assert.equal(wm.windowStateSync(f.id, "REMOVE", "Shaded"), true);
            await waitFor(() => !states(f.id).includes("_NET_WM_STATE_SHADED"));
        });

        test("fullscreen covers the screen", async () => {
            const f = await fixture();
            wm.windowAllowAllSizesSync(f.id); // the fixture's 1000x800 max size would cap it
            wm.windowStateSync(f.id, "add", "fullscreen");
            const screen = wm.getScreenSync();
            await waitFor(() => geometry(f.id).width === screen.width && geometry(f.id).height === screen.height);
        });
    });

    describe("minimize and raise", () => {
        test("windowMinimize hides the window", async () => {
            const f = await fixture();
            assert.equal(wm.windowMinimizeSync(f.id), true);
            await waitFor(() => states(f.id).includes("_NET_WM_STATE_HIDDEN"));
            const g = await fixture();
            assert.equal(await wm.windowMinimize(g.id), true);
            await waitFor(() => states(g.id).includes("_NET_WM_STATE_HIDDEN"));
        });

        test("windowMinimize needs an existing window", async () => {
            assert.throws(() => wm.windowMinimizeSync(MISSING), { message: "[windowMinimizeSync] CAN_NOT_GET_WINDOW_ATTRIBUTES (36)" });
            await assert.rejects(wm.windowMinimize(MISSING), { message: "[windowMinimize] CAN_NOT_GET_WINDOW_ATTRIBUTES (36)" });
        });

        test("windowRaise", async () => {
            const f = await fixture();
            assert.equal(wm.windowRaiseSync(f.id), true);
            assert.equal(await wm.windowRaise(f.id), true);
            // Like XRaiseWindow in the C addon, a missing window is not reported.
            assert.equal(wm.windowRaiseSync(MISSING), true);
        });
    });

    describe("a window on another screen", { skip: skipScreen1 }, () => {
        test("its absolute position is on its own root", async () => {
            const f = await fixture({ screen1: true });
            const { left, top } = wm.getWindowByIdSync(f.id).frame_extents!;
            // Translated to the default root instead, X answers 0,0.
            assert.ok(top > 0);
            assert.ok(geometry(f.id).abs_x >= left && geometry(f.id).abs_y >= top, JSON.stringify(geometry(f.id)));
        });

        test("windowMinimize reaches its window manager", async () => {
            const f = await fixture({ screen1: true });
            assert.equal(wm.windowMinimizeSync(f.id), true);
            await waitFor(() => states(f.id).includes("_NET_WM_STATE_HIDDEN"));
        });
    });

    describe("closing", () => {
        test("by id", async () => {
            const a = await fixture();
            const b = await fixture();
            assert.equal(wm.closeWindowByIdSync(a.id), true);
            await a.exited;
            assert.equal(await wm.closeWindowById(b.id), true);
            await b.exited;
            await waitFor(() => !wm.getWindowListSync().some(w => w.win_id === a.id || w.win_id === b.id));
        });

        test("by pid closes every window of the process", async () => {
            const a = await fixture({ count: 3 });
            const b = await fixture({ count: 2 });
            // The C addon threw on success here.
            assert.equal(wm.closeWindowsByPidSync(a.pid), true);
            await a.exited;
            assert.equal(await wm.closeWindowsByPid(b.pid), true);
            await b.exited;
        });

        test("by class name closes every matching process", async () => {
            const instance = `wmctrljs-shared-${process.pid}`;
            const a = await fixture({ instance });
            const b = await fixture({ instance, count: 2 });
            const other = await fixture();
            assert.equal(wm.closeWindowsByClassNameSync(a.cls), true);
            await Promise.all([a.exited, b.exited]);
            assert.ok(wm.getWindowListSync().some(w => w.win_id === other.id));
            const c = await fixture();
            assert.equal(await wm.closeWindowsByClassName(c.cls), true);
            await c.exited;
        });

        test("errors", async () => {
            assert.throws(() => wm.closeWindowsByPidSync(0), { message: "[closeWindowsByPidSync] NO_WINDOW_FOUND (24)" });
            await assert.rejects(wm.closeWindowsByPid(0), { message: "[closeWindowsByPid] NO_WINDOW_FOUND (24)" });
            assert.throws(() => wm.closeWindowsByClassNameSync("nope.Nope"), { message: "[closeWindowsByClassNameSync] NO_WINDOW_FOUND (24)" });
            await assert.rejects(wm.closeWindowsByClassName("nope.Nope"), { message: "[closeWindowsByClassName] NO_WINDOW_FOUND (24)" });
            // Like the C addon, closing by id only sends the request: a missing window is not reported.
            assert.equal(wm.closeWindowByIdSync(MISSING), true);
        });
    });
});
