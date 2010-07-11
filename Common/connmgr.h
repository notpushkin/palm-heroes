/*
Licensed to the Apache Software Foundation (ASF) under one
or more contributor license agreements.  See the NOTICE file
distributed with this work for additional information
regarding copyright ownership.  The ASF licenses this file
to you under the Apache License, Version 2.0 (the
"License"); you may not use this file except in compliance
with the License.  You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing,
software distributed under the License is distributed on an
"AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
KIND, either express or implied.  See the License for the
specific language governing permissions and limitations
under the License.
*/

#ifndef _CONNMGR_H
#define _CONNMGR_H

#if __GNUC__ >= 3
#pragma GCC system_header
#endif

#ifdef  __cplusplus
extern "C" {
#endif
#define CONNMGR_FLAG_PROXY_HTTP              0x1
#define CONNMGR_FLAG_PROXY_WAP               0x2
#define CONNMGR_FLAG_PROXY_SOCKS4            0x4
#define CONNMGR_FLAG_PROXY_SOCKS5            0x8 

#define CONNMGR_PRIORITY_VOICE               0x20000
#define CONNMGR_PRIORITY_USERINTERACTIVE     0x08000
#define CONNMGR_PRIORITY_USERBACKGROUND      0x02000
#define CONNMGR_PRIORITY_USERIDLE             0x0800
#define CONNMGR_PRIORITY_HIPRIBKGND           0x0200
#define CONNMGR_PRIORITY_IDLEBKGND            0x0080
#define CONNMGR_PRIORITY_EXTERNALINTERACTIVRE 0x0020
#define CONNMGR_PRIORITY_LOWBKGND             0x0008
#define CONNMGR_PRIORITY_CACHED               0x0002

#define CONNMGR_PARAM_GUIDDESTNET             0x1
#define CONNMGR_PARAM_MAXCOST                 0x2
#define CONNMGR_PARAM_MINRCVBW                0x4
#define CONNMGR_PARAM_MAXCONNLATENCY          0x8


#define CONNMGR_STATUS_UNKNOWN                0x00
#define CONNMGR_STATUS_CONNECTED              0x10
#define CONNMGR_STATUS_DISCONNECTED           0x20
#define CONNMGR_STATUS_CONNECTIONFAILED       0x21
#define CONNMGR_STATUS_CONNECTIONCANCELED     0x22
#define CONNMGR_STATUS_CONNECTIONDISABLED     0x23
#define CONNMGR_STATUS_NOPATHTODESTINATION    0x24
#define CONNMGR_STATUS_WAITINGFORPATH         0x25
#define CONNMGR_STATUS_WAITINGFORPHONE        0x26
#define CONNMGR_STATUS_WAITINGCONNECTION      0x40
#define CONNMGR_STATUS_WAITINGFORRESOURCE     0x41
#define CONNMGR_STATUS_WAITINGFORNETWORK      0x42
#define CONNMGR_STATUS_WAITINGDISCONNECTION   0x80
#define CONNMGR_STATUS_WAITINGCONNECTIONABORT 0x81

DEFINE_GUID(IID_DestNetInternet,0x436ef144,0xb4fb,0x4863,0xa0,0x41,0x8f,0x90,0x5a,0x62,0xc5,0x72);
DEFINE_GUID(IID_DestNetCorp,0xa1182988,0x0d73,0x439e,0x87,0xad,0x2a,0x5b,0x36,0x9f,0x80,0x8b);
DEFINE_GUID(IID_DestNetWAP,0x7022e968,0x5a97,0x4051,0xbc,0x1c,0xc5,0x78,0xe2,0xfb,0xa5,0xd9);
DEFINE_GUID(IID_DestNetSecureWAP,0xf28d1f74,0x72be,0x4394,0xa4,0xa7,0x4e,0x29,0x62,0x19,0x39,0x0c);

typedef struct _CONNMGR_CONNECTIONINFO {
  DWORD cbSize;
  DWORD dwParams;
  DWORD dwFlags;
  DWORD dwPriority;
  BOOL bExclusive;
  BOOL bDisabled;
  GUID guidDestNet;
  HWND hWnd;
  UINT uMsg;
  LPARAM lParam;
  ULONG ulMaxCost;
  ULONG ulMinRcvBw;
  ULONG ulMaxConnLatency;
} CONNMGR_CONNECTIONINFO;

#define CONNMGR_MAX_DESC          128 

typedef struct _CONNMGR_DESTINATION_INFO {
  GUID guid;
  TCHAR szDescription[CONNMGR_MAX_DESC];
} CONNMGR_DESTINATION_INFO;

typedef struct _SCHEDULEDCONNECTIONINFO {
  GUID guidDest;
  UINT64 uiStartTime;
  UINT64 uiEndTime;
  UINT64 uiPeriod;
  TCHAR szAppName[MAX_PATH];
  TCHAR szCmdLine[MAX_PATH];
  TCHAR szToken[32];
  BOOL bPiggyback;
} SCHEDULEDCONNECTIONINFO;

HRESULT WINAPI ConnMgrEstablishConnection(CONNMGR_CONNECTIONINFO * pConnInfo,HANDLE * phConnection );
HRESULT WINAPI ConnMgrEnumDestinations(int nIndex,CONNMGR_DESTINATION_INFO * pDestInfo);
HRESULT WINAPI ConnMgrEstablishConnectionSync(CONNMGR_CONNECTIONINFO * pConnInfo,
                   HANDLE * phConnection,DWORD dwTimeout,DWORD * pdwStatus  );
HANDLE WINAPI ConnMgrApiReadyEvent();
HRESULT WINAPI ConnMgrConnectionStatus(HANDLE hConnection,DWORD * pdwStatus );
HRESULT WINAPI ConnMgrMapURL(LPCTSTR pwszURL,GUID * pguid,DWORD * pdwIndex );
HRESULT WINAPI ConnMgrProviderMessage(HANDLE hConnection,const GUID * pguidProvider,
      DWORD * pdwIndex,DWORD dwMsg1,DWORD dwMsg2,PBYTE pParams,ULONG cbParamSize );
HRESULT WINAPI ConnMgrRegisterScheduledConnection(SCHEDULEDCONNECTIONINFO * pSCI);
HRESULT WINAPI ConnMgrReleaseConnection(HANDLE hConnection,BOOL bCache );
HRESULT WINAPI ConnMgrSetConnectionPriority( HANDLE hConnection,DWORD dwPriority );
HRESULT WINAPI ConnMgrUnregisterScheduledConnection(LPCWSTR pwszToken);

#ifdef  __cplusplus
}
#endif

#endif /* _CONNMGR_H */