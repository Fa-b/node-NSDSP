/***************************************************************************
 * (C) 2017-2019 Northern Software Inc                                     *
 *                                                                         *
 * This file is free software provided by Northern Software Inc.           *
 * to demostrate communications through NSDSP programmers.                 *
 * This software may be used for any purpose unless prohinited by law.     *
 *                                                                         *
 * THE SOFTWARE IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND.          *
 * THE VENDOR DISCLAIMS ALL WARRANTIES RELATING TO THIS SOFTWARE, WHETHER  *
 * EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO ANY IMPLIED          *
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE, AND *
 * ALL SUCH WARRANTIES ARE EXPRESSLY AND SPECIFICALLY DISCLAIMED.          *
 * IN NO EVENT SHALL THE VENDOR OR ANYONE ELSE WHO HAS BEEN INVOLVED IN    *
 * THE CREATION, PRODUCTION, OR DELIVERY OF THIS SOFTWARE SHALL BE LIABLE  *
 * FOR ANY DIRECT, INDIRECT, CONSEQUENTIAL, OR INCIDENTAL DAMAGES ARISING  *
 * OUT OF THE USE OR INABILITY TO USE SUCH SOFTWARE EVEN IF THE VENDOR HAS *
 * BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES OR CLAIMS.              * 
 ***************************************************************************/
#ifndef NSDSP_H
#define NSDSP_H

// Each exportable function includes NSDSP_EXPORT as part of the definition
// If necessary, NSDSP_EXPORT may be defined to add attributes to every
// function such as __declspec(dllexport) or extern "C", or change
// the calling conventions
// Such definition must precede #include "nsuart.h"
#ifndef NSDSP_EXPORT
    #define NSDSP_EXPORT(RET) RET
#endif

#define MAX_NSDSP_DEVICES   32
#define MAX_NSDSP_PATH     255
#define MAX_NSDSP_SERIAL    31

// these values are hard-coded in NSDSP progammers
// and must not be changed
#define NSDSP_REPORT_SIZE   1024
#define NSDSP_INPUT_SIZE      64
#define NSDSP_FEATURE_SIZE    32
#define NSDSP_MODE_SIZE        7

// the incoming buffer is filled by the background
// reading thread. The smaller the buffer, the more
// frequent reads are required. The default value
// of 0x40000 (256Kbytes) can hold approximately 4
// seconds of data at the highest transmission speed.
#define NSDSP_INPUT_BUFFER_SIZE  0x40000

// Timeouts are selected according to the baud rate
// but are subject to these limits
#define NSDSP_MIN_TIMEOUT   2000 // ms
#define NSDSP_MAX_TIMEOUT  60000 // ms

// NSDSP modes. It is important to use one of the
// listed strings. Using a wrong string will cause
// malfunction and may damage NSDSP

// Idle mode. You must return NSDSP into the Idle mode after work,
// so that the next application may connect to it without troubles
#define NSDSP_MODE_IDLE           (unsigned char*)"\x00\x0b\x00\x00\x67\x00\x01"

// UART modes estblish a bridge between USB and UART. All the characters
// sent to NSDSP through the USB interface are transmitted through
// the TX pin. All the characters received through the RX pin can be read
// by the program.

// UART modes with traditional baud rates without flow control
#define NSDSP_MODE_UART_1200      (unsigned char*)"\x02\x0f\x27\x43\x9b\x00\xff"
#define NSDSP_MODE_UART_2400      (unsigned char*)"\x02\x87\x13\x43\x4d\x00\xff"
#define NSDSP_MODE_UART_4800      (unsigned char*)"\x02\xc3\x09\x43\x26\x00\xff"
#define NSDSP_MODE_UART_9600      (unsigned char*)"\x02\xe1\x04\x03\xaf\x00\xff"
#define NSDSP_MODE_UART_19200     (unsigned char*)"\x02\x70\x02\x03\x57\x00\xff"
#define NSDSP_MODE_UART_38400     (unsigned char*)"\x02\x38\x01\x02\xaf\x00\xff"
#define NSDSP_MODE_UART_57600     (unsigned char*)"\x02\xcf\x00\x02\x74\x00\xff"
#define NSDSP_MODE_UART_115200    (unsigned char*)"\x02\x67\x00\x01\xe9\x00\xff"
#define NSDSP_MODE_UART_230400    (unsigned char*)"\x02\x33\x00\x01\x74\x00\xff"
#define NSDSP_MODE_UART_460800    (unsigned char*)"\x02\x19\x00\x00\xe9\x00\xff"
#define NSDSP_MODE_UART_921600    (unsigned char*)"\x02\x0c\x00\x00\x74\x00\xff"

// UART modes with rounded baud rates without flow control
#define NSDSP_MODE_UART_250K      (unsigned char*)"\x02\x2f\x00\x01\x6b\x00\xff"
#define NSDSP_MODE_UART_500K      (unsigned char*)"\x02\x17\x00\x00\xd7\x00\xff"
#define NSDSP_MODE_UART_1M        (unsigned char*)"\x02\x0b\x00\x00\x6b\x00\xff"
#define NSDSP_MODE_UART_2M        (unsigned char*)"\x02\x05\x00\x00\x35\x00\xff"

// UART modes with traditional baud rates and flow control
#define NSDSP_MODE_UART_1200_FC   (unsigned char*)"\x02\x0f\x27\x5b\x9b\x00\xf8"
#define NSDSP_MODE_UART_2400_FC   (unsigned char*)"\x02\x87\x13\x5b\x4d\x00\xf8"
#define NSDSP_MODE_UART_4800_FC   (unsigned char*)"\x02\xc3\x09\x5b\x26\x00\xf8"
#define NSDSP_MODE_UART_9600_FC   (unsigned char*)"\x02\xe1\x04\x03\xea\x00\xf8"
#define NSDSP_MODE_UART_19200_FC  (unsigned char*)"\x02\x70\x02\x03\x75\x00\xf8"
#define NSDSP_MODE_UART_38400_FC  (unsigned char*)"\x02\x38\x01\x02\xea\x00\xf8"
#define NSDSP_MODE_UART_57600_FC  (unsigned char*)"\x02\xcf\x00\x02\x9b\x00\xf8"
#define NSDSP_MODE_UART_115200_FC (unsigned char*)"\x02\x67\x00\x02\x4d\x00\xf8"
#define NSDSP_MODE_UART_230400_FC (unsigned char*)"\x02\x33\x00\x01\x9b\x00\xf8"
#define NSDSP_MODE_UART_460800_FC (unsigned char*)"\x02\x19\x00\x01\x4d\x00\xf8"
#define NSDSP_MODE_UART_921600_FC (unsigned char*)"\x02\x0c\x00\x00\x9b\x00\xf8"

// UART modes with rounded baud rates and flow control
#define NSDSP_MODE_UART_250K_FC   (unsigned char*)"\x02\x2f\x00\x01\x8f\x00\xf8"
#define NSDSP_MODE_UART_500K_FC   (unsigned char*)"\x02\x17\x00\x01\x47\x00\xf8"
#define NSDSP_MODE_UART_1M_FC     (unsigned char*)"\x02\x0b\x00\x00\x8f\x00\xf8"
#define NSDSP_MODE_UART_2M_FC     (unsigned char*)"\x02\x05\x00\x00\x47\x00\xf8"

// Programming modes establish a command interface. All the characters
// sent to NSDSP through the USB interface are interpreted by commands and
// executed by NSDSP. This mode can be used for SPI communications or by
// bit-banging.

// Programming modes based on SPI clock
#define NSDSP_MODE_PROG_50KHZ     (unsigned char*)"\x01\xef\x00\x02\x7b\x00\x28"
#define NSDSP_MODE_PROG_100KHZ    (unsigned char*)"\x01\x77\x00\x01\xf1\x00\x14"
#define NSDSP_MODE_PROG_250KHZ    (unsigned char*)"\x01\x2f\x00\x01\x61\x00\x08"
#define NSDSP_MODE_PROG_500KHZ    (unsigned char*)"\x01\x17\x00\x00\xc7\x00\x04"
#define NSDSP_MODE_PROG_1MHZ      (unsigned char*)"\x01\x0b\x00\x00\x67\x00\x02"
#define NSDSP_MODE_PROG_1_5MHZ    (unsigned char*)"\x01\x07\x00\x00\x47\x00\x01"
#define NSDSP_MODE_PROG_2MHZ      (unsigned char*)"\x01\x05\x00\x00\x37\x00\x01"
#define NSDSP_MODE_PROG_2_5MHZ    (unsigned char*)"\x01\x04\x00\x00\x2f\x00\x01"
#define NSDSP_MODE_PROG_3MHZ      (unsigned char*)"\x01\x03\x00\x00\x27\x00\x01"
#define NSDSP_MODE_PROG_4MHZ      (unsigned char*)"\x01\x02\x00\x00\x1f\x00\x01"
#define NSDSP_MODE_PROG_6MHZ      (unsigned char*)"\x01\x01\x00\x00\x17\x00\x01"

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
#define NSDSP_CMD_SPI_CKE_CLEAR  0x20 // No parameters
#define NSDSP_CMD_SPI_CKE_SET    0x1f // No parameters
#define NSDSP_CMD_SPI_SMP_CLEAR  0x38 // No parameters
#define NSDSP_CMD_SPI_SMP_SET    0x37 // No parameters

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
#define NSDSP_CMD_SPI_WRITE_1    0x07 // One parameter - character to write
#define NSDSP_CMD_SPI_WRITE_2    0x51 // 2 parameters - characters to write
#define NSDSP_CMD_SPI_WRITE_4    0x52 // 4 parameters - characters to write
#define NSDSP_CMD_SPI_WRITE_8    0x53 // 8 parameters - characters to write

// This command can be used to transmit several zero bytes in a row
// Sending each zero byte takes one time slot.
#define NSDSP_CMD_SPI_WRITE_Z    0x08 // One parameter - number of zero characters to write

// The following commands initiate SPI reads. If NSDSP read buffers are
// full, these commands will be delayed until there is a space in the buffer
#define NSDSP_CMD_SPI_READ_1     0x15 // No parameters
#define NSDSP_CMD_SPI_READ_2     0x16 // No parameters
#define NSDSP_CMD_SPI_READ_3     0x2b // No parameters
#define NSDSP_CMD_SPI_READ_4     0x17 // No parameters

// Instead of using SPI, it is possible to manipulate ICSPDAT and ICSPCLK
// pins manually, once per time slot. This can be done with one of the
// following commands, which disable SPI and put ICSPDAT and
// ICSPCLK pin into a desired state:
#define NSDSP_CMD_ICSP_C0_D0     0x18 // No parameters
#define NSDSP_CMD_ICSP_C0_D1     0x19 // No parameters
#define NSDSP_CMD_ICSP_C1_D0     0x1a // No parameters
#define NSDSP_CMD_ICSP_C1_D1     0x1b // No parameters

// While SPI is disabled it is possible to instruct NSDSP
// to wait until PCSPDAT pin goes low. NSDSP stops executing
// commands until ICSPDAT is driven low by the PIC. Once
// ICSP goes low, NSDSP resumes command execution.
// If ICSPDAT never goes low, the only way to stop waiting
// is to switch to the idle mode
#define NSDSP_CMD_WAIT_ICSPDAT   0x22 // No parameters

// To return back to SPI, use the following command
#define NSDSP_CMD_ICSP_SPI       0x1c // No parameters

// It is possible to drive MCLR and PGM(LVP) pins regardless
// of whether SPI is enabled or not. These pins can only
// be manipulated one at a time with the following four
// commands
#define NSDSP_CMD_MCLR_LOW       0x01 // No parameters
#define NSDSP_CMD_MCLR_HIGH      0x02 // No parameters
#define NSDSP_CMD_PGM_LOW        0x35 // No parameters
#define NSDSP_CMD_PGM_HIGH       0x36 // No parameters

// There are four commands to generate delays. The NSDSP_CMD_DELAY_0
// command produces one time slot delay. Delays generated by other
// functions are specified by the parameter. You can use a combined
// NSDSDelay() function, which selects the most appropriate command
// to generate a desired delay.
#define NSDSP_CMD_DELAY_0        0x21 // No parameters
#define NSDSP_CMD_DELAY_1        0x03 // One parameter - delay
#define NSDSP_CMD_DELAY_2        0x04 // 2 parameters - delay-1, LSB first
#define NSDSP_CMD_DELAY_3        0x05 // 3 parameters - delay-2. LSB first

#define NSDSP_CMD_ECHO           0x1d // One parameter - a character to send back


// This structure is used to hold the information about NSDSP devices
// connected to the computer. It is used during enumeration by the
// NSDSPEnumerate() function.
typedef struct {
  char Path[MAX_NSDSP_PATH+1];
  char Serial[MAX_NSDSP_SERIAL+1];
  unsigned int VID, PID, Version;
} NSDSP_ENUM_STRUCT;

typedef struct {
  int NDev;
  NSDSP_ENUM_STRUCT Dev[MAX_NSDSP_DEVICES];
} NSDSP_ENUM_DATA;

// When you establish a connection with NSDSP, the
// NSDSP_CONN_HANDLE holds all the connection information.
// It is returned by NSDSPConnect() and then is used
// by other functions until it is destroyed by a
// call to NSDSPDisconnect()
typedef struct NSDSP_CONN_DATA *NSDSP_CONN_HANDLE;

// -------------------------------------------------------------------
// NSDSPEnumerate - enumerates all connected NSDSP devices
// 
// Dev       the structure to be filled with the enumeration
//           information which can hold up to MAX_NSDSP_DEVICES
//
// Return    the number of NSDSP devices enumerated or -1 in case of
//           an error 
//
// NSDSPEnumerate() can be used to get the list of NSDSP devices
// currently plugged in. The Serial numbers of the NSDSP devices
// returned by this function can be used in NSDSPConnect() to
// connect to a specified NSDSP device. 
//
// If there's only one NSDSP present, or if the Serial number of
// the desired NSDSP device is already known, it is not necessary
// to call NSDSPEnumerate()
// -------------------------------------------------------------------
NSDSP_EXPORT(int) NSDSPEnumerate(NSDSP_ENUM_DATA *Dev);

// -------------------------------------------------------------------
// NSDSPConnect - establishes a connection with NSDSP devices
// 
// Serial    the serial number of the NSDSP device. May be NULL.
//           If NULL is specified and there is only one NSDSP
//           device present, NSDSPConnect() connects to this
//           NSDSP device. If NULL is specified when multiple
//           NSDSP devices are present, NSDSPConnect() connects
//           to the first NSDSP device on the enumeration list
//
// Return    the handle to the connection. It can be used in
//           subsequent functions as needed. After use, it must
//           be destroyed with NSDSPDisconnect()
//
// Only one connection can be established to any specific NSDSP device
// at a time. If there are multiple NSDSP devices, you can establish
// multiple independent connections to each of the devices. An attempt
// to establish multiple connections to the same NSDSP produces
// unpredictable results.
// -------------------------------------------------------------------
NSDSP_EXPORT(NSDSP_CONN_HANDLE) NSDSPConnect(char *Serial);

// -------------------------------------------------------------------
// NSDSPDisconnect - destroys an NSDSP connection
// 
// Conn      the connection handle returned by NSDSPConnect().
//           After calling NSDSPDisconnect(), the handle
//           becomes invalid and can no longer be used.
//
// Return    none
//
// All connections established with NSDSPConnect() must be
// eventually destroyed by NSDSPDisconnet(). This must be done
// even if the application is about to be terminated.
//
// To close the connection cleanly, NSDSPDisconnect() waits until
// all the data transfers originated by NSDSPFlush() are complete
// or until the timeout expires. Therefore, it may take
// long time to finish. If this is undesirable, call NSDSPSetTimeout()
// to shorthen the timeout period prior to calling NSDSPDisconnect().
// -------------------------------------------------------------------
NSDSP_EXPORT(NSDSP_CONN_HANDLE) NSDSPDisconnect(NSDSP_CONN_HANDLE Conn);

// -------------------------------------------------------------------
// NSDSPGetSerial - returns the serial number of the connected NSDSP
// 
// Conn      the connection handle returned by NSDSPConnect().
//
// Return    the pointer to a NULL-terminated ASCII string
//           containing the NSDSP serial number. This string
//           cannot be used after the connection is destroyed
//           by NSDSPDisconnect().
// -------------------------------------------------------------------
NSDSP_EXPORT(char*) NSDSPGetSerial(NSDSP_CONN_HANDLE Conn);

// -------------------------------------------------------------------
// NSDSPGetVersion - returns the version of the connected NSDSP
//                   hardware and firmware combination.
// 
// Conn      the connection handle returned by NSDSPConnect().
//
// Return    the version in BCD format. 0x100 corresponds to the
//           version 1.00.
//
// At the time this software was created, the latest version of
// NSDSP hardware is 1.03. It is not anticipated that the
// interface may change in future versions.
//
// Northern Software Inc follows the version update policies which
// require that the software does not work with unknown firmware.
// If you want to comply with this policy, you should check the
// version and refuse the operation if the version returned
// is above 1.03. 
// -------------------------------------------------------------------
NSDSP_EXPORT(unsigned int) NSDSPGetVersion(NSDSP_CONN_HANDLE Conn);

// -------------------------------------------------------------------
// NSDSPGetBaudRate - returns the baud rate for the connection
// 
// Conn      the connection handle returned by NSDSPConnect().
//
// Return    the actual baud rate.
//
// This function returns the baud rate and only makes sense when
// NSDSP is in UART mode.
// -------------------------------------------------------------------
NSDSP_EXPORT(unsigned int) NSDSPGetBaudRate(NSDSP_CONN_HANDLE Conn);

// -------------------------------------------------------------------
// NSDSPGetFlowControl - returns the state of the flow control
// 
// Conn      the connection handle returned by NSDSPConnect().
//
// Return    1 if flow control is enabled, 0 otherwise
//
// This function only makes sense when NSDSP is in UART mode.
// -------------------------------------------------------------------
NSDSP_EXPORT(int) NSDSPGetFlowControl(NSDSP_CONN_HANDLE Conn);

// -------------------------------------------------------------------
// NSDSPGetCTS - returns the logic level on the NSDSP's CTS pin
// 
// Conn      the connection handle returned by NSDSPConnect().
//
// Return    1 if CTS is high, 0 if CTS is low, -1 if error
//
// This function can be called in any mode and is not synchronized
// with the data or command stream.
//
// It typically takes 1 to 2 ms to receive the CTS state, therefore
// the state of the CTS pin will not be up to date.
// -------------------------------------------------------------------
NSDSP_EXPORT(int) NSDSPGetCTS(NSDSP_CONN_HANDLE Conn);

// -------------------------------------------------------------------
// NSDSPGetRX - returns the logic level on the NSDSP's RX pin
// 
// Conn      the connection handle returned by NSDSPConnect().
//
// Return    1 if RX is high, 0 if RX is low, -1 if error
//
// This function can be called in any mode except UART and is not
// synchronized with the command stream.
//
// It typically takes 1 to 2 ms to receive the RX state, therefore
// the state of the RX pin will not be up to date.
// -------------------------------------------------------------------
NSDSP_EXPORT(int) NSDSPGetRX(NSDSP_CONN_HANDLE Conn);

// -------------------------------------------------------------------
// NSDSPSetMode - changes NSDSP mode of operation
// 
// Conn      the connection handle returned by NSDSPConnect().
//
// Mode      one of the NSDSP_MODE_* constants (listed above). If
//           anything else is used, the consequences are
//           unpredictable and may damage NSDSP.
//
// Return    1 if successful, 0 otherwise
//
// The function abandons the current mode of operation and enters
// a new one, which can be either UART, programming, or idle mode.
// The function sets the baud rate and specifies whether the CTS/RTS
// flow control is going to be used. When you enter an UART or
// programming mode the software selects an appropriate Timeout
// based on the baud rate. This is done automatically. The timeout is
// used in NSDSPFlush() and NSDSPWaitForData() operations. You can
// overwrite the timeout to any other value with NSDSPSetTimeout().
//
// NSDSP_MODE_IDLE specifies idle mode. You must return NSDSP into
// idle mode after use.
// -------------------------------------------------------------------
NSDSP_EXPORT(int) NSDSPSetMode(NSDSP_CONN_HANDLE Conn, unsigned char *Mode);

// -------------------------------------------------------------------
// NSDSPSetTimeout - specifies new timeout
// 
// Conn      the connection handle returned by NSDSPConnect().
//
// Timeout   new timeout (in ms). This value is not restricted by
//           NSDSP_MIN_TIMEOUT or NSDSP_MAX_TIMEOUT.
//
// Return    none
//
// The timeout is used by NSDSPFlush() and NSDSPWaitForData()
// operations. Note that NSDSPFlush() may also be called by 
// NSDSPWrite(), NSDSPWriteCommand(), NSDSPWriteSPI() or NSDSPDelay().
// The default timeout based on baud rate and is
// set every time you call NSDSPSetMode(). The default value
// is a reasonable guess to distingush normal operation from
// malfunction (for example if microcontroller holds the CTS
// line for too long) and should work in most cases.
// -------------------------------------------------------------------
NSDSP_EXPORT(void) NSDSPSetTimeout(NSDSP_CONN_HANDLE Conn, int Timeout);

// -------------------------------------------------------------------
// NSDSPWrite - writes data to the outgoing buffer
// 
// Conn      the connection handle returned by NSDSPConnect()
//
// Src       pointer to the data to be written
//
// Size      size of the data in bytes
//
// Return    1 if successful, 0 otherwise
//
// NSDSP is optimized for programming, therefore data transfers are
// done in big chunks. One chunk contains 1008 bytes and can be
// transferred every 16 ms (62.5 chunks per second). Even if you
// only want to send one byte, the whole chunk must be transmitted
// and it still takes 16 ms to transmit.
//
// Therefore, to get better data rate, the data are accumulated
// in the buffer and then sent all together. NSDSPWrite() puts
// the data into the buffer. However, if the buffer becomes full
// NSDSPWrite() initiates buffer transfer to NSDSP by calling
// NSDSPFlush(). Normally, this is a non-blocking operation -
// NSDSPWrite() returns without waiting for the transfer to be
// complete. However, if a previous transfer is still pending,
// NSDSPWrite() will block and will not return until the previous
// transfer completes or the timeout expires.
//
// After you call NSDSPWrite() one or more times and you have no
// further data to send immediately you must call NSDSPFlush()
// to make sure that the data pending in the buffer gets sent
// to NSDSP.
// -------------------------------------------------------------------
NSDSP_EXPORT(int) NSDSPWrite(NSDSP_CONN_HANDLE Conn, char *Src, int Size);

// -------------------------------------------------------------------
// NSDSPWriteCommand - writes command to the outgoing buffer
// 
// Conn      the connection handle returned by NSDSPConnect()
//
// Cmd       the command to be transferred
//
// Src       pointer to the command parameter
//
// Size      size of the command parameter
//
// Return    1 if successful, 0 otherwise
//
// This functions places a command along with its parameter into the
// output buffer. It does the same as NSDSPWrite(), but makes
// sure the commands and parameters are not straddled between
// different buffers, which is absolutely necessary for successful
// command execution.
// -------------------------------------------------------------------
NSDSP_EXPORT(int) NSDSPWriteCommand(NSDSP_CONN_HANDLE Conn, char Cmd, char *Src, int Size);

// -------------------------------------------------------------------
// NSDSPWriteSPI - writes SPI write commands to the outgoing buffer
// 
// Conn      the connection handle returned by NSDSPConnect().
//
// Src       pointer to the data to be transmitted with SPI
//
// Size      size of the data
//
// Return    1 if successful, 0 otherwise
//
// This functions places one or more NSDSP_CMD_SPI_WRITE_* commands
// into the output buffer. It selects the fastest commands to
// transmit the supplied data with SPI through ICSPCLK and ICSPCLK pins
// -------------------------------------------------------------------
NSDSP_EXPORT(int) NSDSPWriteSPI(NSDSP_CONN_HANDLE Conn, char *Src, int Size);

// -------------------------------------------------------------------
// NSDSPDelay - writes delay commands to the outgoing buffer
// 
// Conn      the connection handle returned by NSDSPConnect().
//
// Delay     duration - the number of time slots between 1 and 16777217
//
// Return    1 if successful, 0 otherwise
//
// This functions can only be used in programming mode. It produces
// a delay of specified duration. During the delay, the commands are
// not executed by NSDSP. NSDSP queues the received commands and
// executes them after the delay. Note that if you keep transmitting
// commands during a long delay, your trasactions may time out.
// -------------------------------------------------------------------
NSDSP_EXPORT(int) NSDSPDelay(NSDSP_CONN_HANDLE Conn, int Delay);

// -------------------------------------------------------------------
// NSDSPFlush - sends pendng data
// 
// Conn      the connection handle returned by NSDSPConnect().
//
// Return    1 if successful, 0 otherwise
//
// NSDSPFlush() sends the data accumulated by the previous calls
// to NSDSPWrite() to NSDSP. If there is no data pending, NSDSPFlush()
// returns immediately. Otherwise, it initiates data transfer and
// returns. However, if the previous data transfer is not complete,
// NSDSPFlush() blocks until the data transfer is complete or
// until timeout expires. The timeout is set by NSDSPSetMode() and may
// be overwritten by NSDSPSetTimeout().
//
// When NSDSPFlush() returns, the data may not yet been transferred
// to NSDSP, and if an error occurs, the transfer may never
// complete. Such lingering transfers will have to be waited upon
// during NSDSPDisconnect().
// -------------------------------------------------------------------
NSDSP_EXPORT(int) NSDSPFlush(NSDSP_CONN_HANDLE Conn);

// -------------------------------------------------------------------
// NSDSPWaitForCompletion - wait until all output operations are
//                          completed
// 
// Conn      the connection handle returned by NSDSPConnect().
//
// Return    1 if successful, 0 otherwise
//
// NSDSPWaitForCompletion() only works in UART mode.
// NSDSPWaitForCompletion() waits until all data is transferred
// to NSDSP. Once the data transfer is complete, NSDSP's buffers
// can still hold up to 381 bytes. Therefore, to ensure completion
// NSDSPWaitForCompletion() adds more wait time.
//
// NSDSP prior to version 1.01 didn't have any means to detect
// if the internal buffer has been emptied. Therefore, the
// wait for fixed period of 5000 baud, which should be enough
// to send out all the internal buffers. However, if flow control
// is enabled and the microcontroller holds the CTS line, the wait
// time may not be sufficient. Therefore the successful return
// from NSDSPWaitForCompletion() is not a guarantee that the
// microcontroller received all the data.
//
// For NSDSP version 1.01 and later, NSDSPWaitForCompletion()
// polls NSDSP state in a loop to detect when the internal
// buffers become empty. This allows to avoid unnessecary wait.
// The polling is subject to the timeout set by NSDSPSetMode() or
// NSDSPSetTimeout() and if the timeout expires before the poll
// is over, NSDSPWaitForCompletion() exits and returns 0.
// -------------------------------------------------------------------
NSDSP_EXPORT(int) NSDSPWaitForCompletion(NSDSP_CONN_HANDLE Conn);

// -------------------------------------------------------------------
// NSDSPAvailableData - returns the amout of data received
// 
// Conn      the connection handle returned by NSDSPConnect().
//
// Return    the size of available data in bytes
//
// The data received through NSDSP is collected
// automatically by a separate thread. This thread places data
// into a big buffer. The data can be retreved with NSDSPRead().
// The NSDSPAvailableData() function can be used to find out
// how much data is currently stored in the input buffer.
// -------------------------------------------------------------------
NSDSP_EXPORT(int) NSDSPAvailableData(NSDSP_CONN_HANDLE Conn);

// -------------------------------------------------------------------
// NSDSPWaitForData - waits until the requested amount of data becomes
//                    available or timeout expires
// 
// Conn      the connection handle returned by NSDSPConnect().
//
// Size      the desired amount of data in bytes
//
// Return    1 if successful, 0 otherwise
//
// This functions checks availability of data received from NSDSP, and
// if the amount of currently available data is less than the Size,
// the function blocks and waits until the desired amount of data
// becomes available or until the timeout expires. The timeout is
// automatically set by NSDSPSetMode() and can be changed by
// NSDSPSetTimeout().
// -------------------------------------------------------------------
NSDSP_EXPORT(int) NSDSPWaitForData(NSDSP_CONN_HANDLE Conn, int Size);

// -------------------------------------------------------------------
// NSDSPWaitForDataForever - waits until the requested amount of data
//                           becomes available
// 
// Conn      the connection handle returned by NSDSPConnect().
//
// Size      the desired amount of data in bytes
//
// Return    1 if successful, 0 otherwise
//
// This functions checks availability of data received from NSDSP, and
// if the amount of currently available data is less than the Size,
// the function blocks and waits until the desired amount of data
// becomes available. This function never times out, but can return
// prematurely if there is any error, such as USB malfunction.
// -------------------------------------------------------------------
NSDSP_EXPORT(int) NSDSPWaitForDataForever(NSDSP_CONN_HANDLE Conn, int Size);

// -------------------------------------------------------------------
// NSDSPRead - retrieves data from the input buffer
// 
// Conn      the connection handle returned by NSDSPConnect().
//
// Dst       pointer to the buffer to accept data.
//
// Size      the desired amount of data in bytes
//
// Return    the size of the data actually retreived from the buffer
//
// This functions retrieves data from the input buffer. If there's
// no data available, the function returns 0. If there's less data
// than specified by Size, all the available data are retrieved, and
// the function returns the amount of data actually retrieved. If
// there's more data than Size, only Size bytes are retrieved, and
// the function returns Size.
//
// This function never blocks. If it is necessary to wait for data
// then NSDSPWaitForData() or NSDSPWaitForDataForever() may be used.
// Alternatively, you can poll NSDSPAvailableData().
// -------------------------------------------------------------------
NSDSP_EXPORT(int) NSDSPRead(NSDSP_CONN_HANDLE Conn, char *Dst, int Size);

#endif // defined NSDSP_H