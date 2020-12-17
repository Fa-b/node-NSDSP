# node-NSDSP

Interface with NSDSP devices from nodejs

## Usage

TODO

## Binding API Documentation
This is the API documentation for the binding and <b>not</b> for the wrapper.


### Enumerate
enumerates all connected NSDSP devices

```js
Enumerate(dev: Buffer): number
```

 - `dev`: the buffer to be filled with the enumeration information which can hold up to `MAX_NSDSP_DEVICES`.

 - `returns`: the number of NSDSP devices enumerated or -1 in case of an error.

`Enumerate()` can be used to get the list of NSDSP devices
currently plugged in. The Serial numbers of the NSDSP devices
returned by this function can be used in `Connect()` to
connect to a specified NSDSP device.

If there's only one NSDSP present, or if the Serial number of
the desired NSDSP device is already known, it is not necessary
to call `Enumerate()`

### Connect
establishes a connection with NSDSP devices

```js
Connect(serial?: string): Buffer
```

 - `serial`: the serial number of the NSDSP device. May be NULL. If NULL is specified and there is only one NSDSP device present, `Connect()` connects to this NSDSP device. If `NULL` is specified when multiple NSDSP devices are present, `Connect()` connects to the first NSDSP device on the enumeration list.

 - `returns`: the handle to the connection if successful, null otherwise. It can be used in subsequent functions as needed. After use, it must be destroyed with `Disconnect()`.

Only one connection can be established to any specific NSDSP device
at a time. If there are multiple NSDSP devices, you can establish
multiple independent connections to each of the devices. An attempt
to establish multiple connections to the same NSDSP produces
unpredictable results.

### Disconnect
destroys an NSDSP connection

```js
Disconnect(handle: Buffer): Buffer
```

 - `handle`: the connection handle returned by `Connect()`. After calling `Disconnect()`, the handle becomes invalid and can no longer be used.

 - `returns`: null if successful, the handle passed to this function otherwise.

All connections established with `Connect()` must be
eventually destroyed by `Disconnect()`. This must be done
even if the application is about to be terminated.

To close the connection cleanly, `Disconnect()` waits until
all the data transfers originated by `Flush()` are complete
or until the timeout expires. Therefore, it may take
long time to finish. If this is undesirable, call `SetTimeout()`
to shorthen the timeout period prior to calling `Disconnect()`.

### GetSerial
returns the serial number of the connected NSDSP

```js
GetSerial(handle: Buffer): Buffer|undefined
```

 - `handle`: the connection handle returned by `Connect()`.

 - `returns`: the buffer of a NULL-terminated ASCII string containing the NSDSP serial number. This string cannot be used after the connection is destroyed by `Disconnect()`.

### GetVersion
returns the version of the connected NSDSP hardware and firmware combination.

```js
GetVersion(handle: Buffer): number
```

 - `handle`: the connection handle returned by `Connect()`.

 - `returns`: the version in BCD format. 0x100 corresponds to the version 1.00.

At the time this software was created, the latest version of NSDSP hardware is 1.03. It is not anticipated that the
interface may change in future versions.

Northern Software Inc follows the version update policies which
require that the software does not work with unknown firmware.
If you want to comply with this policy, you should check the
version and refuse the operation if the version returned
is above 1.03. 

### GetBaudRate
returns the baud rate for the connection

```js
GetBaudRate(handle: Buffer): number
```

 - `handle`: the connection handle returned by `Connect()`.

 - `returns`: the actual baud rate.

This function returns the baud rate and only makes sense when NSDSP is in UART mode.

### GetFlowControl
returns the state of the flow control

```js
GetFlowControl(handle: Buffer): number
```

 - `handle`: the connection handle returned by `Connect()`.

 - `returns`: 1 if flow control is enabled, 0 otherwise.

This function only makes sense when NSDSP is in UART mode.

### GetCTS
returns the logic level on the NSDSP's CTS pin

```js
GetCTS(handle: Buffer): number
```

 - `handle`: the connection handle returned by `Connect()`.

 - `returns`: 1 if CTS is high, 0 if CTS is low, -1 if error

This function can be called in any mode and is not synchronized
with the data or command stream.

It typically takes 1 to 2 ms to receive the CTS state, therefore
the state of the CTS pin will not be up to date.

### GetRX
returns the logic level on the NSDSP's RX pin

```js
GetRX(handle: Buffer): number
```

 - `handle`: the connection handle returned by `Connect()`.

 - `returns`: 1 if RX is high, 0 if RX is low, -1 if error.

This function can be called in any mode except UART and is not
synchronized with the command stream.

It typically takes 1 to 2 ms to receive the RX state, therefore
the state of the RX pin will not be up to date.

### SetMode
changes NSDSP mode of operation

```js
SetMode(handle: Buffer, mode: string): number
```

 - `handle`: the connection handle returned by `Connect()`.

 - `mode`: one of the `NSDSP_MODE_*` constants (TODO document). If anything else is used, the consequences are unpredictable and may damage NSDSP.

 - `returns`: 1 if successful, 0 otherwise.

The function abandons the current mode of operation and enters
a new one, which can be either UART, programming, or idle mode.
The function sets the baud rate and specifies whether the CTS/RTS
flow control is going to be used. When you enter an UART or
programming mode the software selects an appropriate Timeout
based on the baud rate. This is done automatically. The timeout is
used in `Flush()` and `WaitForData()` operations. You can
overwrite the timeout to any other value with `SetTimeout()`.

`NSDSP_MODE_IDLE` specifies idle mode. You must return NSDSP into
idle mode after use.

### SetTimeout
specifies new timeout

```js
SetTimeout(handle: Buffer, timeout: number): number
```

 - `handle`: the connection handle returned by `Connect()`.

 - `timeout`: new timeout (in ms). This value is not restricted by `NSDSP_MIN_TIMEOUT` or `NSDSP_MAX_TIMEOUT`.

 - `returns`: the timeout set if successful, undefined otherwise.

The timeout is used by `Flush()` and `WaitForData()`
operations. Note that `Flush()` may also be called by 
`Write()`, `WriteCommand()`, `WriteSPI()` or `Delay()`.
The default timeout based on baud rate and is
set every time you call `SetMode()`. The default value
is a reasonable guess to distingush normal operation from
malfunction (for example if microcontroller holds the CTS
line for too long) and should work in most cases.

### Write
writes data to the outgoing buffer

```js
Write(handle: Buffer, src: Buffer, size: Number): number
```

 - `handle`: the connection handle returned by `Connect()`.

 - `src`: buffer of the data to be written.

 - `size`: size of the data in bytes.

 - `returns`: 1 if successful, 0 otherwise.

NSDSP is optimized for programming, therefore data transfers are
done in big chunks. One chunk contains 1008 bytes and can be
transferred every 16 ms (62.5 chunks per second). Even if you
only want to send one byte, the whole chunk must be transmitted
and it still takes 16 ms to transmit.

Therefore, to get better data rate, the data are accumulated
in the buffer and then sent all together. `Write()` puts
the data into the buffer. However, if the buffer becomes full
`Write()` initiates buffer transfer to NSDSP by calling
`Flush()`. Normally, this is a non-blocking operation -
`Write()` returns without waiting for the transfer to be
complete. However, if a previous transfer is still pending,
`Write()` will block and will not return until the previous
transfer completes or the timeout expires.

After you call `Write()` one or more times and you have no
further data to send immediately you must call `Flush()`
to make sure that the data pending in the buffer gets sent
to NSDSP.

### WriteCommand
writes command to the outgoing buffer

```js
WriteCommand(handle: Buffer, cmd: string, src: Buffer, size: number): number
```

 - `handle`: the connection handle returned by `Connect()`.

 - `cmd`: the command to be transferred.

 - `src`: buffer of the command parameter.

 - `size`: size of the command parameter.

 - `returns`: 1 if successful, 0 otherwise.

This functions places a command along with its parameter into the
output buffer. It does the same as `Write()`, but makes
sure the commands and parameters are not straddled between
different buffers, which is absolutely necessary for successful
command execution.

### WriteSPI
writes SPI write commands to the outgoing buffer

```js
WriteSPI(handle: Buffer, src: Buffer, size: number): number
```

 - `handle`: the connection handle returned by `Connect()`.

 - `src`: buffer of the data to be transmitted with SPI.

 - `size`: size of the data.

 - `returns`: 1 if successful, 0 otherwise.

This functions places one or more `NSDSP_CMD_SPI_WRITE_*` commands
into the output buffer. It selects the fastest commands to
transmit the supplied data with SPI through ICSPCLK and ICSPCLK pins

### Delay
writes delay commands to the outgoing buffer

```js
Delay(handle: Buffer, delay: number): number
```

 - `handle`: the connection handle returned by `Connect()`.

 - `delay`: duration - the number of time slots between 1 and `16777217`

 - `returns`: 1 if successful, 0 otherwise

This functions can only be used in programming mode. It produces
a delay of specified duration. During the delay, the commands are
not executed by NSDSP. NSDSP queues the received commands and
executes them after the delay. Note that if you keep transmitting
commands during a long delay, your trasactions may time out.

### Flush
sends pendng data

```js
Flush(handle: Buffer): number
```

 - `handle`: the connection handle returned by `Connect()`.

 - `returns`: 1 if successful, 0 otherwise.


`Flush()` sends the data accumulated by the previous calls
to `Write()` to NSDSP. If there is no data pending, `Flush()`
returns immediately. Otherwise, it initiates data transfer and
returns. However, if the previous data transfer is not complete,

`Flush()` blocks until the data transfer is complete or
until timeout expires. The timeout is set by `SetMode()` and may
be overwritten by `SetTimeout()`.

When `Flush()` returns, the data may not yet been transferred
to NSDSP, and if an error occurs, the transfer may never
complete. Such lingering transfers will have to be waited upon
during `Disconnect()`.

### WaitForCompletion
wait until all output operations are completed

```js
WaitForCompletion(handle: Buffer): number
```

 - `handle`: the connection handle returned by `Connect()`.

 - `returns`: 1 if successful, 0 otherwise.

`WaitForCompletion()` only works in UART mode.
`WaitForCompletion()` waits until all data is transferred
to NSDSP. Once the data transfer is complete, NSDSP's buffers
can still hold up to 381 bytes. Therefore, to ensure completion
`WaitForCompletion()` adds more wait time.

NSDSP prior to version 1.01 didn't have any means to detect
if the internal buffer has been emptied. Therefore, the
wait for fixed period of 5000 baud, which should be enough
to send out all the internal buffers. However, if flow control
is enabled and the microcontroller holds the CTS line, the wait
time may not be sufficient. Therefore the successful return
from `WaitForCompletion()` is not a guarantee that the
microcontroller received all the data.

For NSDSP version 1.01 and later, `WaitForCompletion()`
polls NSDSP state in a loop to detect when the internal
buffers become empty. This allows to avoid unnessecary wait.
The polling is subject to the timeout set by `SetMode()` or
`SetTimeout()` and if the timeout expires before the poll
is over, `WaitForCompletion()` exits and returns 0.

### AvailableData
returns the amout of data received

```js
AvailableData(handle: Buffer): number
```

 - `handle`: the connection handle returned by `Connect()`.

 - `returns`: the size of available data in bytes.

The data received through NSDSP is collected
automatically by a separate thread. This thread places data
into a big buffer. The data can be retrieved with `Read()`.
The `AvailableData()` function can be used to find out
how much data is currently stored in the input buffer.

### WaitForData
waits until the requested amount of data becomes available or timeout expires

```js
WaitForData(handle: Buffer, size: number): number
```

 - `handle`: the connection handle returned by `Connect()`.

 - `size`: the desired amount of data in bytes.

 - `returns`: 1 if successful, 0 otherwise.

This functions checks availability of data received from NSDSP, and
if the amount of currently available data is less than the Size,
the function blocks and waits until the desired amount of data
becomes available or until the timeout expires. The timeout is
automatically set by `SetMode()` and can be changed by
`SetTimeout()`.

### WaitForDataForever
waits until the requested amount of data becomes available

```js
WaitForDataForever(handle: Buffer, size: number): number
```

 - `handle`: the connection handle returned by `Connect()`.

 - `size`: the desired amount of data in bytes.

 - `returns`: 1 if successful, 0 otherwise.

This functions checks availability of data received from NSDSP, and
if the amount of currently available data is less than the Size,
the function blocks and waits until the desired amount of data
becomes available. This function never times out, but can return
prematurely if there is any error, such as USB malfunction.

### Read
retrieves data from the input buffer

```js
Read(handle: Buffer, dst: Buffer, size: number): number
```

 - `handle`: the connection handle returned by `Connect()`.

 - `dst`: buffer to accept data.

 - `size`: the desired amount of data in bytes.

 - `returns`: the size of the data actually retreived from the buffer.

This functions retrieves data from the input buffer. If there's
no data available, the function returns 0. If there's less data
than specified by Size, all the available data are retrieved, and
the function returns the amount of data actually retrieved. If
there's more data than Size, only Size bytes are retrieved, and
the function returns Size.

This function never blocks. If it is necessary to wait for data
then `WaitForData()` or `WaitForDataForever()` may be used.
Alternatively, you can poll `AvailableData()`.

## License

MIT
