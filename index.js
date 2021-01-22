const binding = require("./lib/binding.js");

const defaultSettings = Object.freeze({
    autoOpen: true,
    mode: binding.NSDSP_MODE_IDLE,
    flowcontrol: false
});

const Interfaces = Object.freeze({
    IDLE: 0x00,
    SPI: 0x01,
    PROG: 0x01,
    UART: 0x02
});

const Commands = Object.freeze({
    SPI_CKE_CLEAR: 0x20,
    SPI_CKE_SET: 0x1F,
    SPI_SMP_CLEAR: 0x38,
    SPI_SMP_SET: 0x37,
    SPI_WRITE_1: 0x07,
    SPI_WRITE_2: 0x51,
    SPI_WRITE_4: 0x52,
    SPI_WRITE_8: 0x53,
    SPI_WRITE_Z: 0x08,
    SPI_READ_1: 0x15,
    SPI_READ_2: 0x16,
    SPI_READ_3: 0x2B,
    SPI_READ_4: 0x17,
    ICSP_C0_D0: 0x18,
    ICSP_C0_D1: 0x19,
    ICSP_C1_D0: 0x1A,
    ICSP_C1_D1: 0x1B,
    WAIT_ICSPDAT: 0x22,
    ICSP_SPI: 0x1C,
    MCLR_LOW: 0x01,
    MCLR_HIGH: 0x02,
    PGM_LOW: 0x35,
    PGM_HIGH: 0x36,
    DELAY_0: 0x21,
    DELAY_1: 0x03,
    DELAY_2: 0x04,
    DELAY_3: 0x05,
    ECHO: 0x1D,
    HIDDEN_ICSP_CLK_ONCE: 0x06,
    HIDDEN_DUNNO_WITH_WORD: 0x0E,
    HIDDEN_SEND_COMMAND: 0x27,
    HIDDEN_SEND_COMMAND_WITH_WORD: 0x28,
    HIDDEN_WRITE_WORD: 0x2A,
    HIDDEN_READ_WORD: 0x2C,
    HIDDEN_INC_PC: 0x2E,
    HIDDEN_WRITE_4_WORDS: 0x2F,
    HIDDEN_READ_4_WORDS: 0x31,

});

let analyzer = function(key, buffer) {
    let T2CKPS = 1 << (2 * (buffer[3] & 0x03));
    let T2OUTPS = ((buffer[3] & 0x78) >> 3) + 1;
    let PR2 = buffer[4] + 1;
    let timeslot = T2OUTPS * T2CKPS * PR2 * (1 / 12) 
    //console.log("%s: 12 MHz / %d (Prescale 1:%d, Postscale 1:%d, Iterations %d, Timeslot %d us); unknown flags: 0x%s 0x%s", key, buffer.readUInt16LE(1) + 1, T2CKPS, T2OUTPS, PR2, timeslot.toFixed(3), buffer[5].toString(16), buffer[6].toString(16));

    return timeslot;
}

function NSDSP(serial, options, openCallback) {
    if (!(this instanceof NSDSP)) {
        return new NSDSP(serial, options, openCallback)
    }
    
    if (options instanceof Function) {
        openCallback = options
        options = {}
    }
    
    this.settings = { ...defaultSettings, ...options }
    
    if(typeof serial !== 'string')
        throw new Error('Invalid type ' + typeof serial);
    
    this.serial = serial;
    this.opening = false;
    this.closing = false;
    this.binding = binding;

    this._Mode = new function(settings, flowcontrol) {
        if (settings.length !== binding.NSDSP_MODE_SIZE) {
            throw new Error('Invalid settings, setting an unsupported mode may damage the NSDSP device.. aborting');
        }
        this._settings = settings;
        this._flow_control = flowcontrol?0xF8:0xFF;

        this._bits_per_packet = () => {
            let bits_per_packet = 0;
            switch(this.mode) {
                case 'IDLE':
                case 'SPI':
                case 'PROG':
                    bits_per_packet = 8;
                    break;
                case 'UART':
                    bits_per_packet = this.flow_control?12:9;
                    break;
                default:
                    throw new Error('Invalid COM Interface: ' + this.mode);
            }
    
            return bits_per_packet;
        };
    
        this._set_timing = () => {
            let norm_min_timeslot = 1 / this.bit_rate * this._bits_per_packet() * 12e6 + 8;
    
            if(norm_min_timeslot < 24) // 2 us lower boundary (thats 62.5k Characters via HID @ 8 bits per packet)
                norm_min_timeslot = 24;
    
            let norm_tick = 255 + 1;
            let pre_scalar = 0;
            let post_scalar = 0;
    
            while(pre_scalar < 3) { // 1:1, 1:4, 1:16, 1:64
                if(norm_tick >= norm_min_timeslot)
                    break;
                ++pre_scalar;
                norm_tick *= 4;
            }
    
            this._settings[3] = pre_scalar & 0x03;
    
            while(post_scalar < 15) { // 1:1, 1:2, ..., 1:15, 1:16
                if(norm_tick >= norm_min_timeslot)
                    break;
                    norm_tick += (norm_tick / ++post_scalar);
            }
    
            this._settings[3] |= (post_scalar << 3) & 0x78;
    
            if(norm_tick < norm_min_timeslot)
                throw new Error('The requested settings are out of bounds! Try using a higher bit_rate');
    
            let multiplier = (1 << (2 * pre_scalar)) * (post_scalar + 1);
            while(norm_tick > 2 && ((--norm_tick * multiplier) >= norm_min_timeslot));
    
            this._settings[4] = norm_tick & 0xFF;

            if(this._settings[0] === Interfaces['PROG']) { // This seems to fit:
                this._settings[6] = Math.floor(2e6 / this.bit_rate);//Math.floor((norm_tick + 1) / 256 / (16 / (1 << (2 * pre_scalar))) * 100);
                this._settings[6] = (this._settings[6] === 0)?1:this._settings[6];
            }

            //console.log(this._settings);
            this._slottime = analyzer(this.bit_rate, this._settings);
        };
    }(this.settings.mode, this.settings.flowcontrol);

    Object.defineProperty(this._Mode, "mode", {
        get: function() {
            return Object.keys(Interfaces).find(mode => Interfaces[mode] === this._settings[0]);
        },
        set: function(mode) {
            if(Interfaces.hasOwnProperty(mode)) {
                this._settings[0] = Interfaces[mode];

                if(mode === 'UART')
                    this._settings[6] = this._flow_control;
    
                this._set_timing();
            }
        }
    });

    Object.defineProperty(this._Mode, "bit_rate", {
        get: function() {
            return 12e6 / (this._settings.readUInt16LE(1) + 1);
        },
        set: function(bit_rate) {
            let div = Math.round(12e6 / bit_rate - 1);
            div = (div < 1)?1:div;
            this._settings[1] = div & 0x00FF;
            this._settings[2] = (div & 0xFF00) >> 8;
    
            this._set_timing();
        }
    });

    Object.defineProperty(this._Mode, "flow_control", {
        get: function() {
            return (this.mode === 'UART') && (this._settings[6]===0xF8);
        },
        set: function(flow_control) {
            if(this.mode === 'UART') {
                this._settings[6] = flow_control?0xF8:0xFF;
                this._flow_control = this._settings[6];
    
                this._set_timing();
            } else {
                throw new Error('Flow Control can only be accessed for the UART interface');
            }
        }
    });

    if (this.settings.autoOpen) {
        this.open(openCallback);
        this.setMode(this.settings.mode);
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
    this.handle = this.binding.Connect(this.serial);
    this.opening = false;
    
    this.isOpen = !!this.handle;
    
    if(openCallback) {
        openCallback.call(!this.isOpen?"Error connecting to device":null, this);
    }

    return this.isOpen;
}

NSDSP.prototype.close = function(closeCallback) {
    if (!this.isOpen) {
        throw new Error('Port is not open');
    }
    
    this.closing = true;
    let result = this.binding.Disconnect(this.handle);
    this.closing = false;
    
    this.isOpen = !!result;
    
    if(closeCallback) {
        closeCallback.call(this.isOpen?"Error disconnecting from device":null, this);
    }

    return !this.isOpen;
}

NSDSP.prototype._setConfiguration = function(mode) {
    if (!this.isOpen) {
        throw new Error('Port is not open');
    }

    if (mode.length !== binding.NSDSP_MODE_SIZE) {
        throw new Error('Invalid mode, setting an unsupported mode may damage the NSDSP device.. aborting');
    }

    this._Mode._settings = mode;

    return this.binding.SetMode(this.handle, mode)?true:false;
}

NSDSP.prototype.getSerial = function() {
    if (!this.isOpen) {
        throw new Error('Port is not open');
    }

    return this.binding.GetSerial(this.handle);
}

NSDSP.prototype.getVersion = function() {
    if (!this.isOpen) {
        throw new Error('Port is not open');
    }

    let result = this.binding.GetVersion(this.handle);
    
    // BCD to version string
    return Array.from(result.toString(16)).reduceRight( (prev, cur) => (prev.length % 2 === 0)?cur.concat('', '.' + prev):cur.concat('', prev) );
}

NSDSP.prototype.setBitrate = function(bitrate) {
    this._Mode.bit_rate = bitrate;
    if (!this.isOpen) {
        throw new Error('Port is not open');
    }
    return this.binding.SetMode(this.handle, this._Mode._settings)?true:false;
}

NSDSP.prototype.getBitrate = function() {
    if (!this.isOpen) {
        throw new Error('Port is not open');
    }

    return this.binding.GetBaudrate(this.handle);
}

NSDSP.prototype.setFlowControl = function(flow_control) {
    this._Mode.flow_control = flow_control;
    if (!this.isOpen) {
        throw new Error('Port is not open');
    }
    return this.binding.SetMode(this.handle, this._Mode._settings)?true:false;
}

NSDSP.prototype.getFlowControl = function() {
    if (!this.isOpen) {
        throw new Error('Port is not open');
    }

    return this.binding.GetFlowControl(this.handle)?true:false;
}

NSDSP.prototype.getCTS = function() {
    if (!this.isOpen) {
        throw new Error('Port is not open');
    }

    let result = this.binding.GetCTS(this.handle);

    if(result < 0) {
        throw new Error('Reading CTS logic level from device %s failed', this.serial);
    }

    return result?'high':'low';
}

NSDSP.prototype.getRX = function() {
    if (!this.isOpen) {
        throw new Error('Port is not open');
    }

    let result = this.binding.GetRX(this.handle);

    if(result < 0) {
        throw new Error('Reading RX logic level from device %s failed', this.serial);
    }

    return result?'high':'low';
}

NSDSP.prototype.setMode = function(mode) {
    if(Interfaces.hasOwnProperty(mode)) {
        this._Mode.mode = mode;
        if (!this.isOpen) {
            throw new Error('Port is not open');
        }
        return this.binding.SetMode(this.handle, this._Mode._settings)?true:false;
    } else {
        return this._setConfiguration(mode);
    }
}

NSDSP.prototype.getMode = function() {
    return this._Mode.mode;
}

NSDSP.prototype.setTimeout = function(timeout) {
    if (!this.isOpen) {
        throw new Error('Port is not open');
    }

    return this.binding.SetTimeout(this.handle, timeout);
}

NSDSP.prototype.write = function(data, size) {
    if (!this.isOpen) {
        throw new Error('Port is not open');
    }

    return this.binding.Write(this.handle, data, data?(size?size:data.length):0)?true:false;
}

NSDSP.prototype.writeCommand = function(cmd, data, size) {
    if (!this.isOpen) {
        throw new Error('Port is not open');
    }

    if (!Commands.hasOwnProperty(cmd)) {
        throw new Error('Unknown Command: ' + cmd);
    }

    return this.binding.WriteCommand(this.handle, Commands[cmd], data?data:Buffer.alloc(0), data?(size?size:data.length):0)?true:false;
}

NSDSP.prototype.writeSPI = function(data, size) {
    if (!this.isOpen) {
        throw new Error('Port is not open');
    }

    return this.binding.WriteSPI(this.handle, data, data?(size?size:data.length):0)?true:false;
}

NSDSP.prototype.delay = function(delay) {
    if (!this.isOpen) {
        throw new Error('Port is not open');
    }

    return this.binding.Delay(this.handle, delay)?true:false;
}

NSDSP.prototype.delay_us = function(delay) {
    if (!this.isOpen) {
        throw new Error('Port is not open');
    }
    
    return this.binding.Delay(this.handle, Math.ceil(delay / this._Mode._slottime))?true:false;
}

NSDSP.prototype.delay_ms = function(delay) {
    if (!this.isOpen) {
        throw new Error('Port is not open');
    }

    return this.binding.Delay(this.handle, Math.ceil(delay * 1000 / this._Mode._slottime))?true:false;
}

NSDSP.prototype.flush = function() {
    if (!this.isOpen) {
        throw new Error('Port is not open');
    }

    return this.binding.Flush(this.handle)?true:false;
}

NSDSP.prototype.echo = function(data, size) {
    let count = data?(size?size:data.length):0;
    let buffer = Buffer.alloc(count);

    if (!this.isOpen) {
        throw new Error('Port is not open');
    }

    for(let i = 0; i < count; i++) {
        if(this.binding.WriteCommand(this.handle, 0x1D, Buffer.from(data[i]), 1) < 1)
            throw new Error('An error occurred writing: ' + data[i]);
    }

    this.binding.Flush(this.handle);

    if(this.binding.WaitForData(this.handle, count) < 1) {
        throw new Error('No echo received');
    }

    if(count != this.binding.Read(this.handle, buffer, count)) {
        throw new Error('Not enough echo data received');
    }

    return buffer.toString();
}

NSDSP.enumerate = function (callback) {
    let list = binding.NSDSP_ENUM_DATA;
    if(binding.Enumerate(list.rawBuffer) < 0) {
        throw new Error('An error occurred attempting to gather a list of NSDSP devices');
    }

    /*Object.keys(binding).filter( key => binding[key].length === 7).forEach(key => {
        analyzer(key, binding[key]);
    });*/
    
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