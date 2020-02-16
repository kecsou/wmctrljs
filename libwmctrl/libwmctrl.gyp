{
    'targets': [
        {
            'target_name':'libwmctrl',
            'type':'static_library',
            'include_dirs':['src'],
            'conditions':[
                ['OS=="linux"', {
                    'sources':[
                        'src/desktop.c',
                        'src/screen.c',
                        'src/utils.c',
                        'src/window.c',
                        'src/window-edit.c',
                        'src/window-manager.c',
                        'src/window-move-resize.c',
                        'src/window-properties.c'
                    ]
                }]
            ],
            "libraries": [
                "-lX11",
                "-lXmu"
            ]
        }
    ]
}