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
#include <dirent.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/time.h>
#include <pthread.h>
#include <sys/select.h>
#include <sys/ioctl.h>
#include <linux/hidraw.h>
#include "nsdsp.h"

#define CLASS_ROOT "/sys/class/hidraw/"
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
  int hDev;
  char *WritePtr;
  pthread_t ThreadIn, ThreadOut;
  pthread_mutex_t MutexIn, MutexOut;
  pthread_cond_t DataIn, WriteState;
  
} NSDSP_CONN_DATA;

int ReadFileContent(char *FileName, char *Dst, unsigned int Size) {
  int f, n;
  f = open(FileName,O_RDONLY);
  if (f == -1) return 0;

  if ((n = read(f,Dst,Size)) <= 0) return 0;

  Dst[n] = 0;
  return 1;
}
 
NSDSP_EXPORT(int) NSDSPEnumerate(NSDSP_ENUM_DATA *Dev) {
  char Buf[256];
  char Lnk[256];
  char LnkN[256];
  char VIDBuf[8];
  char PIDBuf[8];
  char VerBuf[8];
  char UDevBuf[2048];
  char *x, *y, *z, *u;
  int L, N;
  DIR *Dir;
  struct dirent *D;
  NSDSP_ENUM_STRUCT *PDev;

  Dev->NDev = 0;
  
  strcpy(Buf,CLASS_ROOT);
  x = Buf + strlen(Buf);
  
  Dir = opendir(Buf);
  if (!Dir) return -1;

  while (Dev->NDev < MAX_NSDSP_DEVICES) {
    D = readdir(Dir);
    if (!D) break;

    strcpy(x,D->d_name);
    L = readlink(Buf,Lnk,254);
    if (L <= 0) continue;
    Lnk[L] = 0;

    strcpy(LnkN,CLASS_ROOT);
    z = LnkN + strlen(LnkN);
    y = Lnk;
    N = 0;
    while (!memcmp(y,"../",3)) {
      y += 3;
      if (N < 3) {
        N++;
        z--;
        while (*--z != '/');
        z++;
      }
    }
    strcpy(z,y);
    strcpy(Lnk,LnkN); // device path

    y = LnkN + strlen(LnkN);
    while (1) {
      while (*--y != '/');
      y++;

      PDev = Dev->Dev + Dev->NDev;

      strcpy(y,"serial");

      u = PDev->Serial;
      if (!ReadFileContent(LnkN,u,MAX_NSDSP_SERIAL)) goto not_found;
      while (*u) {
        if (*u == '\n') {
          *u = 0;
          break;
        }
        u++;
      }
      if (memcmp(PDev->Serial,"NSDSP",5)) goto not_found;
      
      strcpy(y,"idVendor");
      if (!ReadFileContent(LnkN,VIDBuf,4)) goto not_found;
      VIDBuf[4] = 0;
      PDev->VID = strtol(VIDBuf,NULL,16);

      strcpy(y,"idProduct");
      if (!ReadFileContent(LnkN,PIDBuf,4)) goto not_found;
      PIDBuf[4] = 0;
      PDev->PID = strtol(PIDBuf,NULL,16);

      strcpy(y,"bcdDevice");
      if (!ReadFileContent(LnkN,VerBuf,4)) goto not_found;
      VerBuf[4] = 0;
      PDev->Version = strtol(VerBuf,NULL,16);      

      strcat(Lnk,"/uevent");
      if (!ReadFileContent(Lnk,UDevBuf,2047)) break;
      
      z = strstr(UDevBuf,"DEVNAME=");
      if (!z) break;
      z += 8;
      u = z;
      while (*u) {
        if (*u == '\n') {
          *u = 0;
          break;
        }
        u++;
      }

      if (*z == '/') {
        strcpy(PDev->Path,z);
      } else {
        strcpy(PDev->Path,"/dev/");
        strcat(PDev->Path,z);
      }

      Dev->NDev++;
      break;

    not_found:
      if (y == z) break;
      y -= 2;
    }
  }
  
  return Dev->NDev;
}

void FlushHIDQueue(int h) {
  fd_set R, E;
  struct timeval TV;
  char RBuf[NSDSP_INPUT_SIZE];
  while(1) {
    FD_ZERO(&R);
    FD_SET(h,&R);
    FD_ZERO(&E);
    FD_SET(h,&E);
    TV.tv_sec = 0;
    TV.tv_usec = 0;
    if (select(h+1,&R,NULL,&E,&TV) <= 0) return;
    if (!FD_ISSET(h,&E)) return;
    RBuf[0] = 0;
    if (read(h,&RBuf,NSDSP_INPUT_SIZE) <= 0) return;
  }
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

void *ReaderThreadProc(NSDSP_CONN_HANDLE Conn) {
  unsigned char RBuf[NSDSP_INPUT_SIZE];
  
  FlushHIDQueue(Conn->hDev);
  
  while (!Conn->Killing) { 
    *RBuf = 0;
    if (read(Conn->hDev,&RBuf,NSDSP_INPUT_SIZE) != NSDSP_INPUT_SIZE) break;
    if (Conn->Killing) break;
    ProcessIncoming(Conn,RBuf);
  }

  return NULL;
}

void *WriterThreadProc(NSDSP_CONN_HANDLE Conn) { 
  while (!Conn->Killing) { 
    pthread_mutex_lock(&Conn->MutexOut);
    while (!Conn->WritePtr) pthread_cond_wait(&Conn->WriteState,&Conn->MutexOut);
    pthread_mutex_unlock(&Conn->MutexOut);

    if (write(Conn->hDev,Conn->WritePtr,NSDSP_REPORT_SIZE+1) != 
      (NSDSP_REPORT_SIZE+1)) break;

    pthread_mutex_lock(&Conn->MutexOut);
    Conn->WritePtr = NULL;
    pthread_cond_broadcast(&Conn->WriteState);
    pthread_mutex_unlock(&Conn->MutexOut);
  }

  return NULL;
}

void SetTS(NSDSP_CONN_HANDLE Conn, struct timespec *TS) {
  clock_gettime(CLOCK_REALTIME,TS);
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

NSDSP_EXPORT(NSDSP_CONN_HANDLE) NSDSPConnect(char *Serial) {
  NSDSP_ENUM_DATA Dev;
  NSDSP_CONN_HANDLE Conn;
  int hDev, i, Idx;
  
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
    
  if (Idx < 0) return 0;
  
  hDev = open(Dev.Dev[Idx].Path,O_RDWR);
  if (hDev == -1) return NULL;

  Conn = malloc(sizeof(NSDSP_CONN_DATA));
  if (!Conn) {
    close(hDev);
    return NULL;
  }
  
  memset(Conn,0,sizeof(NSDSP_CONN_DATA));
  Conn->hDev = hDev;
  
  strcpy(Conn->Serial,Dev.Dev[Idx].Serial);
  Conn->Version = Dev.Dev[Idx].Version;
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
  pthread_cancel(Conn->ThreadIn);
  pthread_join(Conn->ThreadIn,NULL);
  pthread_cancel(Conn->ThreadOut);
  pthread_join(Conn->ThreadOut,NULL);
  pthread_mutex_destroy(&Conn->MutexIn);
  pthread_mutex_destroy(&Conn->MutexOut);  
  pthread_cond_destroy(&Conn->DataIn);
  pthread_cond_destroy(&Conn->WriteState);
  close(Conn->hDev);
  free(Conn);
  return NULL;
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
  
  Feature[0] = 0;
  if (ioctl(Conn->hDev,HIDIOCGFEATURE(NSDSP_FEATURE_SIZE+1),Feature) < 0) return 0;
  
  return !!(Feature[8] & 0x10);
}

NSDSP_EXPORT(int) NSDSPGetRX(NSDSP_CONN_HANDLE Conn) {
  unsigned char Feature[NSDSP_FEATURE_SIZE+1];
  
  Feature[0] = 0;
  if (ioctl(Conn->hDev,HIDIOCGFEATURE(NSDSP_FEATURE_SIZE+1),Feature) < 0) return 0;
  
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
  
  if (ioctl(Conn->hDev,HIDIOCSFEATURE(NSDSP_FEATURE_SIZE+1),Feature) < 0) return 0;
  memset(Feature+1,0,NSDSP_MODE_SIZE);
  if (ioctl(Conn->hDev,HIDIOCGFEATURE(NSDSP_FEATURE_SIZE+1),Feature) < 0) return 0;
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
  unsigned char Feature[NSDSP_FEATURE_SIZE+1];
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
    Feature[0] = 0;
    while ((GetTickCount() - BaseTime) < Conn->Timeout) {
      if (ioctl(Conn->hDev,HIDIOCGFEATURE(NSDSP_FEATURE_SIZE+1),Feature) < 0) return 0;
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
