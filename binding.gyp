{
    'targets': [{
            'target_name': 'nsdsp-native',
            'sources': [
                'src/win_nsdsp.c',
                'src/mac_nsdsp.c',
                'src/linux_nsdsp.c',
                'src/nsdsp.cc'
            ],
            'conditions': [
                ['OS!="win"', {
                        'sources!': ['src/win_nsdsp.c']
                    }
                ],
                ['OS!="mac"', {
                        'sources!': ['src/mac_nsdsp.c']
                    }
                ],
                ['OS!="linux"', {
                        'sources!': ['src/linux_nsdsp.c']
                    }
                ]
            ],
            'include_dirs': ["<!@(node -p \"require('node-addon-api').include\")"],
            'dependencies': ["<!(node -p \"require('node-addon-api').gyp\")"],
            'defines':['_MY_FLAG'],
            'cflags!': ['-fno-exceptions'],
            'cflags_cc!': ['-fno-exceptions'],
            'xcode_settings': {
                'GCC_ENABLE_CPP_EXCEPTIONS': 'YES',
                'CLANG_CXX_LIBRARY': 'libc++',
                'MACOSX_DEPLOYMENT_TARGET': '10.7'
            },
            'msvs_settings': {
                'VCCLCompilerTool': {
                    'ExceptionHandling': 1
                }
            }
        }
    ]
}
