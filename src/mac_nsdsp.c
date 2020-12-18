/***************************************************************************
 * (C) 2017-2019 Northern Software Inc                                          *
 *                                                                         *
 * This file is free software provided by Northern Software Inc.           *
 * to demostrate USB-to-UART communications through NSDSP programmers.     *
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
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <pthread.h>
#include <sys/time.h>
#include <CoreFoundation/CoreFoundation.h>
#include <IOKit/hid/IOHIDManager.h>
#include <IOKit/hid/IOHIDKeys.h>
#include "nsdsp.h"

#define RBUF_MASK (NSDSP_INPUT_BUFFER_SIZE-1)

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
  IOHIDDeviceRef DevRef;
  IOHIDManagerRef MgrRef;
  CFRunLoopRef RunLoop;
  char *WritePtr;
  pthread_t ThreadIn, ThreadOut;
  pthread_mutex_t MutexIn, MutexOut;
  pthread_cond_t DataIn, WriteState;
  
} NSDSP_CONN_DATA;

int SignalHookInstalled = 0;

int GetCFString(CFStringRef Str, char *Dst, unsigned int Size) {
  return CFStringGetCString(Str, Dst, Size, kCFStringEncodingUTF8);
}

int GetCFNumber(CFNumberRef Nmb, unsigned int *Dst) {
  return CFNumberGetValue(Nmb, kCFNumberSInt32Type, Dst);
}

void DestroyManager(IOHIDManagerRef Manager) {
  if (!Manager) return;
  IOHIDManagerClose(Manager, 0);
  CFRelease(Manager);
}

IOHIDManagerRef NewManager() {
  IOHIDManagerRef Manager;

  Manager = IOHIDManagerCreate(kCFAllocatorDefault,0);
  
  if (!Manager) return 0;

  IOHIDManagerSetDeviceMatching(Manager,0);
  if (IOHIDManagerOpen(Manager, 0) != kIOReturnSuccess) {
    DestroyManager(Manager);
    return 0;
  }
  
  return Manager;
}

void DevEnum(const IOHIDDeviceRef DevRef, NSDSP_ENUM_DATA *Dev) {
  CFStringRef Transport, Serial;
  CFNumberRef VIDRef, PIDRef, VerRef;
  NSDSP_ENUM_STRUCT *PDev;

  if (Dev->NDev >= MAX_NSDSP_DEVICES) return;
  if (!DevRef) return;
  PDev = Dev->Dev + Dev->NDev;
    
  Transport = IOHIDDeviceGetProperty(DevRef, CFSTR(kIOHIDTransportKey));
  if (!Transport) return;
  if (!GetCFString(Transport,PDev->Path,MAX_NSDSP_PATH)) return;
  if (strcmp(PDev->Path,"USB")) return;
    
  VIDRef = IOHIDDeviceGetProperty(DevRef, CFSTR(kIOHIDVendorIDKey));
  if (!VIDRef) return;
  if (!GetCFNumber(VIDRef,&PDev->VID)) return;
  
  PIDRef = IOHIDDeviceGetProperty(DevRef, CFSTR(kIOHIDProductIDKey));
  if (!PIDRef) return;
  if (!GetCFNumber(PIDRef,&PDev->PID)) return;
    
  VerRef = IOHIDDeviceGetProperty(DevRef, CFSTR(kIOHIDVersionNumberKey));
  if (!VerRef) return;
  if (!GetCFNumber(VerRef,&PDev->Version)) return;
   
  Serial = IOHIDDeviceGetProperty(DevRef, CFSTR(kIOHIDSerialNumberKey));
  if (!Serial) return;
  if (!GetCFString(Serial,PDev->Serial,MAX_NSDSP_SERIAL)) return;
    
  if (memcmp(PDev->Serial,"NSDSP",5)) return;
 
  Dev->NDev++;
}

void DevOpen(const IOHIDDeviceRef DevRef, NSDSP_CONN_HANDLE Conn) {
  char TBuf[32];
  char SBuf[MAX_NSDSP_SERIAL];
  CFStringRef Transport, Serial;
  CFNumberRef VerRef;
  NSDSP_ENUM_STRUCT *PDev;

  if (Conn->DevRef) return;
  if (!DevRef) return;
    
  Transport = IOHIDDeviceGetProperty(DevRef, CFSTR(kIOHIDTransportKey));
  if (!Transport) return;
  if (!GetCFString(Transport,TBuf,32)) return;
  if (strcmp(TBuf,"USB")) return;
    
  VerRef = IOHIDDeviceGetProperty(DevRef, CFSTR(kIOHIDVersionNumberKey));
  if (!VerRef) return;
  if (!GetCFNumber(VerRef,&Conn->Version)) return;
   
  Serial = IOHIDDeviceGetProperty(DevRef, CFSTR(kIOHIDSerialNumberKey));
  if (!Serial) return;
  if (!GetCFString(Serial,SBuf,MAX_NSDSP_SERIAL)) return;
  if (memcmp(SBuf,"NSDSP",5)) return;    

  if (Conn->Serial[0]&&strcmp(SBuf,Conn->Serial)) return;
 
  CFRetain(DevRef);
  Conn->DevRef = DevRef;
  strcpy(Conn->Serial,SBuf);
}
 
NSDSP_EXPORT(int) NSDSPEnumerate(NSDSP_ENUM_DATA *Dev) {
  IOHIDManagerRef Manager;
  CFSetRef DevSet;
  int NDev;

  Dev->NDev = 0;

  Manager = NewManager();
  if (!Manager) return -1;
  
  DevSet = IOHIDManagerCopyDevices(Manager);
  if (DevSet) {
    NDev = CFSetGetCount(DevSet);
    if (NDev > 0) CFSetApplyFunction(DevSet, (void*)DevEnum, Dev);
    CFRelease(DevSet);
  }
  DestroyManager(Manager);
  
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
   
  pthread_mutex_lock(&Conn->MutexIn); 
  pthread_cond_signal(&Conn->DataIn);
  pthread_mutex_unlock(&Conn->MutexIn);
}

void InputCallback(NSDSP_CONN_HANDLE Conn, IOReturn result, void *sender,
  IOHIDReportType ReportType, uint32_t ReportId, unsigned char *ReportBuf, CFIndex Len) {
  ProcessIncoming(Conn, ReportBuf);
}

void *ReaderThreadProc(NSDSP_CONN_HANDLE Conn) {
  uint8_t RBuf[NSDSP_INPUT_SIZE];
  
  Conn->RunLoop = CFRunLoopGetCurrent();
  
  IOHIDDeviceRegisterInputReportCallback(Conn->DevRef,
    RBuf,NSDSP_INPUT_SIZE,(void *)InputCallback,Conn);
  IOHIDDeviceScheduleWithRunLoop(Conn->DevRef, Conn->RunLoop, kCFRunLoopDefaultMode);
  
  CFRunLoopRun();
    
  IOHIDDeviceUnscheduleFromRunLoop(Conn->DevRef, Conn->RunLoop, kCFRunLoopDefaultMode);
  IOHIDDeviceRegisterInputReportCallback(Conn->DevRef,RBuf,NSDSP_INPUT_SIZE, NULL, 0);
  
  pthread_exit(NULL);
  return NULL;
}

void *WriterThreadProc(NSDSP_CONN_HANDLE Conn) {
  while (!Conn->Killing) { 
    pthread_mutex_lock(&Conn->MutexOut);
    while (!Conn->WritePtr&&!Conn->Killing) pthread_cond_wait(&Conn->WriteState,&Conn->MutexOut);
    pthread_mutex_unlock(&Conn->MutexOut);

    if (Conn->Killing) break; 

    if (IOHIDDeviceSetReport(Conn->DevRef,kIOHIDReportTypeOutput,0,
      (void*)(Conn->WritePtr+1),NSDSP_REPORT_SIZE) != kIOReturnSuccess) break;

    if (Conn->Killing) break;

    pthread_mutex_lock(&Conn->MutexOut);
    Conn->WritePtr = NULL;
    pthread_cond_broadcast(&Conn->WriteState);
    pthread_mutex_unlock(&Conn->MutexOut);
  }

  pthread_exit(NULL);
  return NULL;
}

void SetTS(NSDSP_CONN_HANDLE Conn, struct timespec *TS) {
  struct timeval t;
  gettimeofday(&t,NULL);
  TS->tv_sec = t.tv_sec;
  TS->tv_nsec = t.tv_usec * 1000;
  TS->tv_nsec += (Conn->Timeout%1000) * 1000000;
  if (TS->tv_nsec > 999999999) {
    TS->tv_nsec -= 1000000000;
    TS->tv_sec++;
  }
  TS->tv_sec += Conn->Timeout/1000;
}

unsigned int GetTickCount() {
  struct timeval t;
  gettimeofday(&t,NULL);
  return (unsigned int)t.tv_sec*1000 + (unsigned int)t.tv_usec/1000;
}

void EmptySignalHandler(int SigNum) {
  
}

NSDSP_EXPORT(NSDSP_CONN_HANDLE) NSDSPConnect(char *Serial) {
  NSDSP_ENUM_DATA Dev;
  NSDSP_CONN_HANDLE Conn;
  CFSetRef DevSet;
  int NDev;
  struct sigaction SA;

  if (!SignalHookInstalled) {
    memset(&SA, 0, sizeof(struct sigaction));
    SA.sa_handler = EmptySignalHandler;
    SA.sa_flags = 0;
    sigemptyset(&SA.sa_mask);
    sigaction(SIGUSR2, &SA, NULL);
    SignalHookInstalled = 1;
  }

  if (Serial&&(strlen(Serial) >= MAX_NSDSP_SERIAL)) return NULL;
  
  Conn = malloc(sizeof(NSDSP_CONN_DATA));
  if (!Conn) return NULL; 
  memset(Conn,0,sizeof(NSDSP_CONN_DATA));
  if (Serial) strcpy(Conn->Serial, Serial);

  Conn->MgrRef = NewManager();
  if (!Conn->MgrRef) {
    free(Conn);
    return NULL;
  }

  DevSet = IOHIDManagerCopyDevices(Conn->MgrRef);
  if (DevSet) {
    NDev = CFSetGetCount(DevSet);
    if (NDev > 0) CFSetApplyFunction(DevSet, (void*)DevOpen, Conn);
    CFRelease(DevSet);
  }
  
  if (!Conn->DevRef) {
    DestroyManager(Conn->MgrRef);
    free(Conn);
    return NULL;
  }
  
  Conn->QCnt = 1;
  Conn->QPtr = 2;
  Conn->QBufPtr = Conn->QBuf1;

  pthread_cond_init(&Conn->DataIn,NULL);
  pthread_cond_init(&Conn->WriteState,NULL);
  pthread_mutex_init(&Conn->MutexIn,NULL);
  pthread_mutex_init(&Conn->MutexOut,NULL);  
  pthread_create(&Conn->ThreadIn,NULL,(void*)ReaderThreadProc,Conn);
  pthread_create(&Conn->ThreadOut,NULL,(void*)WriterThreadProc,Conn);
  
  return Conn;
}

NSDSP_EXPORT(NSDSP_CONN_HANDLE) NSDSPDisconnect(NSDSP_CONN_HANDLE Conn) {
  Conn->Killing = 1;

  while (Conn->RunLoop == 0) sleep(1);
  while (!CFRunLoopIsWaiting(Conn->RunLoop)) sleep(1);
  CFRunLoopStop(Conn->RunLoop);

  pthread_join(Conn->ThreadIn,NULL);
  pthread_kill(Conn->ThreadOut,SIGUSR2);
  pthread_join(Conn->ThreadOut,NULL);

  pthread_mutex_destroy(&Conn->MutexIn);
  pthread_mutex_destroy(&Conn->MutexOut);  
  pthread_cond_destroy(&Conn->DataIn);
  pthread_cond_destroy(&Conn->WriteState);

  CFRelease(Conn->DevRef);
  DestroyManager(Conn->MgrRef);

  free(Conn);
  return NULL
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
  unsigned char Feature[NSDSP_FEATURE_SIZE];
  CFIndex Size;
  
  Size = NSDSP_FEATURE_SIZE;
  if (IOHIDDeviceGetReport(Conn->DevRef,kIOHIDReportTypeFeature,
    0,Feature,&Size) != kIOReturnSuccess) return -1;
  
  return !!(Feature[7] & 0x10);
}

NSDSP_EXPORT(int) NSDSPGetRX(NSDSP_CONN_HANDLE Conn) {
  unsigned char Feature[NSDSP_FEATURE_SIZE];
  CFIndex Size;
  
  Size = NSDSP_FEATURE_SIZE;
  if (IOHIDDeviceGetReport(Conn->DevRef,kIOHIDReportTypeFeature,
    0,Feature,&Size) != kIOReturnSuccess) return -1;
  
  return !!(Feature[8] & 0x20);
}

NSDSP_EXPORT(int) NSDSPSetMode(NSDSP_CONN_HANDLE Conn, unsigned char *Mode) {
  unsigned char Feature[NSDSP_FEATURE_SIZE];
  CFIndex Size;
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
      
  memcpy(Feature,Mode,NSDSP_MODE_SIZE);
  memset(Feature+NSDSP_MODE_SIZE,0,NSDSP_FEATURE_SIZE-NSDSP_MODE_SIZE);
  
  Size = NSDSP_FEATURE_SIZE;
  if (IOHIDDeviceSetReport(Conn->DevRef,kIOHIDReportTypeFeature,
    0,Feature,Size) != kIOReturnSuccess) return 0;
  memset(Feature,0,NSDSP_MODE_SIZE);
  if (IOHIDDeviceGetReport(Conn->DevRef,kIOHIDReportTypeFeature,
    0,Feature,&Size) != kIOReturnSuccess) return 0;
  if (memcmp(Feature,Mode,3)) return 0;
  
  Conn->SessionId = Feature[5];
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

NSDSP_EXPORT(int) NSDSPWriteCommand(NSDSP_CONN_HANDLE Conn, char Cmd, char *Src, int Size) {
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

NSDSP_EXPORT(int) NSDSPWriteSPI(NSDSP_CONN_HANDLE Conn, char *Src, int Size) {
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

NSDSP_EXPORT(int) NSDSPDelay(NSDSP_CONN_HANDLE Conn, int Delay) {
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
  struct timespec TS;
  int R;
  
  if (Conn->QPtr == 2) return 1;

  SetTS(Conn,&TS);
  R = 0;

  pthread_mutex_lock(&Conn->MutexOut);
  while (Conn->WritePtr&&(R == 0)) 
    R = pthread_cond_timedwait(&Conn->WriteState,&Conn->MutexOut,&TS);

  if (R == 0) {
    Conn->QBufPtr[Conn->QCnt] = Conn->SessionId | ((Conn->QPtr - Conn->QCnt) - 1);
  
    Conn->WritePtr = Conn->QBufPtr;
    pthread_cond_broadcast(&Conn->WriteState);
  }
  pthread_mutex_unlock(&Conn->MutexOut);
  
  if (R) return 0;
  
  Conn->QBufPtr = (Conn->QBufPtr == Conn->QBuf1) ? Conn->QBuf2 : Conn->QBuf1;
  memset(Conn->QBufPtr,0,NSDSP_REPORT_SIZE+1);
  Conn->QCnt = 1;
  Conn->QPtr = 2;

  return 1;
}

NSDSP_EXPORT(int) NSDSPWaitForCompletion(NSDSP_CONN_HANDLE Conn) {
  unsigned char Feature[NSDSP_FEATURE_SIZE];
  CFIndex Size;
  unsigned int BaseTime;
  struct timespec TS;
  int R;
  
  SetTS(Conn,&TS);
  R = 0;

  pthread_mutex_lock(&Conn->MutexOut);
  while (Conn->WritePtr&&(R == 0)) 
    R = pthread_cond_timedwait(&Conn->WriteState,&Conn->MutexOut,&TS);
  pthread_mutex_unlock(&Conn->MutexOut);
  
  if (R) return 0;
  
  if (Conn->Version >= 0x101) {
    BaseTime = GetTickCount();
    Size = NSDSP_FEATURE_SIZE;
    while ((GetTickCount() - BaseTime) < Conn->Timeout) {
      if (IOHIDDeviceGetReport(Conn->DevRef,kIOHIDReportTypeFeature,
        0,Feature,&Size) != kIOReturnSuccess) return 0;
      sleep(1 + (25000/Conn->Baud));
      if ((Feature[8] & 0x04) == 0) return 1;
    }
    return 0;
  } else {
    sleep(10 + (5000000/Conn->Baud));
  }
  
  return 1;
}

NSDSP_EXPORT(unsigned int) NSDSPAvailableData(NSDSP_CONN_HANDLE Conn) {
  return (Conn->RWrite - Conn->RRead);
}

NSDSP_EXPORT(int) NSDSPWaitForData(NSDSP_CONN_HANDLE Conn, unsigned int Size) {
  struct timespec TS;
  int R;

  SetTS(Conn,&TS);
  R = 0;

  pthread_mutex_lock(&Conn->MutexIn);
  while ((NSDSPAvailableData(Conn) < Size)&&(R == 0))
    R = pthread_cond_timedwait(&Conn->DataIn,&Conn->MutexIn,&TS);
  pthread_mutex_unlock(&Conn->MutexIn);
  return (R == 0);
}

NSDSP_EXPORT(int) NSDSPWaitForDataForever(NSDSP_CONN_HANDLE Conn, unsigned int Size) {
  int R;

  R = 0;

  pthread_mutex_lock(&Conn->MutexIn);
  while ((NSDSPAvailableData(Conn) < Size)&&(R == 0))
    R = pthread_cond_wait(&Conn->DataIn,&Conn->MutexIn);
  pthread_mutex_unlock(&Conn->MutexIn);
  return (R == 0);
}

NSDSP_EXPORT(unsigned int) NSDSPRead(NSDSP_CONN_HANDLE Conn, char *Dst, unsigned int Size) {
  unsigned int RSize;
  
  RSize = 0;
  while ((RSize < Size)&&(NSDSPAvailableData(Conn) > 0)) {
    Dst[RSize++] = Conn->RBuf[Conn->RRead++ & RBUF_MASK];
  }
  
  return RSize;
}
