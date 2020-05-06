{
    "targets": [
        {
            "target_name": "node-freerdp2",
            "sources": [
              "rdp.cc",
              "bridge.cc",
              "generator.cc",
              "context.cc",
              "node-freerdp2.cc"
            ],
            "libraries": [
              "-lfreerdp-client2",
              "-lwinpr2",
              "-lfreerdp2",
              "-lws2_32",
              "-lcliprdr-client",
            ],

            "conditions":[
                ["OS=='win'", {
                  "library_dirs":[
                      "libs/win64/libs"
                  ],
                  "include_dirs" : [
                    "<!(node -e \"require('nan')\")",
                    "libs/win64/include",
                  ],
                  "copies":[
                      { 
                          'destination': './build/Release',
                          'files':[
                            "libs/win64/dll/freerdp-client2.dll",
                            "libs/win64/dll/freerdp2.dll",
                            "libs/win64/dll/winpr2.dll"
                          ]
                      }
                  ]}]
            ]
        }
    ],
}