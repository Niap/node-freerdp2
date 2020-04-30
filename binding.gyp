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
            "library_dirs":[
                "D:\Playground\C\\x64\FreeRDP\\Release"
            ],
            "include_dirs" : [
             "<!(node -e \"require('nan')\")",
             "D:\Playground\C\\x64\FreeRDP\include",
             "D:\Playground\C\\x64\FreeRDP\winpr\include"
      ]
        }
    ],
}