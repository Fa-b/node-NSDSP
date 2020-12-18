/***************************************************************************
 * (C) 2017-2019 Northern Software Inc                                          *
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
#ifdef UNICODE
  #undef UNICODE
#endif
#include <windows.h>
#include "nsdsp.h"

#define DIGCF_DEFAULT          0x00000001  // only valid with DIGCF_DEVICEINTERFACE
#define DIGCF_PRESENT          0x00000002
#define DIGCF_ALLCLASSES       0x00000004
#define DIGCF_PROFILE          0x00000008
#define DIGCF_DEVICEINTERFACE  0x00000010

#define RBUF_MASK (NSDSP_INPUT_BUFFER_SIZE-1)

typedef struct {
  DWORD cbSize;
  GUID ClassGuid;
  DWORD DevInst;
  ULONG *Reserved;
} DEVINFO_DATA;

typedef struct {
  DWORD cbSize;
  GUID InterfaceClassGuid;
  DWORD Flags;
  ULONG *Reserved;
} DEVICE_INTERFACE_DATA;

typedef struct {
  DWORD cbSize;
  char DevicePath[2044];
} DEVICE_INTERFACE_DETAIL_DATA;

typedef struct {
  ULONG  Size;
  USHORT VendorID;
  USHORT ProductID;
  USHORT VersionNumber;
} HID_ATTRIBUTES;

typedef struct NSDSP_CONN_DATA {
  // platform-independent part
  unsigned int Version;
  char Serial[MAX_NSDSP_SERIAL];
  unsigned int Baud;
  unsigned int FlowControl;
  unsigned int Timeout;
  unsigned int SessionId;
  unsigned int QCnt;
  unsigned int QPtr;
  char* QBufPtr;
  char QBuf1[NSDSP_REPORT_SIZE+4];
  char QBuf2[NSDSP_REPORT_SIZE+4];
  
  unsigned int RRead;
  unsigned int RWrite;
  unsigned int RequestedData;  
  int Killing;
  int ExpectBarrier;
  char RBuf[NSDSP_INPUT_BUFFER_SIZE];
  
  // platform specific part
  HANDLE hDev, hThread;
  OVERLAPPED OVRead, OVWrite;
  HANDLE EvRead, EvWrite, EvProcess, Mutex;
  DWORD ThreadId;
} NSDSP_CONN_DATA;
 
int library_initialized = 0;
int library_status = 0;

HANDLE (WINAPI *fn_SetupDiGetClassDevs)(GUID *ClassGuid, char* Enumerator,
  HWND hwndParent, DWORD Flags);
BOOL (WINAPI *fn_SetupDiEnumDeviceInterfaces)(HANDLE *DeviceInfoSet,
  DEVINFO_DATA *DeviceInfoData, GUID *InterfaceClassGuid, 
  DWORD MemberIndex, DEVICE_INTERFACE_DATA *DeviceInterfaceData);
BOOL (WINAPI *fn_SetupDiGetDeviceInterfaceDetail)(HANDLE DeviceInfoSet,
    DEVICE_INTERFACE_DATA *DeviceInterfaceData, 
    DEVICE_INTERFACE_DETAIL_DATA *DeviceInterfaceDetailData,
    DWORD DeviceInterfaceDetailDataSize, DWORD *RequiredSize,
    DEVINFO_DATA *DeviceInfoData);
BOOL (WINAPI *fn_SetupDiDestroyDeviceInfoList)(DEVINFO_DATA *DeviceInfoData);


BOOL (WINAPI *fn_HIDFlushQueue)(HANDLE HidDeviceObject);
BOOL (WINAPI *fn_HIDGetAttributes)(HANDLE HidDeviceObject, HID_ATTRIBUTES *Attributes);
BOOL (WINAPI *fn_HIDGetFeature)(HANDLE HidDeviceObject, char *ReportBuffer, ULONG ReportBufferLength);
void (WINAPI *fn_HIDGetHIDGuid)(GUID* HidGuid);
BOOL (WINAPI *fn_HIDGetSerialNumberString)(HANDLE HidDeviceObject, WCHAR *Buffer, ULONG BufferLength);
BOOL (WINAPI *fn_HIDSetFeature)(HANDLE HidDeviceObject, char *ReportBuffer, ULONG ReportBufferLength);
BOOL (WINAPI *fn_HIDSetNumInputBuffers)(HANDLE HidDeviceObject, ULONG NumberBuffers);

// This function loads SetupAPI.DLL and HID.DLL functions manually to avoid dependency
// on the corresponding non-standard headers
int InitLib() {
  HINSTANCE SetupAPILib, HIDLib;
  if (library_initialized) return library_status;
  library_initialized = 1;
  
  SetupAPILib = LoadLibrary("SETUPAPI.DLL");
  if (!SetupAPILib) return 0;
  
  fn_SetupDiGetClassDevs = (void*) GetProcAddress(SetupAPILib,"SetupDiGetClassDevsA");
  if (!fn_SetupDiGetClassDevs) return 0;
  fn_SetupDiEnumDeviceInterfaces = (void*) GetProcAddress(SetupAPILib,"SetupDiEnumDeviceInterfaces");
  if (!fn_SetupDiEnumDeviceInterfaces) return 0;
  fn_SetupDiGetDeviceInterfaceDetail = (void*) GetProcAddress(SetupAPILib,"SetupDiGetDeviceInterfaceDetailA");
  if (!fn_SetupDiGetDeviceInterfaceDetail) return 0;
  fn_SetupDiDestroyDeviceInfoList = (void*) GetProcAddress(SetupAPILib,"SetupDiDestroyDeviceInfoList");
  if (!fn_SetupDiDestroyDeviceInfoList) return 0;
  
  HIDLib = LoadLibrary("HID.DLL");
  if (!HIDLib) return 0;
  
  fn_HIDFlushQueue = (void*) GetProcAddress(HIDLib,"HidD_FlushQueue");
  if (!fn_HIDFlushQueue) return 0;
  fn_HIDGetAttributes = (void*) GetProcAddress(HIDLib,"HidD_GetAttributes");
  if (!fn_HIDGetAttributes) return 0;
  fn_HIDGetFeature = (void*) GetProcAddress(HIDLib,"HidD_GetFeature");
  if (!fn_HIDGetFeature) return 0;
  fn_HIDGetHIDGuid = (void*) GetProcAddress(HIDLib,"HidD_GetHidGuid");
  if (!fn_HIDGetHIDGuid) return 0;
  fn_HIDGetSerialNumberString = (void*) GetProcAddress(HIDLib,"HidD_GetSerialNumberString");
  if (!fn_HIDGetSerialNumberString) return 0;
  fn_HIDSetFeature = (void*) GetProcAddress(HIDLib,"HidD_SetFeature");
  if (!fn_HIDSetFeature) return 0;
  fn_HIDSetNumInputBuffers = (void*) GetProcAddress(HIDLib,"HidD_SetNumInputBuffers");
  if (!fn_HIDSetNumInputBuffers) return 0;
  
  library_status = 1;
  return 1;
}
 
NSDSP_EXPORT(int) NSDSPEnumerate(NSDSP_ENUM_DATA *Dev) {
  GUID HIDGUID;
  HANDLE hDevInfo, hFile;
  DEVICE_INTERFACE_DATA DevInfo;
  DEVICE_INTERFACE_DETAIL_DATA DevDetail;
  HID_ATTRIBUTES Attr;
  NSDSP_ENUM_STRUCT *PDev;
  WCHAR BufW[2*(MAX_NSDSP_SERIAL+1)];
  unsigned int DevIndex;
  WCHAR *UnicodeSerial; 
  char *ASCIISerial;
  
  Dev->NDev = 0;
  
  if (!InitLib()) return -1;
    
  fn_HIDGetHIDGuid(&HIDGUID);
  
  hDevInfo = fn_SetupDiGetClassDevs(&HIDGUID, NULL, 0, DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);
  if (!hDevInfo) return -1;
  
  DevIndex = 0;
  while (1) {
    DevInfo.cbSize = sizeof(DEVICE_INTERFACE_DATA);
    if (!fn_SetupDiEnumDeviceInterfaces(hDevInfo, NULL, &HIDGUID, DevIndex, &DevInfo)) break;
    DevIndex++;
    
    // cbSize must be set to 5 for 32-bit version or 8 for 64-bit version
    DevDetail.cbSize = sizeof(void*) == 8?8:5;
    if (!fn_SetupDiGetDeviceInterfaceDetail(hDevInfo, &DevInfo, &DevDetail, 
      sizeof(DEVICE_INTERFACE_DETAIL_DATA)-sizeof(DWORD), NULL, NULL)) continue;
      
    if (strlen(DevDetail.DevicePath) > MAX_NSDSP_PATH) continue;
      
    hFile = CreateFile(DevDetail.DevicePath, 0, FILE_SHARE_READ | FILE_SHARE_WRITE,
      NULL,OPEN_EXISTING, 0, 0);
    if (hFile == INVALID_HANDLE_VALUE) continue;
    
    Attr.Size = sizeof(HID_ATTRIBUTES);
    if ((Dev->NDev < MAX_NSDSP_DEVICES)&&fn_HIDGetAttributes(hFile, &Attr)) {
      if (fn_HIDGetSerialNumberString(hFile,BufW,2*(MAX_NSDSP_SERIAL+1))&&(memcmp(BufW,L"NSDSP",10) == 0)) {
        PDev = Dev->Dev + Dev->NDev++;
        PDev->Version = Attr.VersionNumber;
        PDev->VID = Attr.VendorID;
        PDev->PID = Attr.ProductID;
        ASCIISerial = PDev->Serial;
        UnicodeSerial = BufW;
        while (*ASCIISerial++ = *UnicodeSerial++);
        strcpy(PDev->Path,DevDetail.DevicePath);
      }
    }
    CloseHandle(hFile);
  }
  
  return Dev->NDev;
}

void ProcessIncoming(NSDSP_CONN_HANDLE Conn, unsigned char *B) {
  unsigned char i, L;

  if (Conn->ExpectBarrier) {
    if ((*B & 0x80) == 0) return;
    Conn->ExpectBarrier = 0;
  }
  
  L = *B & 0x3f;
  B += 0x3f;
  L = 0x3f - L;
  if (L == 0) return;
 
  for (i = 0; i < L; i++) {
    if (NSDSPAvailableData(Conn) > RBUF_MASK) break; // buffer full
    Conn->RBuf[Conn->RWrite++ & RBUF_MASK] = *B--;
  }
    
  WaitForSingleObject(Conn->Mutex,-1);
  if (NSDSPAvailableData(Conn) >= Conn->RequestedData) SetEvent(Conn->EvProcess);
  ReleaseMutex(Conn->Mutex);
}

DWORD WINAPI ReaderThreadProc(NSDSP_CONN_HANDLE Conn) {
  unsigned char RBuf[NSDSP_INPUT_SIZE+1];
  DWORD dwR;
  
  fn_HIDFlushQueue(Conn->hDev);
  
  while (!Conn->Killing) {
    memset(&Conn->OVRead,0,sizeof(OVERLAPPED));
    Conn->OVRead.hEvent = Conn->EvRead;
    dwR = 0;
    *RBuf = 0;
    if (ReadFile(Conn->hDev,RBuf,NSDSP_INPUT_SIZE+1,&dwR,&Conn->OVRead)) {
      if (Conn->Killing) break;
      ProcessIncoming(Conn,RBuf+1);
    } else {
      if (GetLastError() != ERROR_IO_PENDING) break;
      if (WaitForSingleObject(Conn->EvRead,-1) != WAIT_OBJECT_0) break;
      if (Conn->Killing) break;
      if (!GetOverlappedResult(Conn->hDev,&Conn->OVRead,&dwR,0)) break;
      if (dwR != (NSDSP_INPUT_SIZE+1)) break;
      ProcessIncoming(Conn,RBuf+1);
    }
  }
  ExitThread(0);
}

void SetRequestedData(NSDSP_CONN_HANDLE Conn, unsigned int RD) {
  if (RD == Conn->RequestedData) return;
  WaitForSingleObject(Conn->Mutex,-1);
  Conn->RequestedData = RD;
  if (NSDSPAvailableData(Conn) >= RD) SetEvent(Conn->EvProcess);
    else ResetEvent(Conn->EvProcess);
  ReleaseMutex(Conn->Mutex);
}

NSDSP_EXPORT(NSDSP_CONN_HANDLE) NSDSPConnect(char *Serial) {
  NSDSP_ENUM_DATA Dev;
  NSDSP_CONN_HANDLE Conn;
  HANDLE hDev;
  int i, Idx;
  
  if (NSDSPEnumerate(&Dev) <= 0) return NULL;
  
  Idx = 0;
  if (Serial) {
    Idx = -1;
    for (i = 0; i < Dev.NDev; i++) {
      if (strcmp(Dev.Dev[i].Serial,Serial) == 0) {
        Idx = i;
        break;
      }
    }
  }
    
  if (Idx < 0) return NULL;
  
  hDev = CreateFile(Dev.Dev[Idx].Path, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE,
    NULL, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, 0);
  
  if (hDev == INVALID_HANDLE_VALUE) return NULL;

  Conn = GlobalAlloc(GMEM_FIXED,sizeof(NSDSP_CONN_DATA));
  if (!Conn) {
    CloseHandle(hDev);
    return NULL;
  }
  
  memset(Conn,0,sizeof(NSDSP_CONN_DATA));
  Conn->hDev = hDev;
  
  strcpy(Conn->Serial,Dev.Dev[Idx].Serial);
  Conn->Version = Dev.Dev[Idx].Version;
  Conn->EvRead = CreateEvent(NULL,TRUE,TRUE,NULL); // signaled when not busy reading
  Conn->EvWrite = CreateEvent(NULL,TRUE,TRUE,NULL); // signaled when not busy writing
  Conn->EvProcess = CreateEvent(NULL,TRUE,FALSE,NULL); // signaled when AvailableData >= RequestedData
  Conn->Mutex = CreateMutex(NULL,FALSE,NULL);
  Conn->QCnt = 1;
  Conn->QPtr = 2;
  Conn->QBufPtr = Conn->QBuf1;
  Conn->RequestedData = 1;
  
  if (!fn_HIDSetNumInputBuffers(hDev,512)) fn_HIDSetNumInputBuffers(hDev,200);
  fn_HIDFlushQueue(hDev);
  Conn->hThread = CreateThread(NULL,0,ReaderThreadProc,Conn,0,&Conn->ThreadId);
  
  return Conn;
}

NSDSP_EXPORT(NSDSP_CONN_HANDLE) NSDSPDisconnect(NSDSP_CONN_HANDLE Conn) {
  Conn->Killing = 1;
  WaitForSingleObject(Conn->EvWrite,Conn->Timeout);
  while (1) {
    SetEvent(Conn->EvRead);
    if (WaitForSingleObject(Conn->hThread,10) != WAIT_TIMEOUT) break;
  }
  CloseHandle(Conn->hThread);
  CloseHandle(Conn->EvRead);
  CloseHandle(Conn->EvWrite);
  CloseHandle(Conn->EvProcess);
  CloseHandle(Conn->hDev);
  Sleep(100);
  return GlobalFree(Conn);
}

NSDSP_EXPORT(char*) NSDSPGetSerial(NSDSP_CONN_HANDLE Conn) {
  return Conn->Serial;
}

NSDSP_EXPORT(unsigned int) NSDSPGetVersion(NSDSP_CONN_HANDLE Conn) {
  return Conn->Version;
}

NSDSP_EXPORT(unsigned int) NSDSPGetBaudRate(NSDSP_CONN_HANDLE Conn) {
  return Conn->Baud;
}

NSDSP_EXPORT(int) NSDSPGetFlowControl(NSDSP_CONN_HANDLE Conn) {
  return Conn->FlowControl;
}

NSDSP_EXPORT(int) NSDSPGetCTS(NSDSP_CONN_HANDLE Conn) {
  unsigned char Feature[NSDSP_FEATURE_SIZE+1];
  
  memset(Feature,0,NSDSP_FEATURE_SIZE+1);
  if (!fn_HIDGetFeature(Conn->hDev,Feature,NSDSP_FEATURE_SIZE+1)) return -1;
  
  return !!(Feature[8] & 0x10);
}

NSDSP_EXPORT(int) NSDSPGetRX(NSDSP_CONN_HANDLE Conn) {
  unsigned char Feature[NSDSP_FEATURE_SIZE+1];
  
  memset(Feature,0,NSDSP_FEATURE_SIZE+1);
  if (!fn_HIDGetFeature(Conn->hDev,Feature,NSDSP_FEATURE_SIZE+1)) return -1;
  
  return !!(Feature[9] & 0x20);
}

NSDSP_EXPORT(int) NSDSPSetMode(NSDSP_CONN_HANDLE Conn, unsigned char *Mode) {
  unsigned char Feature[NSDSP_FEATURE_SIZE+1];
  unsigned int Div;
  
  Div = Mode[1] + Mode[2]*256 + 1;
  Conn->Baud = 12000000/Div;
  Conn->Timeout = Div*42;
  if (Conn->Timeout < NSDSP_MIN_TIMEOUT) Conn->Timeout = NSDSP_MIN_TIMEOUT;
  if (Conn->Timeout > NSDSP_MAX_TIMEOUT) Conn->Timeout = NSDSP_MAX_TIMEOUT;
  
  Conn->FlowControl = Mode[6] == 0xf8;
  
  // trash pre-existing input
  Conn->RRead = Conn->RWrite;
  Conn->ExpectBarrier = 1;
  
  // trash pre-existing output
  Conn->QCnt = 1;
  Conn->QPtr = 2;
  memset(Conn->QBufPtr,0,NSDSP_REPORT_SIZE+1);
      
  Feature[0] = 0;
  memcpy(Feature+1,Mode,NSDSP_MODE_SIZE);
  memset(Feature+NSDSP_MODE_SIZE+1,0,NSDSP_FEATURE_SIZE-NSDSP_MODE_SIZE);
  
  if (!fn_HIDSetFeature(Conn->hDev,Feature,NSDSP_FEATURE_SIZE+1)) return 0;
  memset(Feature+1,0,NSDSP_MODE_SIZE);
  if (!fn_HIDGetFeature(Conn->hDev,Feature,NSDSP_FEATURE_SIZE+1)) return 0;
  if (memcmp(Feature+1,Mode,3)) return 0;
  
  Conn->SessionId = Feature[6];
  return 1;
}

NSDSP_EXPORT(void) NSDSPSetTimeout(NSDSP_CONN_HANDLE Conn, unsigned int Timeout) {
  Conn->Timeout = Timeout;
}

NSDSP_EXPORT(int) NSDSPWrite(NSDSP_CONN_HANDLE Conn, char *Src, unsigned int Size) {
  while (Size--) {
    if ((Conn->QPtr > NSDSP_REPORT_SIZE)&&!NSDSPFlush(Conn)) return 0;
  
    Conn->QBufPtr[Conn->QPtr++] = *Src++;
    
    if (Conn->QPtr >= (Conn->QCnt + 0x40)) {
      Conn->QBufPtr[Conn->QCnt] = Conn->SessionId | ((Conn->QPtr - Conn->QCnt) - 1);
      Conn->QCnt += 0x40;
      Conn->QPtr = Conn->QCnt + 1;
    }
  }
  return 1;
}

NSDSP_EXPORT(int) NSDSPWriteCommand(NSDSP_CONN_HANDLE Conn, char Cmd, char *Src, unsigned int Size) {
  if ((Conn->QPtr + Size) >= (Conn->QCnt + 0x40)) {
    if ((Conn->QPtr > NSDSP_REPORT_SIZE)&&!NSDSPFlush(Conn)) return 0;
  
    Conn->QBufPtr[Conn->QCnt] = Conn->SessionId | ((Conn->QPtr - Conn->QCnt) - 1);
    Conn->QCnt += 0x40;
    Conn->QPtr = Conn->QCnt + 1;
  }
  
  if (!NSDSPWrite(Conn, &Cmd, 1)) return 0;
  if (Size == 0) return 1;
  
  return NSDSPWrite(Conn, Src, Size);
}

NSDSP_EXPORT(int) NSDSPWriteSPI(NSDSP_CONN_HANDLE Conn, char *Src, unsigned int Size) {
  if (Size & 0x01) {
    if (!NSDSPWriteCommand(Conn, NSDSP_CMD_SPI_WRITE_1, Src++, 1)) return 0;
  }
  if (Size & 0x02) {
    if (!NSDSPWriteCommand(Conn, NSDSP_CMD_SPI_WRITE_2, Src, 2)) return 0;
    Src += 2;
  }
  if (Size & 0x04) {
    if (!NSDSPWriteCommand(Conn, NSDSP_CMD_SPI_WRITE_4, Src, 4)) return 0;
    Src += 4;
  }
  for (Size >>= 3; Size > 0; Size--) {
    if (!NSDSPWriteCommand(Conn, NSDSP_CMD_SPI_WRITE_8, Src, 8)) return 0;
    Src += 8;
  }
  return 1;
}

NSDSP_EXPORT(int) NSDSPDelay(NSDSP_CONN_HANDLE Conn, unsigned int Delay) {
  if (Delay < 1) return 0;
  if (Delay > 0x1000001) return 0;
  
  if (Delay > 0x10000) {
    Delay -= 2;
    return NSDSPWriteCommand(Conn,  NSDSP_CMD_DELAY_3, (char *)&Delay, 3);
  }
  
  if (Delay > 0xff) {
    Delay--;
    return NSDSPWriteCommand(Conn, NSDSP_CMD_DELAY_2, (char *)&Delay, 2);
  }
  
  if (Delay > 1) return NSDSPWriteCommand(Conn, NSDSP_CMD_DELAY_1, (char *)&Delay, 1);
  
  return NSDSPWriteCommand(Conn, NSDSP_CMD_DELAY_0,NULL, 0);
}

NSDSP_EXPORT(int) NSDSPFlush(NSDSP_CONN_HANDLE Conn) {
  DWORD dwW;
  
  if (Conn->QPtr == 2) return 1;
  
  if (WaitForSingleObject(Conn->EvWrite,Conn->Timeout) != WAIT_OBJECT_0) return 0;
  
  Conn->QBufPtr[Conn->QCnt] = Conn->SessionId | ((Conn->QPtr - Conn->QCnt) - 1);
  
  memset(&Conn->OVWrite,0,sizeof(OVERLAPPED));
  Conn->OVWrite.hEvent = Conn->EvWrite;
  if (!WriteFile(Conn->hDev,Conn->QBufPtr,NSDSP_REPORT_SIZE+1,&dwW,&Conn->OVWrite)) {
    if (GetLastError() != ERROR_IO_PENDING) return 0;
  }
  
  Conn->QBufPtr = (Conn->QBufPtr == Conn->QBuf1) ? Conn->QBuf2 : Conn->QBuf1;
  memset(Conn->QBufPtr,0,NSDSP_REPORT_SIZE+1);
  Conn->QCnt = 1;
  Conn->QPtr = 2;
  return 1;
}

NSDSP_EXPORT(int) NSDSPWaitForCompletion(NSDSP_CONN_HANDLE Conn) {
  unsigned char Feature[NSDSP_FEATURE_SIZE+1];
  unsigned int BaseTime;
  
  if (WaitForSingleObject(Conn->EvWrite,Conn->Timeout) != WAIT_OBJECT_0) return 0;
    
  if (Conn->Version >= 0x101) {
    BaseTime = GetTickCount();
    while ((GetTickCount() - BaseTime) < Conn->Timeout) {
      memset(Feature,0,NSDSP_FEATURE_SIZE+1);
      if (!fn_HIDGetFeature(Conn->hDev,Feature,NSDSP_FEATURE_SIZE+1)) return 0;
      Sleep(1 + (25000/Conn->Baud));
      if ((Feature[8] & 0x04) == 0) return 1;
    }
    return 0;
  } else {
    Sleep(10 + (5000000/Conn->Baud));
  }
  
  return 1;
}

NSDSP_EXPORT(unsigned int) NSDSPAvailableData(NSDSP_CONN_HANDLE Conn) {
  return (Conn->RWrite - Conn->RRead);
}

NSDSP_EXPORT(int) NSDSPWaitForData(NSDSP_CONN_HANDLE Conn, unsigned int Size) {
  if (NSDSPAvailableData(Conn) >= Size) return 1;
  SetRequestedData(Conn, Size);
  return (WaitForSingleObject(Conn->EvProcess,Conn->Timeout) == WAIT_OBJECT_0);
}

NSDSP_EXPORT(int) NSDSPWaitForDataForever(NSDSP_CONN_HANDLE Conn, unsigned int Size) {
  if (NSDSPAvailableData(Conn) >= Size) return 1;
  SetRequestedData(Conn, Size);
  return (WaitForSingleObject(Conn->EvProcess,-1) == WAIT_OBJECT_0);
}

NSDSP_EXPORT(unsigned int) NSDSPRead(NSDSP_CONN_HANDLE Conn, char *Dst, unsigned int Size) {
  unsigned int RSize;
  
  RSize = 0;
  while ((RSize < Size)&&(NSDSPAvailableData(Conn) > 0)) {
    Dst[RSize++] = Conn->RBuf[Conn->RRead++ & RBUF_MASK];
  }
  
  WaitForSingleObject(Conn->Mutex,-1);
  if (NSDSPAvailableData(Conn) < Conn->RequestedData) ResetEvent(Conn->EvProcess);
  ReleaseMutex(Conn->Mutex);
  
  return RSize;
}
