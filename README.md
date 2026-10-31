# wmctrljs

Native window management for Node.js on Linux X11 (works under XWayland too): `wmctrl` and more, without spawning a process.

The native part is written in Rust ([napi-rs](https://napi.rs) + [x11rb](https://github.com/psychon/x11rb)). It talks the X11 protocol directly, so it needs no `libX11`/`libXmu`. Prebuilt binaries for every target below ship inside the npm package, and the right one is picked at runtime. Installing compiles nothing and needs no system packages.

| OS | Architectures | Tested in CI |
|----|---------------|--------------|
| Linux, glibc 2.17+ (2.19+ on ppc64le, 2.27+ on riscv64) | x64, arm64, arm (armv7), ppc64le, s390x, riscv64 | full suite against Xvfb + openbox (QEMU for non-native arches) |
| Linux, musl (Alpine) | x64, arm64, arm (armv7) | full suite against Xvfb + openbox |
| FreeBSD | x64 | full suite against Xvfb + openbox, in a VM |
| macOS (XQuartz) | x64, arm64 | API suite only (no X server on the runners) |
| Windows (an X server such as VcXsrv, `DISPLAY=host:0`) | x64, ia32, arm64 | API suite only |

## Install

`npm install wmctrljs`

## Description
Every function exists in two flavours: `xxxSync` runs on the calling thread, `xxx` returns a promise and runs on the libuv thread pool.

Errors are `Error` objects whose message looks like `[functionName] NO_WINDOW_FOUND (24)`.
A bug that would have crashed the native code is reported as `INTERNAL_ERROR (43)` instead of taking the Node process down.

## Upgrading from 1.x

The JS API is unchanged. The behaviour changes below are fixes:

- No system packages are needed anymore (`libx11-dev`, `libxmu-dev`, node-gyp, a C++ compiler).
- Promises now reject with `Error` objects instead of strings. The message is the same.
- `getWindowsByPid*` / `getWindowsByClassName*` return `[]` when nothing matches instead of throwing `NO_WINDOW_FOUND`. Their windows now include `frame_extents`, and `getWindowsByPid(0)` no longer matches windows that have no pid.
- `closeWindowsByPidSync` / `closeWindowsByClassNameSync` return `true` on success. They used to throw even when they worked.
- Windows now carry `win_icon_name` (it was declared but never set). The `Aspect` type now says `numerator` / `denominator`, which is what was always returned.
- `windowMoveResizeTransition` really animates y (it used to step it with x) and starts from the window's absolute position.
- The 1.x C addon double-freed memory in `getActiveWindow()` and could crash the process. That is gone.
- `windowMoveResizeTransition` rejects an `await_time` no timer can hold (`Infinity`) and handles extreme coordinates without overflowing.

## Development

Requirements: Node.js 18+, a Rust toolchain, and for tests an X server with an EWMH window manager (for example `xvfb` + `openbox`).

```sh
yarn install
yarn build:native   # cargo build -> prebuilds/<platform>-<arch>/wmctrljs.node (+ the test fixture window)
yarn build          # TypeScript 7
xvfb-run -a -s '-screen 0 1280x1024x24 -screen 1 800x600x24' sh -c 'openbox & DISPLAY=$DISPLAY.1 openbox & WMCTRLJS_SCREEN1=$DISPLAY.1 yarn test'
cd native && cargo test --lib   # Rust unit tests, no X server needed
```

Cross builds use [cargo-zigbuild](https://github.com/rust-cross/cargo-zigbuild) (and zig): `node scripts/build-native.js --zig --target aarch64-unknown-linux-gnu.2.17 --dir linux-arm64`.

The native code has no `unsafe`, `unwrap`, `expect`, `panic!`, unchecked indexing or unchecked arithmetic outside tests: lints in `native/Cargo.toml` reject them, and CI runs clippy with `-D warnings`.

Three suites guard against regressions:
- Rust unit tests: property decoding, window JSON, size hints, gravity flags, transitions, error codes.
- `api.test`: exports, argument validation and error messages of every function, and the binary loader. Runs without X.
- `x11.test`: every function against a real window manager. It spawns its own fixture windows (`native/examples/fixture.rs`) and only ever acts on those. It is skipped when `DISPLAY` is unset, and its second-screen tests when `WMCTRLJS_SCREEN1` (a display string for screen 1 of the same server, with its own window manager) is unset.

CI builds and tests the 15 targets above, and publishes to npm when a `v*` tag is pushed.

## Window

wmctrljs provide you an access to alot of properties on your windows many more than wmctrl process.
See the prototype as follow.
```TS
interface Window {
    win_id:number;
    win_pid:number;
    win_client_machine:string;
    win_class:string;
    win_types:TypeDesc[];
    win_actions:TypeDesc[];
    win_states:TypeDesc[];
    win_name:string;
    win_visible_name:string;
    win_icon_name:string;
    win_visible_icon_name:string;
    win_geometry:Geometry|undefined;

    wm_normal_hints_supplied:number;
    wm_hints_supplied:number;
    WM_NORMAL_HINTS:XSizeHints|undefined;
    WM_HINTS:XSizeHints|undefined;
    desktop_number:number;
    showing_desktop:number;
    frame_extents:FrameExtents|undefined
}
```

### Types

Window types are describe by this interface.
```TS
interface TypeDesc {
    flag:string
    number:number
}
```

You have access to window's size and positions.
```TS
interface Geometry {
    x:number
    y:number
    abs_x:number
    abs_y:number
    width:number
    height:number
}
```
wmctrljs also provide you XSizeHints propertis based on Xutils.h structure.

```TS
interface Aspect {
    numerator:number
    denominator:number
}
```

```TS 
interface XSizeHints { //Based on Xutils.h
    flags:number	/* marks which fields in this structure are defined */
    x:number
    y:number		/* obsolete for new window mgrs, but clients */
    width:number
    height:number	/* should set so old wm's don't mess up */
    min_width:number
    min_height:number
    max_width:number
    max_height:number
    width_inc:number
    height_inc:number
    min_aspect:Aspect 
    max_aspect:Aspect
    base_width:number
    base_height:number
    win_gravity:number
}
```

It is possible to have access to window's frame extents provided by his window manager.

```TS
interface FrameExtents {
    left:number,
    right:number,
    top:number,
    bottom:number
}
```

Screen description is also available.
```TS
interface Screen {
    backing_store:number;
    black_pixel:number;
    cmap:number;
    height:number;
    max_maps:number;
    mheight:number;
    min_maps:number;
    mwidth:number;
    ndepths:number;
    root_depth:number;
    root_input_mask:number;
    save_unders:number;
    white_pixel:number;
    width:number
}
```

## Available functions

Get screen's information
```TS
function getScreenSync():Screen
function getScreen():Promise<Screen>
```

Get windows information
```TS
function getWindowListSync():Window[]
function getWindowList():Promise<Window[]>
```

Get Window by id
```TS
function getWindowByIdSync():Window
function getWindowById():Promise<Window>
```

Get the current active window, it is possible no window is active for now then return value can be `null`
```TS
function getActiveWindowSync():Window
function getActiveWindow():Promise<Window>
```

Get a list of window with the pid provided, an empty list if no window matches
```TS
function getWindowsByPidSync(win_pid:number):Window[]
function getWindowsByPid(win_pid:number):Promise<Window[]>
```

Get a list of window with the class provided (`instance.Class`, e.g. `xeyes.XEyes`), an empty list if no window matches
```TS
function getWindowsByClassNameSync(win_class_name:string):Window[]
function getWindowsByClassName(win_class_name:string):Promise<Window[]>
```

Active a window by his id, if no window match with this id a window not found exception will be throw.
```TS
function activeWindowByIdSync(win_id:number):boolean
function activeWindowById(win_id:number):Promise<boolean>
```

Active multiple windows by classname or by pid with respective functions
```TS
function activeWindowsByClassNameSync(win_class_name:string):boolean
function activeWindowsByClassName(win_class_name:string):Promise<boolean>

function activeWindowsByPidSync(win_pid:number):boolean
function activeWindowsByPid(win_pid:number):Promise<boolean>
```

It's the same issue for closing window(s)
```TS
function closeWindowByIdSync(win_id:number):boolean
function closeWindowById(win_id:number):Promise<boolean>

function closeWindowsByClassNameSync(win_class_name:string):boolean
function closeWindowsByClassName(win_class_name:string):Promise<boolean>

function closeWindowsByPidSync(win_pid:number):boolean
function closeWindowsByPid(win_pid:number):Promise<boolean>
```

Move and resize your window
```TS
function windowMoveResizeSync(win_id:number, win_gravity:number, win_x:number, 
                                    win_y:number, win_width:number, win_height:number):boolean
function windowMoveResize(win_id:number, win_gravity:number, win_x:number, 
                                    win_y:number, win_width:number, win_height:number):Promise<boolean>
```

You can also move and resize windows with transitions, this is a pure `wmctrljs` function
```TS
function windowMoveResizeTransition(win_id:number, win_gravity:number, win_x:number, 
    win_y:number, win_width:number, win_height:number, await_time:number=1, pad:number=1):Promise<boolean>
```
Here the `await_time` is the duration between all window's positions and dimensions, and `pad` allow you to control the number of step it will have.

Because of the duration of this animation it is only available as a promise.

Make some action on window's states

Actions available are `add` `remove` and `toggle`

Properties: `modal`  `sticky` `maximized_vert` `maximized_horz` `shaded` `skip_taskbar` `skip_pager` `hidden` `fullscreen` `above` `below`

```TS
function windowStateSync(win_id:number, action:string, prop1:string, prop2?:string):boolean
function windowState(win_id:number, action:string, prop1:string, prop2?:string):Promise<boolean>
```

Minimize a window
```TS
function windowMinimizeSync(win_id:number):boolean
function windowMinimize(win_id:number):Promise<boolean>
```

Take off all restriction on the window size, thus you can resize it as you want.
```TS
function windowAllowAllSizesSync(win_id:number):boolean
function windowAllowAllSizes(win_id:number):Promise<boolean>
```

Put a window in first plan
```TS
function windowRaiseSync(win_id:number):boolean
function windowRaise(win_id:number):Promise<boolean>
```