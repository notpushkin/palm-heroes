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
//#include <windows.h>

#include "xxc.cfg.h"
#include "xxc.shash.h"
#include "xxc.hwid.h"
#include <winioctl.h>
#include "xxc.lowlevel.h"

#ifdef OS_WINCE

#define DEVICE_ID_LENGTH            20
//#define APPLICATION_DATA            "\0\0\0\0\0\0\0\0"
#define APPLICATION_DATA_LENGTH     8
#ifndef GETDEVICEUNIQUEID_V1
#	define GETDEVICEUNIQUEID_V1 1
#endif

typedef HRESULT (*idfunc)(LPBYTE,DWORD,DWORD,LPBYTE,DWORD*);
typedef BOOL (*kicfunc) (DWORD, LPVOID, DWORD, LPVOID, DWORD, LPDWORD);


///#define IOCTL_HAL_GET_DEVICEID  CTL_CODE(FILE_DEVICE_HAL, 21, METHOD_BUFFERED, FILE_ANY_ACCESS)

typedef struct _DEVICE_ID {
  DWORD dwSize;
  DWORD dwPresetIDOffset;
  DWORD dwPresetIDBytes;
  DWORD dwPlatformIDOffset;
  DWORD dwPlatformIDBytes;
} DEVICE_ID, *PDEVICE_ID; 


//
inline void DecBuff(const uint16* srcBuff, uint32 srcBuffLen, uint16* dstBuff, uint32 dstBuffLen)
{
	uint32 cnt =  iMIN(srcBuffLen, dstBuffLen) - 1;
	uint32 nn;
	for (nn=0; nn<cnt; ++nn) {
		dstBuff[nn] = srcBuff[nn] ^ (0x1234 + nn);
	}
	dstBuff[nn] = '\0';
}

const uint16 FUncName_GetDeviceUniqueID[] = { 0x1273, 0x1250, 0x1242, 0x1273, 0x125D, 0x124F, 0x1253, 0x1258, 0x1259, 0x1268, 0x1250, 0x1256, 0x1231, 0x1234, 0x1227, 0x120A, 0x1200, 0x0000 };
const uint16 FUncName_KernelIoControl[] = { 0x127F, 0x1250, 0x1244, 0x1259, 0x125D, 0x1255, 0x1273, 0x1254, 0x127F, 0x1252, 0x1250, 0x124B, 0x1232, 0x122E, 0x122E, 0x0000 };

// Works only on devices with WM 5.0 or newer
bool GetDeviceId_WM5(xxc::hash& h)
{
	WCHAR funcName[64];
	BYTE APPLICATION_DATA[ APPLICATION_DATA_LENGTH ];
	memset( APPLICATION_DATA, 0, APPLICATION_DATA_LENGTH );
	DecBuff(FUncName_GetDeviceUniqueID, sizeof(FUncName_GetDeviceUniqueID)/sizeof(uint16), (uint16*)funcName, 64);
	idfunc  GetDeviceUniqueID_func = (idfunc)GetProcAddress(*pCoreHandle, funcName);
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
bool GetDeviceId_IoCtl(xxc::hash& h)
{
	WCHAR funcName[64];
	DecBuff(FUncName_KernelIoControl, sizeof(FUncName_KernelIoControl)/sizeof(uint16), (uint16*)funcName, 64);
	kicfunc  KernelIoControl_func = (kicfunc)GetProcAddress(*pCoreHandle, funcName);
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

// 
bool GetDeviceId_OwnerName(xxc::hash& h)
{
	HKEY hkey;
	if (RegOpenKeyEx( HKEY_CURRENT_USER, TEXT("\\ControlPanel\\Owner"), 0, 0, &hkey) != ERROR_SUCCESS) return false; 
	DWORD dwType = REG_BINARY; 
	DWORD dwOwnerSize = 0x300 ; 
	WCHAR *pOwner = new WCHAR[dwOwnerSize / 2]; 
	memset(pOwner, 0, dwOwnerSize); 
	if (RegQueryValueEx( hkey, TEXT("Owner"), 0, &dwType, (PBYTE)pOwner, &dwOwnerSize ) != ERROR_SUCCESS) { 
		RegCloseKey(hkey);
		delete[] pOwner;
		return false;
	}
	h.add( pOwner, wcslen(pOwner));
	RegCloseKey(hkey);
	delete[] pOwner;
	return true;
}
#endif // OS_WINCE


//////////////////////////////////////////////////////////////////////////
namespace xxc {

void GetDeviceId(xxc::hash& h)
{
#ifdef OS_WINCE
	if (!GetDeviceId_WM5(h) && !GetDeviceId_IoCtl(h)/* && !GetDeviceId_OwnerName(h) */) return;
#endif // OS_WINCE
}


/*
 *
 */
class iBitStream 
{
public:
	iBitStream(size_t bit_count)
	{
		// init bit stream buffer
		m_bRead = false;
		m_buffSiz = (bit_count+7) / 8;
		m_pBuff = (unsigned char*)calloc( m_buffSiz, 1 );
		m_bsPos = 0;
	}

	iBitStream(const unsigned char* buff, size_t buff_siz)
	{
		// init bit stream buffer
		m_bRead = true;
		m_buffSiz = buff_siz;
		m_pBuff = (unsigned char*)malloc( m_buffSiz );
		memcpy(m_pBuff, buff, m_buffSiz);
		m_bsPos = 0;
	}


	~iBitStream() { free(m_pBuff); }

	inline void Write(unsigned int val)
	{
		if (val) {
			unsigned char* ptr = m_pBuff + (m_bsPos>>3);
			size_t bitPos = m_bsPos & 0x7;
			*ptr |= (1<<bitPos);
		}
		m_bsPos++;
	}

	inline void Write(unsigned int val, size_t bits)
	{
		while (bits--) {
			Write(val&0x1);
			val >>= 1;
		}
	}

	inline unsigned int Read()
	{
		unsigned char* ptr = m_pBuff + (m_bsPos>>3);
		size_t bitPos = m_bsPos & 0x7;
		m_bsPos++;
		return ((*ptr) >> bitPos) & 0x1;
	}

	inline unsigned int Read(size_t bits)
	{
		unsigned int res = 0;
		for (size_t bb=0; bb<bits; ++bb) {
			res |= (Read() << bb);
		}
		return res;
	}

	inline const unsigned char* GetRawData() const { return m_pBuff; }
	inline size_t GetRawDataSiz() const { return m_buffSiz; }

private:
	bool			m_bRead;
	size_t			m_buffSiz;
	unsigned char* 	m_pBuff;
	size_t			m_bsPos;
};

//////////////////////////////////////////////////////////////////////////
const WCHAR 		BIN2TXT[32+1] = { L"0123456789ACEFGHJKLMNPQRSTUVWXYZ" };
//const unsigned char TXT2BIN[26] = { 0, 0xFF, 1, 0xFF, 2, 3, 4, 5, 0xFF, 6, 7, 8, 9, 10, 0xFF, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21 };

inline unsigned char Char2Int(WCHAR c)
{
//	if (ch >= '0' && ch <= '9') return ch - '0';
//	else if (ch >= 'A' && ch <= 'Z') return TXT2BIN[ch - 'A'] + 10;
//	check(0);
//	return 0xFF;
	WCHAR cd = '0';
	if ( c > '@' ) c &= ~(0x20);
	if ( c > '@' ) cd+= ('A'-':');
	if ( c > 'A' ) cd++;
	if ( c > 'C' ) cd++;
	if ( c > 'H' ) cd++;
	if ( c > 'N' ) cd++;
	if ( (c == '-') || (c > 'Z') ) return 255;
	return c - cd;
}

inline WCHAR Int2Char (unsigned char i)
{ check(i<32); return BIN2TXT[ (i&0x1F) ]; }

void Text2Bin(LPCWSTR text, size_t sizText, unsigned char* bin, size_t sizBin)
{
	iBitStream wbs(sizBin*8);
	const WCHAR* ptr = text;
	while (sizText) {
		wbs.Write(Char2Int(*ptr++), 5);
		sizText--;
	}
	memcpy(bin, wbs.GetRawData(), sizBin);
}

void Bin2Text(const unsigned char* bin, size_t sizBin, LPWSTR text, size_t sizText)
{
	// check the size of output buffer
	//check ( (sizBin * 8 + 4) / 5 <= sizText );
	iBitStream rbs(bin, sizBin);
	WCHAR* ptr = text;
	size_t to_read = (sizBin * 8 + 4) / 5;
	while (to_read) {
		*ptr = Int2Char(rbs.Read(5));
		ptr++;
		to_read--;
	}
	*ptr = 0;
}

} //namespace xxc
