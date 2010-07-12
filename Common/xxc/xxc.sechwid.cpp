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

#include "stdafx.h"
#include "xxc.shash.h"
#include "xxc.sechwid.h"
#include <winioctl.h>
#include "xxc.lowlevel.h"

#ifdef OS_WINCE

#define DEVICE_ID_LENGTH            20
//#define APPLICATION_DATA            "\0\0\0\0\0\0\0\0"
#define APPLICATION_DATA_LENGTH     8
#ifndef GETDEVICEUNIQUEID_V1
#	define GETDEVICEUNIQUEID_V1 1
#endif

//#define IOCTL_HAL_GET_DEVICEID  CTL_CODE(FILE_DEVICE_HAL, 21, METHOD_BUFFERED, FILE_ANY_ACCESS)

typedef HRESULT (*idfunc)(LPBYTE,DWORD,DWORD,LPBYTE,DWORD*);
typedef BOOL (*kicfunc) (DWORD, LPVOID, DWORD, LPVOID, DWORD, LPDWORD);

typedef struct _DEVICE_ID {
  DWORD dwSize;
  DWORD dwPresetIDOffset;
  DWORD dwPresetIDBytes;
  DWORD dwPlatformIDOffset;
  DWORD dwPlatformIDBytes;
} DEVICE_ID, *PDEVICE_ID; 


// Works only on devices with WM 5.0 or newer
bool GetSecDeviceId_WM5(xxc::hash& h)
{
	idfunc  GetDeviceUniqueID_func = (idfunc)pSecret->fnGetDeviceUniqueID;
	BYTE APPLICATION_DATA[ APPLICATION_DATA_LENGTH ];
	memset( APPLICATION_DATA, 0, APPLICATION_DATA_LENGTH );
	if (GetDeviceUniqueID_func) { 
		HRESULT hr = NOERROR;
		BYTE rgDeviceId[DEVICE_ID_LENGTH];
		DWORD cbDeviceId = sizeof(rgDeviceId);
		hr = GetDeviceUniqueID_func(reinterpret_cast<PBYTE>(APPLICATION_DATA), APPLICATION_DATA_LENGTH, GETDEVICEUNIQUEID_V1, rgDeviceId, &cbDeviceId);
		if(hr == NOERROR) {
			h.add( rgDeviceId, cbDeviceId );
			return true;
		}
	}
	return false;
}

// Should work on most of PPC2002+ devices
bool GetSecDeviceId_IoCtl(xxc::hash& h)
{
	kicfunc  KernelIoControl_func = (kicfunc)pSecret->fnKernelIoControl;
	if (KernelIoControl_func) { 
		BYTE deviceID[256];
		DWORD dwSize = sizeof(deviceID);
		BOOL bRes = KernelIoControl_func(IOCTL_HAL_GET_DEVICEID, NULL, 0, deviceID, dwSize, &dwSize);
		if (!bRes) {
			bRes = KernelIoControl_func(IOCTL_HAL_GET_DEVICEID, NULL, 0, deviceID, dwSize, &dwSize);
			if (!bRes) {
				bRes = KernelIoControl_func(IOCTL_HAL_GET_DEVICEID, NULL, 0, deviceID, sizeof(_DEVICE_ID) + 16, &dwSize);
			}
		}
		if (bRes && dwSize > sizeof(_DEVICE_ID)) {
			h.add( deviceID, dwSize );
			return true;
		}
	}
	return false;
}
#endif // OS_WINCE


//////////////////////////////////////////////////////////////////////////
namespace xxc {

uint32 XorSecValue(uint32 val)
{
#ifdef OS_WINCE
	xxc::hash h;
	uint32 res[8];
	h.reset();
	bool bOk = (GetSecDeviceId_WM5(h) || GetSecDeviceId_IoCtl(h));
	h.finalize( res );
	if (bOk) {
		uint32 hwid = res[0] + res[1] + res[2];
		return val ^ hwid;
	} else return 0;
#else
	return 0;
#endif // OS_WINCE
}

} // xxc

