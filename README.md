# wmctrljs

Be carefull this package can be use only on a linux X11 machine.

You also need packages like libx11-dev, libxmu-dev

## Update your indexes
`sudo apt update`

## Install your packages
`sudo apt install libx11-dev -y`

`sudo apt install libxmu-dev -y`


## Description
Wmctrljs is an adaptation of wmctrl command and more, with that library you can easily manage your windows very fastly beacuase wmctrls does'nt make fork of wmctrl process but directly call X11 library.

To do that wmctrls has access to an addon natif using `N-API` for scalability between nodejs version.


## Use

### Window

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
    WM_NORMAL_HINTS:XSizeHints|undefined;
    WM_HINTS:XSizeHints|undefined;
    desktop_number:number;
    showing_desktop:number;
    frame_extents:FrameExtents|undefined
}
```

#### Types

Window types are describe by this interface.
```TS
interface TypeDesc {
    flags:string
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