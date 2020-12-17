const NSDSP = require("../lib/binding.js");
const assert = require("assert");

assert(NSDSP.Connect, "The expected function is undefined");

String.prototype.pad = function(size) {
  var s = String(this);
  while (s.length < (size || 2)) {s = "0" + s;}
  return s;
}

function testInit() {
    let desc = NSDSP.NSDSP_ENUM_DATA;
    
    console.log("\nTesting enumeration..");
    let result = NSDSP.Enumerate(desc.rawBuffer);
    assert.ok(result >= 0, "Enumerate failed");
    console.log("Enumeration complete, found %d devices:", result);
    
    let device, serial;
    for(let i = 0; i < desc.NDev; i++) {
        device = desc.Dev[i];
        serial = device.Serial.toString('utf-8');
        console.log("%s: Device VID (0x%s) PID (0x%s)", serial, device.VID.toString(16).pad(4), device.PID.toString(16).pad(4));
    }
    let version = device.Version;
    let path = device.Path.toString('utf-8')
    
    console.log("\nTesting connection to last found device");
    console.log("(S/N: %s @ %s)", serial, path);
    let handle = NSDSP.Connect(serial);
    assert.ok(handle, "Connect failed: invalid handle received");
    int_handle = handle.length >=6 ? handle.readBigUInt64LE(0) : handle.readUInt(0, handle.length);
    console.log("\nReceived valid device handle:", int_handle);

    console.log("Comparing S/N..");
    result = NSDSP.GetSerial(handle);
    assert.strictEqual(result.localeCompare(serial), 0, "GetSerial failed: invalid S/N received");
    console.log("O.K.");
    
    console.log("Comparing Version..");
    result = NSDSP.GetVersion(handle);
    assert.strictEqual(result, version, "GetVersion failed: invalid Version received");
    console.log("O.K.");
    
    let mode = NSDSP.NSDSP_MODE_UART_19200;
    
    console.log("\nSetting UART mode: 0x%s", mode.toString('hex'));
    result = NSDSP.SetMode(handle, mode);
    assert.strictEqual(result, 1, "SetMode failed");
    console.log("O.K.");
    
    let timeout = NSDSP.NSDSP_MIN_TIMEOUT;
    
    console.log("\nSetting timeout: %d ms", timeout);
    result = NSDSP.SetTimeout(handle, timeout);
    assert.strictEqual(result, timeout, "SetTimeout failed: incorrect value set");
    console.log("O.K.");
    
    console.log("\nRetrieving UART settings..");
    result = NSDSP.GetBaudRate(handle);
    assert.ok(result >= 0, "GetBaudRate failed: invalid Setting received");
    console.log("BaudRate is set to", result);
    
    result = NSDSP.GetFlowControl(handle);
    assert.ok(result >= 0 && result <= 1, "GetFlowControl failed: invalid Setting received");
    console.log("FlowControl %s", result?"enabled":"disabled");
    
    result = NSDSP.GetCTS(handle);
    assert.ok(result >= 0 && result <= 1, "GetCTS failed: invalid Setting received");
    console.log("CTS logic level is %s", result?"high":"low");
    
    result = NSDSP.GetRX(handle);
    assert.ok(result >= 0 && result <= 1, "GetRX failed: invalid Setting received");
    console.log("RX logic level is %s", result?"high":"low");
    
    
    /* TODO:
        NSDSP.Write
        NSDSP.WriteCommand
        NSDSP.WriteSPI
        NSDSP.Delay
        NSDSP.Flush
        NSDSP.WaitForCompletion
        NSDSP.AvailableData
        NSDSP.WaitForData
        NSDSP.WaitForDataForever
        NSDSP.Read
     */
    
    console.log("\nDisconnecting..");
    result = NSDSP.Disconnect(handle);
    assert.strictEqual(result, undefined, "Disconnect failed");
    console.log("Successfully disconnected from device");
}

assert.doesNotThrow(testInit, undefined, "testInit threw an expection");



console.log("Tests passed- everything looks OK!");