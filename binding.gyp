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
              "-lfreerdp2",
              "-lfreerdp-client2",
              "-lwinpr2",
              "-lws2_32",
              "-lcliprdr-client",
            ],
            "library_dirs":[
                "D:\Playground\C\FreeRDP\Debug"
            ],
            "include_dirs" : [
             "<!(node -e \"require('nan')\")",
             "D:\Playground\C\FreeRDP\include",
             "D:\Playground\C\FreeRDP\winpr\include"
      ]
        }
    ],
}