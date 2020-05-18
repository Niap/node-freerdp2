# node-freerdp2
Node.js addon for libfreerdp2

# Dependencies
libfreerdp-client2
libwinpr2
libfreerdp2
libcliprdr-client
libws2_32(windows)

# Features
This is based on node-freerdp,devplop with freerdp 2.0.0-rc4 ,Support remote app、clipboard.

# Example Usage
```cpp
var freerdp = require('./index');

var rdp = new freerdp.Session({
    host: "host",
    domain : null, 
    username : "username",
    password : "password",
    port: 3389, // optional
    width: 1024, // optional
    height:768, // optional
    app:"notepad", // optional
    certIgnore: true,
}).connect();
setTimeout(function(){
    rdp.sendClipboard("optional");
},3000)
setTimeout(function(){
    rdp.close();
},10000) //dalay
```
# Build with mstsc.js

https://github.com/Niap/mstsc.js

# Todo
audio support

