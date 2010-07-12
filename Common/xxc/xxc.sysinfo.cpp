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

//
// device info class
// 0x3940622c - MICROSOFT DEVICEEMULATOR

#include "stdafx.h"

#include "xxc.cfg.h"
#include "xxc.defs.h"
#include "xxc.bswap.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include "xxc.sysinfo.h"

uint32 pageSize;

namespace xxc {

always_inline 
uint32 very_weird_upcase( uint32 ch )
{
	if ( ch >= 'a' && ch <= 'z' ) return ch & ~(0x20);
	return ch;
}

always_inline 
uint32 hash_to_upper( TCHAR* str )
{
	uint32 h = 13 * 104729;
	uint32 ch;
	while ( 0 != (ch = *str++) ) {
		ch = very_weird_upcase(ch)&0xff;
		*str++ = (uint16)ch;
		h ^= ( (h << 5) + (ch) + (h >> 2) );
	}
	return h;
	
}

uint32 dev_acquire( dev_info& info )
{
	// retrieve os version info
	OSVERSIONINFO versionInfo;
	versionInfo.dwOSVersionInfoSize = sizeof(versionInfo);
	GetVersionEx( &versionInfo );

	// retrieve system info
	SYSTEM_INFO	sysInfo;
	GetSystemInfo( &sysInfo );
	pageSize = sysInfo.dwPageSize;
	
	// define generic platform params
	info.type		= dev_type::unknown;
	info.version	= versionInfo.dwMajorVersion * 100 + versionInfo.dwMinorVersion;
	info.flags		= 0;
	info.oeminfo[0]= 0;
	info.platform[0]=0;
	info.lang[0] = 0;


	// WCE specifics
#ifdef _WIN32_WCE

	// figure out device type
	if ( SystemParametersInfo( SPI_GETPLATFORMTYPE, sizeof(info.platform), info.platform, 0 ) != 0 ) {
		if ( 0 == _tcsicmp( info.platform, _T("PocketPC") ) ) {
			info.type = dev_type::pocket_pc;
		} else if ( 0 == _tcsicmp( info.platform, _T("Smartphone") ) ) {
			info.type = dev_type::smartphone;
		}
	} else if ( GetLastError() == ERROR_ACCESS_DENIED ) {
		_tcscpy( info.platform, _T("Smartphone") );
		info.type = dev_type::smartphone;
	} else {
		_tcscpy( info.platform, _T("Windows CE") );
	}

	// retrieve OEM info
	SystemParametersInfo( SPI_GETOEMINFO, sizeof(info.oeminfo), info.oeminfo, 0 );

#endif //_WIN32_WCE
	// capitalize othes
	hash_to_upper( info.platform );

	// capitalize oem info stirng for the sake of simplicity
	TCHAR oemUpcase[256];
	DWORD oemHash;
	_tcscpy( oemUpcase, info.oeminfo );
	oemHash = hash_to_upper( oemUpcase );
	_tcscpy( info.oeminfo, oemUpcase );
	
	// detect cpu and model if ARM
#if defined(ARM) && defined(_WIN32_WCE)
	// cpu detection ;)
	_tcscpy( info.cpu, _T("ARM") );
	// detect cpu and model if MIPS
#elif defined(MIPS) && defined(_WIN32_WCE)
	// cpu detection
	_tcscpy( info.cpu, _T("MIPS") );
#elif defined(SH3) && defined(_WIN32_WCE)
	// cpu detection
	_tcscpy( info.cpu, _T("SH3") );
#elif defined(_M_IX86)
	_tcscpy( info.cpu, _T("x86") );
#endif // _M_IX86
	hash_to_upper( info.cpu );

	// detect locale and language code
	TCHAR		lang[16];

	if ( GetLocaleInfo( LOCALE_USER_DEFAULT, LOCALE_SABBREVLANGNAME, lang, sizeof(lang)/sizeof(TCHAR)) >= 2 ) {
		_tcsupr( lang );

		if ( 0 == _tcscmp( lang, _T("JPN")) ) {
			_tcscpy( lang, _T("JA") );
		} else if ( 0 == _tcscmp( lang, _T("GER")) ) {
			_tcscpy( lang, _T("DE") );
		} else if ( 0 == _tcscmp( lang, _T("SPA")) ) {
			_tcscpy( lang, _T("ES") );
		}

		if ( 0 == _tcscmp( lang, _T("ZHI")) || 0 == _tcscmp( lang, _T("ZHM")) ) {
			_tcscpy( lang, _T("CHS") );
		} else if ( 0 == _tcscmp( lang, _T("ZHH")) ) {
			_tcscpy( lang, _T("CHT") );
		}

		// convert copy
		_tcscpy( info.lang, lang );
	}

	return oemHash;
}

} // xxc

#ifdef TEST_SYSINFO
int WINAPI WinMain(HINSTANCE hInstance,
                   HINSTANCE hPrevInstance,
                   LPTSTR    lpCmdLine,
                   int       nCmdShow)
{
	xxc::dev_info info;
	memset( &info, 0, sizeof(info) );
	xxc::dev_acquire( info );

	wchar_t str[64];
	wsprintf( str, L"0x%08x", info.oem_hash );
	MessageBox( 0, info.oeminfo, str, MB_OK );
	
	return 0;
}
#endif // TEST_SYSINFO

