//! Test windows with known properties: `fixture <instance> <title> [count] [legacy]`.
//! Prints one window id per line once mapped, exits once the WM closed all of them.
//! `legacy` windows only carry ICCCM properties: WM_CLASS, WM_NAME and 15-long WM_NORMAL_HINTS.

use x11rb::connection::Connection;
use x11rb::protocol::Event;
use x11rb::protocol::xproto::*;
use x11rb::wrapper::ConnectionExt as _;

fn main() -> Result<(), Box<dyn std::error::Error>> {
    let args: Vec<String> = std::env::args().skip(1).collect();
    let instance = args.first().map_or("wmctrljs-fixture", |s| s);
    let title = args.get(1).map_or("fixture", |s| s);
    let count: usize = args.get(2).map_or(Ok(1), |s| s.parse())?;
    let legacy = args.get(3).is_some_and(|s| s == "legacy");

    let (c, n) = x11rb::connect(None)?;
    let screen = &c.setup().roots[n];
    let atom = |name: &str| -> Result<Atom, Box<dyn std::error::Error>> { Ok(c.intern_atom(false, name.as_bytes())?.reply()?.atom) };
    let (utf8, protocols, delete) = (atom("UTF8_STRING")?, atom("WM_PROTOCOLS")?, atom("WM_DELETE_WINDOW")?);

    let mut wins = vec![];
    for _ in 0..count {
        let win = c.generate_id()?;
        let aux = CreateWindowAux::new().background_pixel(screen.white_pixel);
        c.create_window(0, win, screen.root, 10, 10, 300, 200, 0, WindowClass::INPUT_OUTPUT, 0, &aux)?;
        let set8 = |prop: Atom, ty: Atom, v: &[u8]| c.change_property8(PropMode::REPLACE, win, prop, ty, v);
        let set32 = |prop: Atom, ty: Atom, v: &[u32]| c.change_property32(PropMode::REPLACE, win, prop, ty, v);
        let string = AtomEnum::STRING.into();
        set8(AtomEnum::WM_CLASS.into(), string, format!("{instance}\0Fixture\0").as_bytes())?;
        set8(AtomEnum::WM_NAME.into(), string, title.as_bytes())?;
        set32(protocols, AtomEnum::ATOM.into(), &[delete])?;
        if legacy {
            // PMinSize, min 120x90, in the pre-ICCCM-1.0 15 long layout.
            let hints = [16, 0, 0, 0, 0, 120, 90, 0, 0, 0, 0, 0, 0, 0, 0];
            set32(AtomEnum::WM_NORMAL_HINTS.into(), AtomEnum::WM_SIZE_HINTS.into(), &hints)?;
        } else {
            set8(atom("_NET_WM_NAME")?, utf8, format!("{title} ✓").as_bytes())?;
            set8(atom("_NET_WM_ICON_NAME")?, utf8, b"fixture-icon")?;
            set8(AtomEnum::WM_CLIENT_MACHINE.into(), string, b"fixture-host")?;
            set32(atom("_NET_WM_PID")?, AtomEnum::CARDINAL.into(), &[std::process::id()])?;
            set32(atom("_NET_WM_WINDOW_TYPE")?, AtomEnum::ATOM.into(), &[atom("_NET_WM_WINDOW_TYPE_NORMAL")?])?;
            // PMinSize|PMaxSize|PBaseSize|PWinGravity, min 100x80, max 1000x800, base 10x20, NorthWest.
            let hints = [816, 0, 0, 0, 0, 100, 80, 1000, 800, 0, 0, 0, 0, 0, 0, 10, 20, 1];
            set32(AtomEnum::WM_NORMAL_HINTS.into(), AtomEnum::WM_SIZE_HINTS.into(), &hints)?;
        }
        c.map_window(win)?;
        wins.push(win);
    }
    c.sync()?;
    for win in &wins {
        println!("{win}");
    }

    while !wins.is_empty() {
        if let Event::ClientMessage(ev) = c.wait_for_event()?
            && ev.data.as_data32()[0] == delete
        {
            c.destroy_window(ev.window)?;
            c.flush()?;
            wins.retain(|w| *w != ev.window);
        }
    }
    Ok(())
}
