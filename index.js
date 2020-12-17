const binding = require("./lib/binding.js");

const defaultSettings = Object.freeze({
    autoOpen: true
});

function NSDSP(serial, options, openCallback) {
    if (!(this instanceof NSDSP)) {
        return new NSDSP(serial, options, openCallback)
    }
    
    if (options instanceof Function) {
        openCallback = options
        options = {}
    }
    
    this.settings = { ...defaultSettings, ...options }
    
    this.serial = serial;
    this.opening = false;
    this.closing = false;
  
    if (this.settings.autoOpen) {
        this.open(openCallback)
    }
}

NSDSP.prototype.open = function(openCallback) {
    if (this.isOpen) {
        throw new Error('Port is already open');
    }

    if (this.opening) {
        throw new Error('Port is opening');
    }

    this.opening = true;
    this.handle = binding.Connect(this.serial);
    this.opening = false;
    
    if(!this.handle) {
        throw new TypeError('Connection to device %s failed', this.serial);
    }
    
    this.isOpen = true;
    console.log("opened", this.serial);
    
    if(openCallback) {
        openCallback.call(this, null);
    }
}

NSDSP.prototype.close = function(closeCallback) {
    if (!this.isOpen) {
        throw new Error('Port is not open');
    }
    
    this.closing = true;
    let result = binding.Disconnect(this.handle);
    this.closing = false;
    
    if(result) {
        throw new TypeError('Disconnecting from device %s failed', this.serial);
    }
    
    this.isOpen = false;
    console.log("closed", this.serial);
    
    if(closeCallback) {
        closeCallback.call(this, null);
    }
}

NSDSP.enumerate = function (callback) {
    let list = binding.NSDSP_ENUM_DATA;
    if(binding.Enumerate(list.rawBuffer) < 0) {
        throw new TypeError('An error occurred attempting to gather a list of NSDSP devices');
    }
    
    let retVal = {
        NDev: list.NDev,
        Dev: []
    };
    
    for(let i = 0; i < list.NDev; i++) {
        retVal.Dev[i] = {
            Path: list.Dev[i].Path.toString().replace(/\0/g, ''),
            Serial: list.Dev[i].Serial.toString().replace(/\0/g, ''),
            VID: list.Dev[i].VID,
            PID: list.Dev[i].PID,
            Version: list.Dev[i].Version
        };
    }
    
    callback(retVal);
}

module.exports = NSDSP;