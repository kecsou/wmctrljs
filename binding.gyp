{
    "targets":[
        {
            "target_name":"wmctrl",
            "sources": [
                "<!@(node -p 'require(`fs`).readdirSync(`./src/napi`).map(f => `./src/napi/${f}`).join(` `)')"
            ],
            "libraries": [
                "Release/wmctrl.a",
                "-lX11",
                "-lXmu",
                "-lICE"
            ]
        },
        {
            "target_name": "libwmctrl",
            "type": "static_library",
            "sources": [
                "<!@(node -p 'require(`fs`).readdirSync(`./libwmctrl/src`).map(f => `./libwmctrl/src/${f}`).join(` `)')"
            ]
        }
    ]
}