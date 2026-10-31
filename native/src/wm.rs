//! EWMH window management spoken directly over the X11 protocol (x11rb):
//! no libX11/libXmu at build or run time.
#![forbid(unsafe_code)]

use std::{collections::HashMap, fmt, thread, time::Duration};

use serde_json::{Value, json};
use x11rb::atom_manager;
use x11rb::connection::Connection;
use x11rb::cookie::Cookie;
use x11rb::errors::{ConnectionError, ReplyError};
use x11rb::protocol::xproto::*;
use x11rb::rust_connection::RustConnection;
use x11rb::wrapper::ConnectionExt as _;

const TYPES: &[&str] = &[
    "_NET_WM_WINDOW_TYPE_DESKTOP",
    "_NET_WM_WINDOW_TYPE_DOCK",
    "_NET_WM_WINDOW_TYPE_TOOLBAR",
    "_NET_WM_WINDOW_TYPE_MENU",
    "_NET_WM_WINDOW_TYPE_UTILITY",
    "_NET_WM_WINDOW_TYPE_SPLASH",
    "_NET_WM_WINDOW_TYPE_DIALOG",
    "_NET_WM_WINDOW_TYPE_NORMAL",
];

const ACTIONS: &[&str] = &[
    "_NET_WM_ACTION_MOVE",
    "_NET_WM_ACTION_RESIZE",
    "_NET_WM_ACTION_MINIMIZE",
    "_NET_WM_ACTION_SHADE",
    "_NET_WM_ACTION_STICK",
    "_NET_WM_ACTION_MAXIMIZE_HORZ",
    "_NET_WM_ACTION_MAXIMIZE_VERT",
    "_NET_WM_ACTION_FULLSCREEN",
    "_NET_WM_ACTION_CHANGE_DESKTOP",
    "_NET_WM_ACTION_CLOSE",
];

const STATES: &[&str] = &[
    "_NET_WM_STATE_MODAL",
    "_NET_WM_STATE_STICKY",
    "_NET_WM_STATE_MAXIMIZED_VERT",
    "_NET_WM_STATE_MAXIMIZED_HORZ",
    "_NET_WM_STATE_SHADED",
    "_NET_WM_STATE_SKIP_TASKBAR",
    "_NET_WM_STATE_SKIP_PAGER",
    "_NET_WM_STATE_HIDDEN",
    "_NET_WM_STATE_FULLSCREEN",
    "_NET_WM_STATE_ABOVE",
    "_NET_WM_STATE_BELOW",
    "_NET_WM_STATE_DEMANDS_ATTENTION",
];

atom_manager! {
    Atoms: AtomsCookie {
        UTF8_STRING,
        WM_VISIBLE_NAME,
        WM_CHANGE_STATE,
        _NET_SUPPORTED,
        _NET_CLIENT_LIST,
        _WIN_CLIENT_LIST,
        _NET_ACTIVE_WINDOW,
        _NET_CLOSE_WINDOW,
        _NET_MOVERESIZE_WINDOW,
        _NET_WM_PID,
        _NET_WM_DESKTOP,
        _WIN_WORKSPACE,
        _NET_SHOWING_DESKTOP,
        _NET_WM_WINDOW_TYPE,
        _NET_WM_ALLOWED_ACTIONS,
        _NET_WM_STATE,
        _NET_WM_NAME,
        _NET_WM_VISIBLE_NAME,
        _NET_WM_ICON_NAME,
        _NET_WM_VISIBLE_ICON_NAME,
        _NET_FRAME_EXTENTS,
    }
}

/// Error codes kept identical to the former C `enum STATES`.
#[derive(Debug, Clone, Copy, PartialEq)]
pub enum Fail {
    CanNotGetClientList = 1,
    CanNotOpenDisplay = 3,
    CanNotActivateWindow = 10,
    CanNotSetWindowState = 22,
    NoWindowFound = 24,
    CanNotMoveResizeWindow = 27,
    NoWindowActiveForNow = 28,
    CanNotGetWindowAttributes = 36,
    /// A panic, caught at the N-API boundary.
    Internal = 43,
}

impl fmt::Display for Fail {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        let name = match self {
            Fail::CanNotGetClientList => "CAN_NOT_GET_CLIENT_LIST",
            Fail::CanNotOpenDisplay => "CAN_NOT_OPEN_DISPLAY",
            Fail::CanNotActivateWindow => "CAN_NOT_ACTIVATE_WINDOW",
            Fail::CanNotSetWindowState => "CAN_NOT_SET_WINDOW_STATE",
            Fail::NoWindowFound => "NO_WINDOW_FOUND",
            Fail::CanNotMoveResizeWindow => "CAN_NOT_MOVE_RESIZE_WINDOW",
            Fail::NoWindowActiveForNow => "NO_WINDOW_ACTIVE_FOR_NOW",
            Fail::CanNotGetWindowAttributes => "CAN_NOT_GET_WINDOW_ATTRIBUTES",
            Fail::Internal => "INTERNAL_ERROR",
        };
        write!(f, "{name} ({})", *self as i32)
    }
}

// A broken connection mid-call is reported like a display that cannot be opened.
impl From<ConnectionError> for Fail {
    fn from(_: ConnectionError) -> Self {
        Fail::CanNotOpenDisplay
    }
}

impl From<ReplyError> for Fail {
    fn from(_: ReplyError) -> Self {
        Fail::CanNotOpenDisplay
    }
}

pub type Res = Result<Value, Fail>;

pub enum By {
    Pid(u32),
    Class(String),
}

type PropCookie<'c> = Cookie<'c, RustConnection, GetPropertyReply>;

type Flags = HashMap<&'static str, Atom>;

struct X {
    c: RustConnection,
    root: Window,
    a: Atoms,
    flags: Flags,
}

/// Window info requests already sent to the server, waiting for their replies.
struct Pending<'c> {
    win: Window,
    props: [(Option<PropCookie<'c>>, Atom); 16],
    geo: Option<Cookie<'c, RustConnection, GetGeometryReply>>,
    abs: Option<Cookie<'c, RustConnection, TranslateCoordinatesReply>>,
}

fn connect() -> Result<(RustConnection, usize), Fail> {
    RustConnection::connect(None).map_err(|_| Fail::CanNotOpenDisplay)
}

/// The property exists with the expected type (ANY accepts all).
fn accept(r: &GetPropertyReply, ty: Atom) -> bool {
    r.type_ != x11rb::NONE && (ty == u32::from(AtomEnum::ANY) || r.type_ == ty)
}

fn reply(ck: Option<PropCookie<'_>>, ty: Atom) -> Option<GetPropertyReply> {
    ck?.reply().ok().filter(|r| accept(r, ty))
}

fn longs(r: Option<GetPropertyReply>) -> Vec<u32> {
    r.and_then(|r| Some(r.value32()?.collect())).unwrap_or_default()
}

fn card(r: Option<GetPropertyReply>) -> u32 {
    longs(r).first().copied().unwrap_or(0)
}

fn cstr(b: &[u8]) -> String {
    String::from_utf8_lossy(b.split(|&c| c == 0).next().unwrap_or(&[])).into_owned()
}

fn text(r: Option<GetPropertyReply>) -> Option<String> {
    r.map(|r| cstr(&r.value))
}

/// WM_CLASS "instance\0Class\0" -> "instance.Class".
fn class(r: Option<GetPropertyReply>) -> Option<String> {
    let mut v = r?.value;
    // The first NUL, unless it is the last byte.
    if let Some(nul) = v.split_last_mut().and_then(|(_, init)| init.iter_mut().find(|c| **c == 0)) {
        *nul = b'.';
    }
    Some(cstr(&v))
}

/// Mirrors XGetWMNormalHints: needs 15 longs, base size and gravity need all 18.
fn size_hints(r: Option<GetPropertyReply>) -> (u32, Value) {
    let v: Vec<i32> = longs(r).into_iter().map(|n| n as i32).collect();
    let supplied = match v.len() {
        0..15 => 0,
        15..18 => 255,
        _ => 1023,
    };
    let f = |i: usize| if supplied == 0 || (i >= 15 && supplied != 1023) { 0 } else { v.get(i).copied().unwrap_or(0) };
    let hints = json!({
        "flags": f(0) as u32 & supplied,
        "x": f(1),
        "y": f(2),
        "width": f(3),
        "height": f(4),
        "min_width": f(5),
        "min_height": f(6),
        "max_width": f(7),
        "max_height": f(8),
        "width_inc": f(9),
        "height_inc": f(10),
        "min_aspect": { "numerator": f(11), "denominator": f(12) },
        "max_aspect": { "numerator": f(13), "denominator": f(14) },
        "base_width": f(15),
        "base_height": f(16),
        "win_gravity": f(17),
    });
    (supplied, hints)
}

/// _NET_MOVERESIZE_WINDOW flags: gravity, plus one bit per value that is not -1 (untouched).
fn moveresize_flags(grav: i32, xywh: [i32; 4]) -> u32 {
    xywh.iter().zip([0x100, 0x200, 0x400, 0x800]).filter(|(v, _)| **v != -1).fold(grav as u32, |f, (_, bit)| f | bit)
}

/// Plain ConfigureWindow used when the WM lacks _NET_MOVERESIZE_WINDOW.
fn fallback_geometry([x, y, w, h]: [i32; 4]) -> ConfigureWindowAux {
    let mut aux = ConfigureWindowAux::new();
    if x >= 0 && y >= 0 {
        aux = aux.x(x).y(y);
    }
    if w >= 1 && h >= 1 {
        aux = aux.width(w as u32).height(h as u32);
    }
    aux
}

/// One step of `pad` from `cur` towards `to`, saturating instead of overflowing.
fn step(cur: i32, to: i32, pad: i32) -> i32 {
    if cur < to { cur.saturating_add(pad).min(to) } else { cur.saturating_sub(pad).max(to) }
}

/// Every (x, y, width, height) a transition requests, `pad` at a time; the last one is `to`, and
/// there is at least one. -1 in `to` leaves a value untouched. Lazy: pad 1 to a far target is
/// billions of steps.
fn transition(from: [i32; 4], to: [i32; 4], pad: i32) -> impl Iterator<Item = [i32; 4]> {
    let pad = pad.max(1);
    let ([fx, fy, fw, fh], [tx, ty, tw, th]) = (from, to);
    let mut axes = [(fx, tx), (fy, ty), (fw, tw), (fh, th)];
    let mut first = true;
    std::iter::from_fn(move || {
        let next = axes.map(|(c, t)| (if t == -1 { c } else { step(c, t, pad) }, t));
        if next == axes && !std::mem::take(&mut first) {
            return None;
        }
        first = false;
        axes = next;
        Some(next.map(|(c, t)| if t == -1 { -1 } else { c }))
    })
}

/// _NET_WM_STATE action: REMOVE, ADD or TOGGLE.
fn state_action(action: &str) -> Option<u32> {
    ["REMOVE", "ADD", "TOGGLE"].iter().position(|a| *a == action).map(|i| i as u32)
}

fn state_atom_name(prop: &str) -> String {
    format!("_NET_WM_STATE_{}", prop.to_uppercase())
}

/// Known EWMH flags among `atoms`, in property order.
fn descs(flags: &Flags, r: Option<GetPropertyReply>, names: &[&'static str]) -> Value {
    longs(r)
        .into_iter()
        .flat_map(|atom| {
            names
                .iter()
                .filter(move |n| flags[*n] == atom)
                .map(move |n| json!({ "flag": n, "number": atom }))
        })
        .collect()
}

/// The JS window object, from the replies to the 16 properties `X::request` asks for.
fn window_json(
    win: Window,
    props: [Option<GetPropertyReply>; 16],
    geo: Option<(GetGeometryReply, TranslateCoordinatesReply)>,
    flags: &Flags,
) -> Value {
    let [
        pid,
        desktop,
        showing,
        machine,
        wm_class,
        types,
        actions,
        states,
        net_name,
        name,
        net_visible,
        visible,
        icon,
        visible_icon,
        extents,
        normal_hints,
    ] = props;
    let (supplied, normal_hints) = size_hints(normal_hints);
    let mut w = json!({
        "win_id": win,
        "win_pid": card(pid),
        "desktop_number": card(desktop),
        "showing_desktop": card(showing),
        "win_client_machine": text(machine).unwrap_or_default(),
        "win_class": class(wm_class).unwrap_or_default(),
        "win_types": descs(flags, types, TYPES),
        "win_actions": descs(flags, actions, ACTIONS),
        "win_states": descs(flags, states, STATES),
        "win_name": text(net_name).or_else(|| text(name)).unwrap_or_default(),
        "win_visible_name": text(net_visible).or_else(|| text(visible)).unwrap_or_default(),
        "win_icon_name": text(icon).unwrap_or_default(),
        "win_visible_icon_name": text(visible_icon).unwrap_or_default(),
        "wm_normal_hints_supplied": supplied,
        "wm_hints_supplied": 0,
    });
    if let Some(o) = w.as_object_mut() {
        if let Some((g, t)) = geo {
            let geometry = json!({
                "width": g.width,
                "height": g.height,
                "x": g.x,
                "y": g.y,
                "abs_x": t.dst_x,
                "abs_y": t.dst_y,
            });
            o.insert("win_geometry".into(), geometry);
        }
        o.insert("WM_NORMAL_HINTS".into(), normal_hints);
        // ponytail: WM_HINTS was always an empty XSizeHints in the C addon; kept for shape compatibility.
        o.insert("WM_HINTS".into(), size_hints(None).1);
        if let [left, right, top, bottom] = longs(extents)[..] {
            o.insert("frame_extents".into(), json!({ "left": left, "right": right, "top": top, "bottom": bottom }));
        }
    }
    w
}

fn matches(w: &Value, by: &By) -> bool {
    match by {
        By::Pid(pid) => *pid != 0 && w["win_pid"] == *pid,
        By::Class(class) => w["win_class"] == class.as_str(),
    }
}

fn screen_json(s: &Screen) -> Value {
    json!({
        "backing_store": u32::from(s.backing_stores),
        "black_pixel": s.black_pixel,
        "cmap": s.default_colormap,
        "height": s.height_in_pixels,
        "max_maps": s.max_installed_maps,
        "mheight": s.height_in_millimeters,
        "min_maps": s.min_installed_maps,
        "mwidth": s.width_in_millimeters,
        "ndepths": s.allowed_depths.len(),
        "root_depth": s.root_depth,
        "root_input_mask": u32::from(s.current_input_masks),
        "save_unders": u8::from(s.save_unders),
        "white_pixel": s.white_pixel,
        "width": s.width_in_pixels,
    })
}

impl X {
    fn open() -> Result<Self, Fail> {
        let (c, screen) = connect()?;
        let root = c.setup().roots.get(screen).ok_or(Fail::CanNotOpenDisplay)?.root;
        let names = TYPES.iter().chain(ACTIONS).chain(STATES);
        let a = Atoms::new(&c)?;
        let cookies = names
            .clone()
            .map(|n| c.intern_atom(false, n.as_bytes()))
            .collect::<Result<Vec<_>, _>>()?;
        let a = a.reply()?;
        let flags = names
            .zip(cookies)
            .map(|(n, ck)| Ok((*n, ck.reply()?.atom)))
            .collect::<Result<_, ReplyError>>()?;
        Ok(X { c, root, a, flags })
    }

    /// Waits until the server has processed every request, like XCloseDisplay did.
    fn done(self) -> Res {
        self.c.sync()?;
        Ok(Value::Bool(true))
    }

    fn get(&self, win: Window, prop: impl Into<Atom>, ty: impl Into<Atom>) -> Option<PropCookie<'_>> {
        self.c.get_property(false, win, prop, ty, 0, u32::MAX).ok()
    }

    fn prop(&self, win: Window, prop: Atom, ty: impl Into<Atom>) -> Option<GetPropertyReply> {
        let ty = ty.into();
        reply(self.get(win, prop, ty), ty)
    }

    fn msg(&self, root: Window, win: Window, ty: Atom, data: [u32; 5]) -> Result<(), Fail> {
        let ev = ClientMessageEvent::new(32, win, ty, data);
        let mask = EventMask::SUBSTRUCTURE_REDIRECT | EventMask::SUBSTRUCTURE_NOTIFY;
        self.c.send_event(false, root, mask, ev)?;
        Ok(())
    }

    fn clients(&self) -> Result<Vec<Window>, Fail> {
        self.prop(self.root, self.a._NET_CLIENT_LIST, AtomEnum::WINDOW)
            .or_else(|| self.prop(self.root, self.a._WIN_CLIENT_LIST, AtomEnum::CARDINAL))
            .map(|r| longs(Some(r)))
            .ok_or(Fail::CanNotGetClientList)
    }

    fn request(&self, win: Window) -> Pending<'_> {
        let a = &self.a;
        let (card, string, atom) = (AtomEnum::CARDINAL.into(), AtomEnum::STRING.into(), AtomEnum::ATOM.into());
        let wanted: [(Atom, Atom); 16] = [
            (a._NET_WM_PID, card),
            (a._NET_WM_DESKTOP, card),
            (a._NET_SHOWING_DESKTOP, card),
            (AtomEnum::WM_CLIENT_MACHINE.into(), string),
            (AtomEnum::WM_CLASS.into(), string),
            (a._NET_WM_WINDOW_TYPE, atom),
            (a._NET_WM_ALLOWED_ACTIONS, atom),
            (a._NET_WM_STATE, atom),
            (a._NET_WM_NAME, a.UTF8_STRING),
            (AtomEnum::WM_NAME.into(), string),
            (a._NET_WM_VISIBLE_NAME, a.UTF8_STRING),
            (a.WM_VISIBLE_NAME, string),
            (a._NET_WM_ICON_NAME, a.UTF8_STRING),
            (a._NET_WM_VISIBLE_ICON_NAME, a.UTF8_STRING),
            (a._NET_FRAME_EXTENTS, AtomEnum::ANY.into()),
            (AtomEnum::WM_NORMAL_HINTS.into(), AtomEnum::WM_SIZE_HINTS.into()),
        ];
        Pending {
            win,
            props: wanted.map(|(p, ty)| (self.get(win, p, ty), ty)),
            geo: self.c.get_geometry(win).ok(),
            abs: self.c.translate_coordinates(win, self.root, 0, 0).ok(),
        }
    }

    fn window(&self, p: Pending<'_>) -> Value {
        let geo = p.geo.and_then(|c| c.reply().ok());
        let abs = match (&geo, p.abs.and_then(|c| c.reply().ok())) {
            // On another screen than the default one: translate to the window's own root (one more round trip).
            (Some(g), Some(a)) if !a.same_screen => self.c.translate_coordinates(p.win, g.root, 0, 0).ok().and_then(|c| c.reply().ok()),
            (_, abs) => abs,
        };
        window_json(p.win, p.props.map(|(ck, ty)| reply(ck, ty)), geo.zip(abs), &self.flags)
    }

    /// All client windows, fetched in a single pipelined round trip.
    fn list(&self) -> Result<Vec<Value>, Fail> {
        let pending: Vec<_> = self.clients()?.into_iter().map(|w| self.request(w)).collect();
        Ok(pending.into_iter().map(|p| self.window(p)).collect())
    }

    fn find(&self, by: &By) -> Result<Vec<Value>, Fail> {
        let mut wins = self.list()?;
        wins.retain(|w| matches(w, by));
        Ok(wins)
    }

    fn find_ids(&self, by: &By) -> Result<Vec<Window>, Fail> {
        let ids: Vec<Window> = self.find(by)?.iter().filter_map(|w| w["win_id"].as_u64()).map(|id| id as Window).collect();
        if ids.is_empty() { Err(Fail::NoWindowFound) } else { Ok(ids) }
    }

    fn activate(&self, win: Window) -> Result<(), Fail> {
        // Like the C addon: a window without a desktop is not an activable client.
        if self.prop(win, self.a._NET_WM_DESKTOP, AtomEnum::CARDINAL).is_none()
            && self.prop(win, self.a._WIN_WORKSPACE, AtomEnum::CARDINAL).is_none()
        {
            return Err(Fail::CanNotActivateWindow);
        }
        self.c.configure_window(win, &ConfigureWindowAux::new().stack_mode(StackMode::ABOVE))?;
        self.c.map_window(win)?;
        self.msg(self.root, win, self.a._NET_ACTIVE_WINDOW, [0; 5])
    }

    fn supports(&self, atom: Atom) -> bool {
        longs(self.prop(self.root, self.a._NET_SUPPORTED, AtomEnum::ATOM)).contains(&atom)
    }

    fn move_resize(&self, win: Window, grav: i32, x: i32, y: i32, w: i32, h: i32) -> Result<(), Fail> {
        if grav < 0 {
            return Err(Fail::CanNotMoveResizeWindow);
        }
        if self.supports(self.a._NET_MOVERESIZE_WINDOW) {
            let flags = moveresize_flags(grav, [x, y, w, h]);
            return self.msg(self.root, win, self.a._NET_MOVERESIZE_WINDOW, [flags, x as u32, y as u32, w as u32, h as u32]);
        }
        self.c.configure_window(win, &fallback_geometry([x, y, w, h]))?;
        Ok(())
    }
}

pub fn screen() -> Res {
    let (c, n) = connect()?;
    Ok(screen_json(c.setup().roots.get(n).ok_or(Fail::CanNotOpenDisplay)?))
}

pub fn window(win: Window) -> Res {
    let x = X::open()?;
    Ok(x.window(x.request(win)))
}

pub fn active_window() -> Res {
    let x = X::open()?;
    // C quirk kept: a WM without _NET_ACTIVE_WINDOW reports CAN_NOT_ACTIVATE_WINDOW.
    let active = x
        .prop(x.root, x.a._NET_ACTIVE_WINDOW, AtomEnum::WINDOW)
        .ok_or(Fail::CanNotActivateWindow)?;
    match card(Some(active)) {
        0 => Err(Fail::NoWindowActiveForNow),
        win => Ok(x.window(x.request(win))),
    }
}

pub fn list() -> Res {
    Ok(X::open()?.list()?.into())
}

pub fn windows_by(by: &By) -> Res {
    Ok(X::open()?.find(by)?.into())
}

pub fn activate(win: Window) -> Res {
    let x = X::open()?;
    x.activate(win)?;
    x.done()
}

pub fn activate_by(by: &By) -> Res {
    let x = X::open()?;
    for win in x.find_ids(by)? {
        x.activate(win)?;
    }
    x.done()
}

pub fn close(win: Window) -> Res {
    let x = X::open()?;
    x.msg(x.root, win, x.a._NET_CLOSE_WINDOW, [0; 5])?;
    x.done()
}

pub fn close_by(by: &By) -> Res {
    let x = X::open()?;
    for win in x.find_ids(by)? {
        x.msg(x.root, win, x.a._NET_CLOSE_WINDOW, [0; 5])?;
    }
    x.done()
}

pub fn move_resize(win: Window, grav: i32, x: i32, y: i32, w: i32, h: i32) -> Res {
    let c = X::open()?;
    c.move_resize(win, grav, x, y, w, h)?;
    c.done()
}

/// Steps each of x/y/width/height `pad` pixels at a time towards the target,
/// waiting `await_ms` between steps. -1 leaves a value untouched.
#[allow(clippy::too_many_arguments)]
pub fn move_resize_transition(win: Window, grav: i32, x: i32, y: i32, w: i32, h: i32, await_ms: f64, pad: i32) -> Res {
    let pause = Duration::try_from_secs_f64(await_ms.max(0.0) / 1000.0).map_err(|_| Fail::CanNotMoveResizeWindow)?;
    let c = X::open()?;
    if grav < 0 {
        return Err(Fail::CanNotMoveResizeWindow);
    }
    let geo = c.c.get_geometry(win)?.reply()?;
    let abs = c.c.translate_coordinates(win, geo.root, 0, 0)?.reply()?;
    let from = [abs.dst_x.into(), abs.dst_y.into(), geo.width.into(), geo.height.into()];
    for [x, y, w, h] in transition(from, [x, y, w, h], pad) {
        c.move_resize(win, grav, x, y, w, h)?;
        c.c.sync()?;
        thread::sleep(pause);
    }
    c.done()
}

pub fn state(win: Window, action: &str, prop1: &str, prop2: &str) -> Res {
    let action = state_action(action).ok_or(Fail::CanNotSetWindowState)?;
    let x = X::open()?;
    let atom = |p: &str| -> Result<Atom, Fail> {
        if p.is_empty() {
            return Ok(0);
        }
        Ok(x.c.intern_atom(false, state_atom_name(p).as_bytes())?.reply()?.atom)
    };
    x.msg(x.root, win, x.a._NET_WM_STATE, [action, atom(prop1)?, atom(prop2)?, 0, 0])?;
    x.done()
}

/// XIconifyWindow: ask the WM of the window's own screen to switch it to IconicState (3).
pub fn minimize(win: Window) -> Res {
    let x = X::open()?;
    let (attrs, geo) = (x.c.get_window_attributes(win)?, x.c.get_geometry(win)?);
    attrs.reply().map_err(|_| Fail::CanNotGetWindowAttributes)?;
    let root = geo.reply().map_err(|_| Fail::CanNotGetWindowAttributes)?.root;
    x.msg(root, win, x.a.WM_CHANGE_STATE, [3, 0, 0, 0, 0])?;
    x.done()
}

/// Replaces WM_NORMAL_HINTS with empty hints so the WM lifts any size constraint.
pub fn allow_all_sizes(win: Window) -> Res {
    let x = X::open()?;
    x.c.change_property32(PropMode::REPLACE, win, AtomEnum::WM_NORMAL_HINTS, AtomEnum::WM_SIZE_HINTS, &[0; 18])?;
    x.done()
}

pub fn raise(win: Window) -> Res {
    let x = X::open()?;
    x.c.configure_window(win, &ConfigureWindowAux::new().stack_mode(StackMode::ABOVE))?;
    x.done()
}


#[cfg(test)]
mod tests {
    use super::*;

    const UTF8: Atom = 500;

    fn reply32(ty: impl Into<Atom>, v: &[u32]) -> Option<GetPropertyReply> {
        let value: Vec<u8> = v.iter().flat_map(|n| n.to_ne_bytes()).collect();
        Some(GetPropertyReply { format: 32, sequence: 0, length: 0, type_: ty.into(), bytes_after: 0, value_len: v.len() as u32, value })
    }

    fn reply8(ty: impl Into<Atom>, v: &[u8]) -> Option<GetPropertyReply> {
        Some(GetPropertyReply { format: 8, sequence: 0, length: 0, type_: ty.into(), bytes_after: 0, value_len: v.len() as u32, value: v.to_vec() })
    }

    /// Every known flag gets atom 1000 + its index in TYPES ++ ACTIONS ++ STATES.
    fn flags() -> Flags {
        TYPES.iter().chain(ACTIONS).chain(STATES).zip(1000..).map(|(n, a)| (*n, a)).collect()
    }

    fn atom_of(name: &str) -> u32 {
        flags()[name]
    }

    #[test]
    fn transitions_refuse_pauses_a_duration_cannot_hold() {
        // Checked before connecting: no display needed.
        for ms in [f64::INFINITY, 1e300] {
            assert_eq!(move_resize_transition(1, 0, 0, 0, 1, 1, ms, 1), Err(Fail::CanNotMoveResizeWindow), "{ms}");
        }
    }

    #[test]
    fn fail_messages_keep_the_c_codes() {
        let all = [
            (Fail::CanNotGetClientList, "CAN_NOT_GET_CLIENT_LIST (1)"),
            (Fail::CanNotOpenDisplay, "CAN_NOT_OPEN_DISPLAY (3)"),
            (Fail::CanNotActivateWindow, "CAN_NOT_ACTIVATE_WINDOW (10)"),
            (Fail::CanNotSetWindowState, "CAN_NOT_SET_WINDOW_STATE (22)"),
            (Fail::NoWindowFound, "NO_WINDOW_FOUND (24)"),
            (Fail::CanNotMoveResizeWindow, "CAN_NOT_MOVE_RESIZE_WINDOW (27)"),
            (Fail::NoWindowActiveForNow, "NO_WINDOW_ACTIVE_FOR_NOW (28)"),
            (Fail::CanNotGetWindowAttributes, "CAN_NOT_GET_WINDOW_ATTRIBUTES (36)"),
            (Fail::Internal, "INTERNAL_ERROR (43)"),
        ];
        for (fail, msg) in all {
            assert_eq!(fail.to_string(), msg);
        }
    }

    #[test]
    fn connection_errors_map_to_can_not_open_display() {
        assert_eq!(Fail::from(ConnectionError::UnknownError), Fail::CanNotOpenDisplay);
        assert_eq!(Fail::from(ReplyError::from(ConnectionError::UnknownError)), Fail::CanNotOpenDisplay);
    }

    #[test]
    fn accept_checks_existence_and_type() {
        let card = u32::from(AtomEnum::CARDINAL);
        assert!(accept(&reply32(card, &[1]).unwrap(), card));
        assert!(!accept(&reply32(AtomEnum::ATOM, &[1]).unwrap(), card));
        assert!(!accept(&reply32(x11rb::NONE, &[]).unwrap(), card));
        assert!(accept(&reply32(card, &[1]).unwrap(), AtomEnum::ANY.into()));
        assert!(!accept(&reply32(x11rb::NONE, &[]).unwrap(), AtomEnum::ANY.into()));
    }

    #[test]
    fn longs_and_card() {
        assert_eq!(longs(reply32(AtomEnum::CARDINAL, &[1, 2, u32::MAX])), [1, 2, u32::MAX]);
        assert_eq!(longs(reply8(AtomEnum::STRING, b"abcd")), Vec::<u32>::new());
        assert_eq!(longs(None), Vec::<u32>::new());
        assert_eq!(card(reply32(AtomEnum::CARDINAL, &[42, 7])), 42);
        assert_eq!(card(reply32(AtomEnum::CARDINAL, &[])), 0);
        assert_eq!(card(None), 0);
    }

    #[test]
    fn strings_stop_at_nul_and_survive_bad_utf8() {
        assert_eq!(cstr(b"hello\0world"), "hello");
        assert_eq!(cstr(b""), "");
        assert_eq!(cstr("caf\u{e9} \u{2713}".as_bytes()), "caf\u{e9} \u{2713}");
        assert_eq!(cstr(b"bad\xff"), "bad\u{fffd}");
        assert_eq!(text(reply8(UTF8, b"title\0")), Some("title".into()));
        assert_eq!(text(None), None);
    }

    #[test]
    fn wm_class_joins_instance_and_class() {
        let c = |v: &[u8]| class(reply8(AtomEnum::STRING, v));
        assert_eq!(c(b"xeyes\0XEyes\0"), Some("xeyes.XEyes".into()));
        assert_eq!(c(b"xeyes\0XEyes"), Some("xeyes.XEyes".into()));
        assert_eq!(c(b"xeyes\0"), Some("xeyes".into()));
        assert_eq!(c(b"xeyes"), Some("xeyes".into()));
        assert_eq!(c(b"xeyes\0\0"), Some("xeyes.".into()));
        assert_eq!(c(b""), Some("".into()));
        assert_eq!(class(None), None);
    }

    #[test]
    fn size_hints_absent_or_short_are_all_zero() {
        for r in [None, reply32(AtomEnum::WM_SIZE_HINTS, &[1; 14])] {
            let (supplied, hints) = size_hints(r);
            assert_eq!(supplied, 0);
            assert_eq!(hints, size_hints(None).1);
            assert_eq!(hints["min_aspect"], json!({ "numerator": 0, "denominator": 0 }));
        }
    }

    #[test]
    fn size_hints_full_18_longs() {
        let v: Vec<u32> = (0..18).map(|i| i * 10).collect();
        let mut v = v;
        v[0] = 0xffff; // every flag bit, only the defined ones survive
        v[1] = -5i32 as u32;
        let (supplied, h) = size_hints(reply32(AtomEnum::WM_SIZE_HINTS, &v));
        assert_eq!(supplied, 1023);
        assert_eq!(
            h,
            json!({
                "flags": 1023, "x": -5, "y": 20, "width": 30, "height": 40,
                "min_width": 50, "min_height": 60, "max_width": 70, "max_height": 80,
                "width_inc": 90, "height_inc": 100,
                "min_aspect": { "numerator": 110, "denominator": 120 },
                "max_aspect": { "numerator": 130, "denominator": 140 },
                "base_width": 150, "base_height": 160, "win_gravity": 170,
            })
        );
        let keys: Vec<_> = h.as_object().unwrap().keys().cloned().collect();
        assert_eq!(keys[0], "flags");
        assert_eq!(keys.last().unwrap(), "win_gravity");
    }

    #[test]
    fn size_hints_old_15_to_17_longs_skip_base_and_gravity() {
        for n in 15..18 {
            let v: Vec<u32> = (1..=n).collect();
            let (supplied, h) = size_hints(reply32(AtomEnum::WM_SIZE_HINTS, &v));
            assert_eq!(supplied, 255, "{n} longs");
            assert_eq!(h["flags"], 1);
            assert_eq!(h["max_aspect"]["denominator"], 15);
            assert_eq!((h["base_width"].clone(), h["base_height"].clone(), h["win_gravity"].clone()), (json!(0), json!(0), json!(0)));
        }
        // PBaseSize / PWinGravity flags are dropped when not supplied.
        let mut v = vec![0; 15];
        v[0] = 256 | 512 | 16;
        assert_eq!(size_hints(reply32(AtomEnum::WM_SIZE_HINTS, &v)).1["flags"], 16);
    }

    #[test]
    fn size_hints_ignore_8_bit_data() {
        assert_eq!(size_hints(reply8(AtomEnum::WM_SIZE_HINTS, &[1; 72])).0, 0);
    }

    #[test]
    fn descs_keep_property_order_and_skip_unknown_atoms() {
        let f = flags();
        let r = reply32(AtomEnum::ATOM, &[atom_of("_NET_WM_STATE_ABOVE"), 4242, atom_of("_NET_WM_STATE_MODAL")]);
        assert_eq!(
            descs(&f, r, STATES),
            json!([
                { "flag": "_NET_WM_STATE_ABOVE", "number": atom_of("_NET_WM_STATE_ABOVE") },
                { "flag": "_NET_WM_STATE_MODAL", "number": atom_of("_NET_WM_STATE_MODAL") },
            ])
        );
        // A type atom in the state list is not a state.
        assert_eq!(descs(&f, reply32(AtomEnum::ATOM, &[atom_of("_NET_WM_WINDOW_TYPE_NORMAL")]), STATES), json!([]));
        assert_eq!(descs(&f, None, TYPES), json!([]));
    }

    #[test]
    fn known_flag_lists_match_the_c_addon() {
        assert_eq!((TYPES.len(), ACTIONS.len(), STATES.len()), (8, 10, 12));
        for n in TYPES.iter().chain(ACTIONS).chain(STATES) {
            assert!(n.starts_with("_NET_WM_"), "{n}");
        }
    }

    #[test]
    fn window_json_of_a_window_without_properties() {
        let w = window_json(7, Default::default(), None, &flags());
        let keys: Vec<_> = w.as_object().unwrap().keys().cloned().collect();
        assert_eq!(
            keys,
            [
                "win_id", "win_pid", "desktop_number", "showing_desktop", "win_client_machine", "win_class",
                "win_types", "win_actions", "win_states", "win_name", "win_visible_name", "win_icon_name",
                "win_visible_icon_name", "wm_normal_hints_supplied", "wm_hints_supplied", "WM_NORMAL_HINTS", "WM_HINTS",
            ]
        );
        assert_eq!(w["win_id"], 7);
        assert_eq!(w["win_pid"], 0);
        assert_eq!(w["win_class"], "");
        assert_eq!(w["win_name"], "");
        assert_eq!(w["win_types"], json!([]));
        assert_eq!(w["WM_NORMAL_HINTS"], w["WM_HINTS"]);
    }

    #[test]
    fn window_json_with_every_property() {
        let s = AtomEnum::STRING;
        let card = AtomEnum::CARDINAL;
        let props = [
            reply32(card, &[1234]),
            reply32(card, &[u32::MAX]),
            reply32(card, &[1]),
            reply8(s, b"host"),
            reply8(s, b"app\0App\0"),
            reply32(AtomEnum::ATOM, &[atom_of("_NET_WM_WINDOW_TYPE_DIALOG")]),
            reply32(AtomEnum::ATOM, &[atom_of("_NET_WM_ACTION_CLOSE"), atom_of("_NET_WM_ACTION_MOVE")]),
            reply32(AtomEnum::ATOM, &[atom_of("_NET_WM_STATE_FULLSCREEN")]),
            reply8(UTF8, "net \u{2713}".as_bytes()),
            reply8(s, b"legacy"),
            reply8(UTF8, b"net visible"),
            reply8(s, b"legacy visible"),
            reply8(UTF8, b"icon"),
            reply8(UTF8, b"visible icon"),
            reply32(card, &[1, 2, 3, 4]),
            reply32(AtomEnum::WM_SIZE_HINTS, &[16, 0, 0, 0, 0, 10, 20, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0]),
        ];
        let geo = GetGeometryReply { depth: 24, sequence: 0, length: 0, root: 1, x: 1, y: 18, width: 300, height: 200, border_width: 0 };
        let abs = TranslateCoordinatesReply { same_screen: true, sequence: 0, length: 0, child: 0, dst_x: -20, dst_y: 30 };
        let w = window_json(9, props, Some((geo, abs)), &flags());
        assert_eq!(w["win_pid"], 1234);
        assert_eq!(w["desktop_number"], u32::MAX); // sticky windows are on every desktop
        assert_eq!(w["showing_desktop"], 1);
        assert_eq!(w["win_client_machine"], "host");
        assert_eq!(w["win_class"], "app.App");
        assert_eq!(w["win_types"][0]["flag"], "_NET_WM_WINDOW_TYPE_DIALOG");
        assert_eq!(w["win_actions"].as_array().unwrap().len(), 2);
        assert_eq!(w["win_states"][0]["flag"], "_NET_WM_STATE_FULLSCREEN");
        assert_eq!(w["win_name"], "net \u{2713}");
        assert_eq!(w["win_visible_name"], "net visible");
        assert_eq!(w["win_icon_name"], "icon");
        assert_eq!(w["win_visible_icon_name"], "visible icon");
        assert_eq!(w["win_geometry"], json!({ "width": 300, "height": 200, "x": 1, "y": 18, "abs_x": -20, "abs_y": 30 }));
        assert_eq!(w["frame_extents"], json!({ "left": 1, "right": 2, "top": 3, "bottom": 4 }));
        assert_eq!(w["wm_normal_hints_supplied"], 1023);
        assert_eq!(w["WM_NORMAL_HINTS"]["min_height"], 20);
    }

    #[test]
    fn window_json_falls_back_to_legacy_names() {
        let mut props: [Option<GetPropertyReply>; 16] = Default::default();
        props[9] = reply8(AtomEnum::STRING, b"legacy");
        props[11] = reply8(AtomEnum::STRING, b"legacy visible");
        let w = window_json(1, props, None, &flags());
        assert_eq!(w["win_name"], "legacy");
        assert_eq!(w["win_visible_name"], "legacy visible");
    }

    #[test]
    fn frame_extents_need_exactly_four_values() {
        for v in [&[1, 2, 3][..], &[1, 2, 3, 4, 5][..]] {
            let mut props: [Option<GetPropertyReply>; 16] = Default::default();
            props[14] = reply32(AtomEnum::CARDINAL, v);
            assert!(window_json(1, props, None, &flags()).get("frame_extents").is_none());
        }
    }

    #[test]
    fn matches_by_pid_and_class() {
        let w = json!({ "win_pid": 42, "win_class": "app.App" });
        assert!(matches(&w, &By::Pid(42)));
        assert!(!matches(&w, &By::Pid(43)));
        assert!(matches(&w, &By::Class("app.App".into())));
        assert!(!matches(&w, &By::Class("app".into())));
        // pid 0 means "no _NET_WM_PID": it never matches.
        assert!(!matches(&json!({ "win_pid": 0, "win_class": "" }), &By::Pid(0)));
    }

    #[test]
    fn screen_json_maps_every_field() {
        let s = Screen {
            root: 1,
            default_colormap: 32,
            white_pixel: 0xffffff,
            black_pixel: 0,
            current_input_masks: EventMask::from(7u32),
            width_in_pixels: 1280,
            height_in_pixels: 800,
            width_in_millimeters: 325,
            height_in_millimeters: 203,
            min_installed_maps: 1,
            max_installed_maps: 2,
            root_visual: 33,
            backing_stores: BackingStore::WHEN_MAPPED,
            save_unders: true,
            root_depth: 24,
            allowed_depths: vec![Depth { depth: 24, visuals: vec![] }, Depth { depth: 1, visuals: vec![] }],
        };
        assert_eq!(
            screen_json(&s),
            json!({
                "backing_store": 1, "black_pixel": 0, "cmap": 32, "height": 800, "max_maps": 2, "mheight": 203,
                "min_maps": 1, "mwidth": 325, "ndepths": 2, "root_depth": 24, "root_input_mask": 7,
                "save_unders": 1, "white_pixel": 0xffffff, "width": 1280,
            })
        );
    }

    #[test]
    fn moveresize_flags_mark_set_values() {
        assert_eq!(moveresize_flags(0, [-1; 4]), 0);
        assert_eq!(moveresize_flags(10, [-1; 4]), 10);
        assert_eq!(moveresize_flags(1, [0, 0, 1, 1]), 1 | 0xf00);
        assert_eq!(moveresize_flags(0, [5, -1, -1, 9]), 1 << 8 | 1 << 11);
        assert_eq!(moveresize_flags(0, [-2, 0, 0, 0]), 0xf00); // only -1 means untouched
    }

    #[test]
    fn fallback_geometry_matches_the_c_rules() {
        let g = |v| {
            let a = fallback_geometry(v);
            (a.x, a.y, a.width, a.height)
        };
        assert_eq!(g([10, 20, 300, 200]), (Some(10), Some(20), Some(300), Some(200)));
        assert_eq!(g([10, 20, -1, -1]), (Some(10), Some(20), None, None));
        assert_eq!(g([-1, -1, 300, 200]), (None, None, Some(300), Some(200)));
        assert_eq!(g([-1, 5, 300, -1]), (None, None, None, None));
        assert_eq!(g([0, 0, 0, 1]), (Some(0), Some(0), None, None));
    }

    #[test]
    fn steps_walk_towards_the_target_without_overflowing() {
        assert_eq!([step(0, 10, 3), step(9, 10, 3), step(10, 0, 4), step(5, 5, 1)], [3, 10, 6, 5]);
        assert_eq!([step(0, 3, 100), step(-30, -10, 10)], [3, -20]);
        assert_eq!(step(i32::MAX - 1, i32::MAX, i32::MAX), i32::MAX);
        assert_eq!(step(i32::MIN + 1, i32::MIN, i32::MAX), i32::MIN);
    }

    #[test]
    fn transition_ends_on_the_target_and_holds_finished_axes() {
        let all = |from, to, pad| transition(from, to, pad).collect::<Vec<_>>();
        assert_eq!(all([0, 0, 100, 100], [20, 10, 100, 120], 10), [[10, 10, 100, 110], [20, 10, 100, 120]]);
        assert_eq!(all([5, 5, 5, 5], [5, 5, 5, 5], 1), [[5, 5, 5, 5]]);
        assert_eq!(all([0, 0, 10, 10], [-1, -1, 12, -1], 1), [[-1, -1, 11, -1], [-1, -1, 12, -1]]);
        assert_eq!(all([0, 0, 0, 0], [-1, -1, -1, -1], 1), [[-1, -1, -1, -1]]);
        // pad <= 0 behaves like 1 instead of looping forever like the C addon.
        assert_eq!(all([0, 0, 0, 0], [2, 0, 0, 0], 0).len(), 2);
        assert_eq!(all([0, 0, 0, 0], [2, 0, 0, 0], -5).len(), 2);
        let long = all([0, 0, 50, 50], [1000, 500, 800, 600], 7);
        assert_eq!(long.len(), 143);
        assert_eq!(*long.last().unwrap(), [1000, 500, 800, 600]);
        assert!(long.windows(2).all(|p| (0..4).all(|i| p[1][i] >= p[0][i])));
        // Extreme values neither overflow nor allocate every step up front.
        assert_eq!(all([i32::MIN, 0, 0, 0], [i32::MAX, 0, 0, 0], i32::MAX).len(), 3);
        let far: Vec<_> = transition([0, 0, 0, 0], [i32::MAX, 0, 0, 0], 1).take(2).collect();
        assert_eq!(far, [[1, 0, 0, 0], [2, 0, 0, 0]]);
    }

    #[test]
    fn state_action_and_atom_names() {
        assert_eq!(state_action("REMOVE"), Some(0));
        assert_eq!(state_action("ADD"), Some(1));
        assert_eq!(state_action("TOGGLE"), Some(2));
        assert_eq!(state_action("add"), None);
        assert_eq!(state_action(""), None);
        assert_eq!(state_atom_name("maximized_vert"), "_NET_WM_STATE_MAXIMIZED_VERT");
        for s in STATES {
            assert_eq!(state_atom_name(&s.strip_prefix("_NET_WM_STATE_").unwrap().to_lowercase()), *s);
        }
    }
}
