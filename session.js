var rdp = require('./build/Release/node-freerdp2');
var EventEmitter = require('events');

// This is from include/freerdp/input.h to simplify addon code
const PTR_FLAGS_MOVE            = 0x0800
const PTR_FLAGS_DOWN            = 0x8000
const PTR_FLAGS_BUTTON1         = 0x1000 /* left */
const PTR_FLAGS_BUTTON2         = 0x2000 /* right */
const PTR_FLAGS_BUTTON3         = 0x4000 /* middle */

const PTR_FLAGS_HWHEEL          = 0x0400
const PTR_FLAGS_WHEEL           = 0x0200
const PTR_FLAGS_WHEEL_NEGATIVE  = 0x0100

class Session extends EventEmitter {
  constructor(options) {
    super();
    this.host = options.host;
    this.username = options.username;
    this.password = options.password;
    this.s_domain = options.domain;// EventEmitter 本省有个domain
    this.port = options.port || 3389;
    this.width = options.width || 1366;
    this.height = options.height || 768;
    this.bitsPerPixel = 24;
    this.certIgnore = options.certIgnore;
    this.app = options.app;
  }

  sendKeyEventScancode(code, pressed) {
    rdp.sendKeyEventScancode(this._sessionIndex, code, pressed);
  }

  sendWheelEvent(x, y, step, isNegative, isHorizontal){
    var flags = 0;
    if (isHorizontal)
      flags |= PTR_FLAGS_HWHEEL;
    else
      flags |= PTR_FLAGS_WHEEL;
    

    if (isNegative)
    {
      flags |= PTR_FLAGS_WHEEL_NEGATIVE;
    }
    flags += step;
    rdp.sendPointerEvent(this._sessionIndex, flags, x, y);
  }

  sendPointerEvent(x, y,button,isPressed) {
    var flags = 0;

    x = x || 0;
    y = y || 0;


    if (button == 1 && isPressed) flags |= PTR_FLAGS_BUTTON1 | PTR_FLAGS_DOWN;
    if (button == 2 && isPressed) flags |= PTR_FLAGS_BUTTON2 | PTR_FLAGS_DOWN;

    if (button == 1 && !isPressed) flags |= PTR_FLAGS_BUTTON1;
    if (button == 2 && !isPressed) flags |= PTR_FLAGS_BUTTON2;
    
    if (x !== null && y !== null && flags == 0) {
      flags |= PTR_FLAGS_MOVE;
    }

    rdp.sendPointerEvent(this._sessionIndex, flags, x, y);
  }

  setClipboard(val) {
    rdp.setClipboard(this._sessionIndex, val);
  }

  close() {
    rdp.close(this._sessionIndex);
  }

  _eventHandler(event, args) {
    args.unshift(event);
    this.emit.apply(this, args);
  }

  connect() {
    var params = [];

    params.push(`/v:${this.host}`);
    params.push(`/u:${this.username}`);
    params.push(`/p:${this.password}`);
    params.push(`/w:${this.width}`);
    params.push(`/h:${this.height}`);
    params.push(`/bpp:${this.bitsPerPixel}`);

    params.push('-clipboard');
    //params.push('/log-level:debug')


    if (this.app) {
      params.push(`/app:||${this.app}`);
    }
    
    if (this.certIgnore) {
      params.push('/cert-ignore');
    }

    if (this.s_domain) {
      params.push(`/d:${this.s_domain}`);
    }

    if (this.port) {
      params.push(`/port:${this.port}`);
    }

    this._sessionIndex = rdp.connect(params, this._eventHandler.bind(this));
    console.log(this._sessionIndex);
    return this;
  }
}

module.exports = Session;
