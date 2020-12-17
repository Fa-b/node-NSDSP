const nsdsp = require('../build/Release/nsdsp-native');
const sharedStructs = require('shared-structs');

module.exports = {
Enumerate: nsdsp.NSDSPEnumerate,
Connect: nsdsp.NSDSPConnect,
Disconnect: nsdsp.NSDSPDisconnect,
GetSerial: nsdsp.NSDSPGetSerial,
GetVersion: nsdsp.NSDSPGetVersion,
GetBaudRate: nsdsp.NSDSPGetBaudRate,
GetFlowControl: nsdsp.NSDSPGetFlowControl,
GetCTS: nsdsp.NSDSPGetCTS,
GetRX: nsdsp.NSDSPGetRX,
SetMode: nsdsp.NSDSPSetMode,
SetTimeout: nsdsp.NSDSPSetTimeout,
Write: nsdsp.NSDSPWrite,
WriteCommand: nsdsp.NSDSPWriteCommand,
WriteSPI: nsdsp.NSDSPWriteSPI,
Delay: nsdsp.NSDSPDelay,
Flush: nsdsp.NSDSPFlush,
WaitForCompletion: nsdsp.NSDSPWaitForCompletion,
AvailableData: nsdsp.NSDSPAvailableData,
WaitForData: nsdsp.NSDSPWaitForData,
WaitForDataForever: nsdsp.NSDSPWaitForDataForever,
Read: nsdsp.NSDSPRead,
MAX_NSDSP_DEVICES: 32,
MAX_NSDSP_PATH: 255,
MAX_NSDSP_SERIAL: 31,

// these values are hard-coded in NSDSP progammers
// and must not be changed
NSDSP_REPORT_SIZE: 1024,
NSDSP_INPUT_SIZE: 64,
NSDSP_FEATURE_SIZE: 32,
NSDSP_MODE_SIZE: 7,

// the incoming buffer is filled by the background
// reading thread. The smaller the buffer, the more
// frequent reads are required. The default value
// of 0x40000 (256Kbytes) can hold approximately 4
// seconds of data at the highest transmission speed.
NSDSP_INPUT_BUFFER_SIZE: 0x40000,

// Timeouts are selected according to the baud rate
// but are subject to these limits
NSDSP_MIN_TIMEOUT: 2000, // ms
NSDSP_MAX_TIMEOUT: 60000, // ms

// NSDSP modes. It is important to use one of the
// listed strings. Using a wrong string will cause
// malfunction and may damage NSDSP

// Idle mode. You must return NSDSP into the Idle mode after work,
// so that the next application may connect to it without troubles
NSDSP_MODE_IDLE: "\x00\x0b\x00\x00\x67\x00\x01",

// UART modes estblish a bridge between USB and UART. All the characters
// sent to NSDSP through the USB interface are transmitted through
// the TX pin. All the characters received through the RX pin can be read
// by the program.

// UART modes with traditional baud rates without flow control
NSDSP_MODE_UART_1200: Buffer.from("\x02\x0f\x27\x43\x9b\x00\xff", 'ascii'),
NSDSP_MODE_UART_2400: Buffer.from("\x02\x87\x13\x43\x4d\x00\xff", 'ascii'),
NSDSP_MODE_UART_4800: Buffer.from("\x02\xc3\x09\x43\x26\x00\xff", 'ascii'),
NSDSP_MODE_UART_9600: Buffer.from("\x02\xe1\x04\x03\xaf\x00\xff", 'ascii'),
NSDSP_MODE_UART_19200: Buffer.from("\x02\x70\x02\x03\x57\x00\xff", 'ascii'),
NSDSP_MODE_UART_38400: Buffer.from("\x02\x38\x01\x02\xaf\x00\xff", 'ascii'),
NSDSP_MODE_UART_57600: Buffer.from("\x02\xcf\x00\x02\x74\x00\xff", 'ascii'),
NSDSP_MODE_UART_115200: Buffer.from("\x02\x67\x00\x01\xe9\x00\xff", 'ascii'),
NSDSP_MODE_UART_230400: Buffer.from("\x02\x33\x00\x01\x74\x00\xff", 'ascii'),
NSDSP_MODE_UART_460800: Buffer.from("\x02\x19\x00\x00\xe9\x00\xff", 'ascii'),
NSDSP_MODE_UART_921600: Buffer.from("\x02\x0c\x00\x00\x74\x00\xff", 'ascii'),

// UART modes with rounded baud rates without flow control
NSDSP_MODE_UART_250K: Buffer.from("\x02\x2f\x00\x01\x6b\x00\xff", 'ascii'),
NSDSP_MODE_UART_500K: Buffer.from("\x02\x17\x00\x00\xd7\x00\xff", 'ascii'),
NSDSP_MODE_UART_1M: Buffer.from("\x02\x0b\x00\x00\x6b\x00\xff", 'ascii'),
NSDSP_MODE_UART_2M: Buffer.from("\x02\x05\x00\x00\x35\x00\xff", 'ascii'),

// UART modes with traditional baud rates and flow control
NSDSP_MODE_UART_1200_FC: Buffer.from("\x02\x0f\x27\x5b\x9b\x00\xf8", 'ascii'),
NSDSP_MODE_UART_2400_FC: Buffer.from("\x02\x87\x13\x5b\x4d\x00\xf8", 'ascii'),
NSDSP_MODE_UART_4800_FC: Buffer.from("\x02\xc3\x09\x5b\x26\x00\xf8", 'ascii'),
NSDSP_MODE_UART_9600_FC: Buffer.from("\x02\xe1\x04\x03\xea\x00\xf8", 'ascii'),
NSDSP_MODE_UART_19200_FC: Buffer.from("\x02\x70\x02\x03\x75\x00\xf8", 'ascii'),
NSDSP_MODE_UART_38400_FC: Buffer.from("\x02\x38\x01\x02\xea\x00\xf8", 'ascii'),
NSDSP_MODE_UART_57600_FC: Buffer.from("\x02\xcf\x00\x02\x9b\x00\xf8", 'ascii'),
NSDSP_MODE_UART_115200_FC: Buffer.from("\x02\x67\x00\x02\x4d\x00\xf8", 'ascii'),
NSDSP_MODE_UART_230400_FC: Buffer.from("\x02\x33\x00\x01\x9b\x00\xf8", 'ascii'),
NSDSP_MODE_UART_460800_FC: Buffer.from("\x02\x19\x00\x01\x4d\x00\xf8", 'ascii'),
NSDSP_MODE_UART_921600_FC: Buffer.from("\x02\x0c\x00\x00\x9b\x00\xf8", 'ascii'),

// UART modes with rounded baud rates and flow control
NSDSP_MODE_UART_250K_FC: Buffer.from("\x02\x2f\x00\x01\x8f\x00\xf8", 'ascii'),
NSDSP_MODE_UART_500K_FC: Buffer.from("\x02\x17\x00\x01\x47\x00\xf8", 'ascii'),
NSDSP_MODE_UART_1M_FC: Buffer.from("\x02\x0b\x00\x00\x8f\x00\xf8", 'ascii'),
NSDSP_MODE_UART_2M_FC: Buffer.from("\x02\x05\x00\x00\x47\x00\xf8", 'ascii'),

// Programming modes establish a command interface. All the characters
// sent to NSDSP through the USB interface are interpreted by commands and
// executed by NSDSP. This mode can be used for SPI communications or by
// bit-banging.

// Programming modes based on SPI clock
NSDSP_MODE_PROG_50KHZ: Buffer.from("\x01\xef\x00\x02\x7b\x00\x28", 'ascii'),
NSDSP_MODE_PROG_100KHZ: Buffer.from("\x01\x77\x00\x01\xf1\x00\x14", 'ascii'),
NSDSP_MODE_PROG_250KHZ: Buffer.from("\x01\x2f\x00\x01\x61\x00\x08", 'ascii'),
NSDSP_MODE_PROG_500KHZ: Buffer.from("\x01\x17\x00\x00\xc7\x00\x04", 'ascii'),
NSDSP_MODE_PROG_1MHZ: Buffer.from("\x01\x0b\x00\x00\x67\x00\x02", 'ascii'),
NSDSP_MODE_PROG_1_5MHZ: Buffer.from("\x01\x07\x00\x00\x47\x00\x01", 'ascii'),
NSDSP_MODE_PROG_2MHZ: Buffer.from("\x01\x05\x00\x00\x37\x00\x01", 'ascii'),
NSDSP_MODE_PROG_2_5MHZ: Buffer.from("\x01\x04\x00\x00\x2f\x00\x01", 'ascii'),
NSDSP_MODE_PROG_3MHZ: Buffer.from("\x01\x03\x00\x00\x27\x00\x01", 'ascii'),
NSDSP_MODE_PROG_4MHZ: Buffer.from("\x01\x02\x00\x00\x1f\x00\x01", 'ascii'),
NSDSP_MODE_PROG_6MHZ: Buffer.from("\x01\x01\x00\x00\x17\x00\x01", 'ascii'),

// Commands should be transmitted to NSDSP with NSDSPWriteCommand()

// A command consists of a command code followed by optional parameters.
// Each command code requires fixed number of parameter bytes. If an incorrect
// command code or a wrong number of parameter bytes is sent, NSDSP will
// get desynchronized and will produce unpredictable results. The only
// way to get NSDSP out of this state is to transition to the idle mode.

// Every command takes the same time to execute, which is called "time slot".
// Some of the commands may require multiple time slots. The duration
// of the time slot is the time required to send one SPI character, and
// is determined by the programming mode, as follows:

// NSDSP_MODE_PROG_50KHZ      - 164.000 us
// NSDSP_MODE_PROG_100KHZ     -  80.333 us
// NSDSP_MODE_PROG_250KHZ     -  32.667 us
// NSDSP_MODE_PROG_500KHZ     -  16.667 us
// NSDSP_MODE_PROG_1MHZ       -   8.667 us
// NSDSP_MODE_PROG_1_5MHZ     -   6.000 us
// NSDSP_MODE_PROG_2MHZ       -   4.667 us
// NSDSP_MODE_PROG_2_5MHZ     -   4.000 us
// NSDSP_MODE_PROG_3MHZ       -   3.333 us
// NSDSP_MODE_PROG_4MHZ       -   2.667 us 
// NSDSP_MODE_PROG_6MHZ       -   2.000 us

// Since NSDSP is a HID device, the command rate is limited by the
// amount of data which can be sent through HID, which is 63000
// characters per second. This means that NSDSP is capable of
// excuting commands almost 8 times faster than the commands can
// be transferred to NSDSP. When NSDSP runs out of commands to
// execute, it stops and waits for the next command to arrive.
// If it is necessary to time the execution of commands, delay
// commands can be used. Alternatively, a slower programming mode
// may be helpful. The 500kHz and slower modes will execute commands
// slower than the commands are sent through USB which can produce
// continuous uninterrupted execution.

// When the programming mode is first entered, SPI is active on
// ICSPCLK and ICSPDAT. The SPI clock rate is determined by the mode.
// NSDSP is always a master.

// While SPI is active, it is possible to communicate with the
// target PIC through SPI. Some of the PICs may allow you to
// map their SPI module pins to ICSPCLK and ICSPDAT pins with PPS
// which greatly simplifies the communications. There's no
// separate MOSI and MISO lines (athough if NSDSP is not used for
// PIC programming the pins on the NSDSP chip are separated),
// instead there's only one ICSPDAT line. Therefore the SPI
// communications are always half duplex.

// The SPI communications use a built-in MSSP module. You can
// use the following command to tune up the MSSP module by
// manipulating CKE (Clock Edge) and SMP (Sample) bits with
// the following commands:
NSDSP_CMD_SPI_CKE_CLEAR: 0x20, // No parameters
NSDSP_CMD_SPI_CKE_SET: 0x1f, // No parameters
NSDSP_CMD_SPI_SMP_CLEAR: 0x38, // No parameters
NSDSP_CMD_SPI_SMP_SET: 0x37, // No parameters

// Data can be transmitted through SPI using the following
// commands. Sending one SPI byte takes one time slot.
// When SPI writes are followed by a non-SPI operation
// there must be at least one time slot delay between
// the last SPI write and the following non-SPI operation.

// There are 4 different commands to send 1, 2, 4, or 8 bytes
// in a row. Sending more bytes at once is more efficient
// because it requires less command bytes. You can also use
// NSDSWriteSPI() function, which select the appropriate command
// to transmit data of arbitrary length.
NSDSP_CMD_SPI_WRITE_1: 0x07, // One parameter - character to write
NSDSP_CMD_SPI_WRITE_2: 0x51, // 2 parameters - characters to write
NSDSP_CMD_SPI_WRITE_4: 0x52, // 4 parameters - characters to write
NSDSP_CMD_SPI_WRITE_8: 0x53, // 8 parameters - characters to write

// This command can be used to transmit several zero bytes in a row
// Sending each zero byte takes one time slot.
NSDSP_CMD_SPI_WRITE_Z: 0x08, // One parameter - number of zero characters to write

// The following commands initiate SPI reads. If NSDSP read buffers are
// full, these commands will be delayed until there is a space in the buffer
NSDSP_CMD_SPI_READ_1: 0x15, // No parameters
NSDSP_CMD_SPI_READ_2: 0x16, // No parameters
NSDSP_CMD_SPI_READ_3: 0x2b, // No parameters
NSDSP_CMD_SPI_READ_4: 0x17, // No parameters

// Instead of using SPI, it is possible to manipulate ICSPDAT and ICSPCLK
// pins manually, once per time slot. This can be done with one of the
// following commands, which disable SPI and put ICSPDAT and
// ICSPCLK pin into a desired state:
NSDSP_CMD_ICSP_C0_D0: 0x18, // No parameters
NSDSP_CMD_ICSP_C0_D1: 0x19, // No parameters
NSDSP_CMD_ICSP_C1_D0: 0x1a, // No parameters
NSDSP_CMD_ICSP_C1_D1: 0x1b, // No parameters

// While SPI is disabled it is possible to instruct NSDSP
// to wait until PCSPDAT pin goes low. NSDSP stops executing
// commands until ICSPDAT is driven low by the PIC. Once
// ICSP goes low, NSDSP resumes command execution.
// If ICSPDAT never goes low, the only way to stop waiting
// is to switch to the idle mode
NSDSP_CMD_WAIT_ICSPDAT: 0x22, // No parameters

// To return back to SPI, use the following command
NSDSP_CMD_ICSP_SPI: 0x1c, // No parameters

// It is possible to drive MCLR and PGM(LVP) pins regardless
// of whether SPI is enabled or not. These pins can only
// be manipulated one at a time with the following four
// commands
NSDSP_CMD_MCLR_LOW: 0x01, // No parameters
NSDSP_CMD_MCLR_HIGH: 0x02, // No parameters
NSDSP_CMD_PGM_LOW: 0x35, // No parameters
NSDSP_CMD_PGM_HIGH: 0x36, // No parameters

// There are four commands to generate delays. The NSDSP_CMD_DELAY_0
// command produces one time slot delay. Delays generated by other
// functions are specified by the parameter. You can use a combined
// NSDSDelay() function, which selects the most appropriate command
// to generate a desired delay.
NSDSP_CMD_DELAY_0: 0x21, // No parameters
NSDSP_CMD_DELAY_1: 0x03, // One parameter - delay
NSDSP_CMD_DELAY_2: 0x04, // 2 parameters - delay-1, LSB first
NSDSP_CMD_DELAY_3: 0x05, // 3 parameters - delay-2. LSB first

NSDSP_CMD_ECHO: 0x1d // One parameter - a character to send back
};

let structs = sharedStructs(`
    struct NSDSP_ENUM_STRUCT {
        char Path[` + (module.exports.MAX_NSDSP_PATH + 1) + `];
        char Serial[` + (module.exports.MAX_NSDSP_SERIAL + 1) + `];
        int VID;
        int PID;
        int Version;
    };
    struct NSDSP_ENUM_DATA {
        int NDev;
        NSDSP_ENUM_STRUCT Dev[` + module.exports.MAX_NSDSP_DEVICES + `];
    };
`);

module.exports["NSDSP_ENUM_STRUCT"] = structs.NSDSP_ENUM_STRUCT();

module.exports["NSDSP_ENUM_DATA"] = structs.NSDSP_ENUM_DATA();

