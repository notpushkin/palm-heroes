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
//

#include "stdafx.h"

#include "xxc.cfg.h"
#include "xxc.defs.h"
#include "xxc.app.h"
#include "xxc.gf2.h"
#include "xxc.shash.h"
#include "xxc.dllfn.h"
#include "xxc.security.h"

#include "xxc.lowlevel.h"
#include "wce.psyscall.h"

namespace xxc {

// set the kernel mode
uint32 jump_setkmode( void* str )
{
	// for now use straightforward call

	// OBUSCATED!
	//uint32 cmdBranch = 1;
	//while ( cmdBranch != 0 ) {
	//	switch ( cmdBranch ) {
	//		case 1:
	//			cmdBranch = 2;
	//			break;
	//		case 2:
	//			cmdBranch = 3;
	//			break;
	//		case 3:
	//			cmdBranch = 0;
	//			break;
	//	}
	//}

#ifdef OS_WINCE
	HMODULE hcore = LoadLibrary(L"coredll.dll");
	// store hcore under SEC_HCORE
	snode_t* key = store_insert( str, SEC_HCORE );
	key->data = (void*)hcore;

	BOOL (WINAPI* FuncSetKMode)(BOOL) = NULL;
	FuncSetKMode = (BOOL (WINAPI*)(BOOL)) GetProcAddress ( hcore, L"SetKMode" );



	if (FuncSetKMode) {
		return FuncSetKMode(1);
	}
#endif
	return 0;
}

// ANTIDEBUG: disables low-level fault 
// kernel mode required
uint32 jump_disablefaults( void* )
{
	XXC_DISABLEFAULTS();
	return 0;
}

/*
// ANTIDEBUG: detects presence of the debugger
// stores result under SEC_DBG_PRESENT
uint32 jump_checkdebuger( void* str )
{
	// OBUSCATED!
	uint32 cmdBranch = 1;
	uint32 addr = ~0x0000;
	uint32 pptr = 0xdead;

	while ( cmdBranch != 0 ) {
		switch ( cmdBranch ) {
			case 1:
				addr -= 0x37ff;
				// OBFUSCATION
				if ( addr == 0 ) {
					cmdBranch = 1;
				} else {
					addr += 0x090;
					cmdBranch = 2;
				}
				break;
			case 2:
				{
					void* pptr = *((void**)( addr ));
					if ( !pptr ) {
						cmdBranch = 3;
					} else {
						// OBFUSCATION
						cmdBranch = 1;
					}
				}
				break;
			case 3:
				{
					uint32 res = *(uint32*)( (char*)pptr + 0x1c );
					res *= 3;
					// store result
					snode_t* key = store_insert( str, SEC_DBG_PRESENT );
					key->data = (void*)res;
				}
				cmdBranch = 0;
				break;
		}
	}
	return 0;
	//uint32 addrCrypt
	//void* pptr = *((void**)(0xffffc800+0x090));
	//return *(DWORD*)( (char*)pptr + 0x1c );
}

// ANTIDUMP: checks number of PE sections -
// packed file should have exactly 5
// stores result under SEC_PE_SECT
uint32 jump_checkPE( void* str )
{

	// OBFUSCATION
	uint32 z = 11;
	uint32 addr = 0;
	uint32 pptr = 0;
	uint32 res = 71;
	for( uint32 n = 0; n != 100; n++ ) {
		if ( z == 11 ) {
			addr = ~addr;
			addr -= 0x37ff;
			z = 31;
		} else if ( z == 31) {
			addr += 0x090;
			z *= 2;
		} else if ( z == 62 ) {
			void* pptr = *((void**)(addr));
			void* zptr = (void*)( (char*)pptr + 0x50 + 0x10 );
			e32_lite* oe = (e32_lite*)zptr;

			res += oe->e32_objcnt;
			// crypt it as (71 + x)*4  (i.e. correct shoudl be 304 )
			res <<= 2;
			z += res;
		} else {
			// store 
			snode_t* key = store_insert( str, SEC_PE_SECT );
			key->data = (void*)res;
			key++;
			// end of cycle
			z >>= 3;
			n = 100;
		}
	}

	//void* pptr = *((void**)(0xffffc800+0x090));
	//return (void*)( (char*)pptr + 0x50 + 0x10 );
	return 0;
}
*/

// secret HW id - based on direct trap call
uint32 jump_hwid( void* str )
{
	UINT32 fnAddr = 0xf0000000;
	if ( fnAddr ) {
		fnAddr += 0xffff;
	} else {
		// OBfUSCATION
		fnAddr >>= 4;
	}
	fnAddr -= 0x018B;
	// 0xF000FE74
	// store it under SEC_HWID_FN
	snode_t* key = store_insert( str, SEC_HWID_FN );
	key->data = (void*)fnAddr;

	// create hash object and put its ptr
	xxc::hash hasher;
	hasher.reset();
	key = store_insert( str, SEC_HWID_HASH );
	key->data = (void*)&hasher;

	// retrieve 

	// retrieve addr of check function (SEC_HWID_CHECK1)
	// and call it
	//key = store_find( str, SEC_HWID_CHECK1 );
	
	// calls WM5 function
	uint32 success =  XXC_JUMPID( str, SEC_HWID_CHECK1, str );

	// finalize hash & store result
	uint32 res[8];
	memset( res, 0, sizeof(res) );
	hasher.finalize( res );

	key = store_insert( str, SEC_HWID_CNST );
	key->data = (void*)(res[0] + res[1] + res[2]);
	if ( !success ) key->data = (void*)0;

	return XXC_JUMPID( str, SEC_FN_DFAULT, str );
}

uint32 get_ioctl_deviceid()
{
	static volatile uint32 some = 0x01010101;
	uint32 res = some << 15;
	res |= 0x2a;
	res <<= 1;
	check( res == IOCTL_HAL_GET_DEVICEID)
	return res;
}

// actually calls ioctl to check HWid
// returns 1 upon success	
// and hash object contains result
// requires SEC_HWID_FN set
// and SEC_HWID_HASH set
uint32 jump_hwid_check_kio( void* str )
{
	// retrieve fnAddr
	snode_t* key = store_find( str, SEC_HWID_FN );
	void* ppfn = key->data;

	// TODO:: hide IOCTL code!
	fnKIO_t	KernelIoControl_func = (fnKIO_t)ppfn;
	if (KernelIoControl_func) { 
		BYTE deviceID[256];
		DWORD dwSize = sizeof(deviceID);
		DWORD dwIOCTLCode = get_ioctl_deviceid();
		BOOL bRes = FALSE;

		for( uint32 nc = 0; nc != 3; nc++ ) {
			bRes = KernelIoControl_func(dwIOCTLCode, NULL, 0, deviceID, dwSize, &dwSize);
			if ( bRes ) break;
			if ( nc > 1 ) dwSize = SIZE_OF_DEVICEID + 16;
		}

		if (bRes && dwSize > SIZE_OF_DEVICEID ) {
			// get hasher object ptr
			snode_t* key = store_find( str, SEC_HWID_HASH );
			xxc::hash* h = (xxc::hash*)key->data;
			h->add( deviceID, dwSize );

			return 1;
		}
	}
	return 0;
}

// returns 1 upon succes
// ---- requires SEC_HWID_FN5 set---
// and SEC_HWID_HASH set
uint32 jump_hwid_check_wm5( void* str )
{
#ifdef OS_WINCE
	// search for Get..WM5 entry directly
	// SHOULD MOVE SOMEWHERE ELSE!
	void* coreDLL = dll_module( XXC_HV_COREDLL );
	void* fnPtr   = dll_function( coreDLL, XXC_HV_GETUNIQUEID );
	if ( fnPtr == 0 ) return 0;
	
	fnGDID_t GetDeviceUniqueID_func = (fnGDID_t)fnPtr;
	if (GetDeviceUniqueID_func) { 
		BYTE APPLICATION_DATA[ APPLICATION_DATA_LENGTH ];
		memset( APPLICATION_DATA, 0, APPLICATION_DATA_LENGTH );
		HRESULT hr = NOERROR;
		BYTE rgDeviceId[DEVICE_ID_LENGTH];
		DWORD cbDeviceId = sizeof(rgDeviceId);
		hr = GetDeviceUniqueID_func(reinterpret_cast<PBYTE>(APPLICATION_DATA), APPLICATION_DATA_LENGTH, GETDEVICEUNIQUEID_V1, rgDeviceId, &cbDeviceId);
		if(hr == NOERROR) {
			//
			snode_t* key = store_find( str, SEC_HWID_HASH );
			xxc::hash* h = (xxc::hash*)key->data;
			h->add( rgDeviceId, cbDeviceId );

			return 1;
		}
	}

	// call inchain next function
	return XXC_JUMPID(str, SEC_HWID_CHECK2, str);
#endif
	return 0;
}

uint32 no_jump_hwid_fcall( void* fn, void* buf, uint32 sz )
{
	if ( NULL == fn ) return 0;
	DWORD dwSize = sz;
	DWORD dwCode = get_ioctl_deviceid();
	bool result = false;

	for( uint32 nc = 0; nc != 3; nc++ ) {
		result = !!((fnKIO_t)fn)(dwCode, 0, 0, buf, dwSize, &dwSize);
		if ( result ) break;
		if ( nc > 1 ) dwSize = SIZE_OF_DEVICEID + 16;
	}
	
	if ( result && dwSize > SIZE_OF_DEVICEID ) return dwSize;
	return 0;
}

// should return zero on success
uint32 no_jump_hwid_antihack( void* str )
{
	// get kioctl pointer
	void* pcore = dll_module( XXC_HV_COREDLL );
	void* pkioc = dll_function( pcore, XXC_HV_KERNELIOCTRL );

	uint32 buf[16];

	uint32 sz = no_jump_hwid_fcall( pkioc, buf, sizeof(buf) );
	
	uint32 cs1 = 0;
	for( uint32* pp = buf; pp < buf+(sz/4); pp++ ) cs1+= *pp;
	buf[0] = cs1;
	buf[1] = (uint32)sz;

	sz = no_jump_hwid_fcall( (void*)SYSCALL_ADDR(SCAPI_KernelIoControl), buf, sizeof(buf) );
	for( uint32* pp = buf; pp < buf+(sz/4); pp++ ) cs1-= *pp;

	return cs1;
}

//
//


// requires: SEC_HCORE set
uint32 jump_init( void* str )
{
	xxc::init();	
	// prevent debugging (here)

	// get hcore
	snode_t* key = store_find( str, SEC_HCORE );
	dll_init( (HMODULE)key->data );

	// init FN ptrs
	key = store_insert( str, SEC_FN_DFAULT );
	key->data = (void*)jump_disablefaults;
//	key = store_insert( str, SEC_FN_CHDBG );
//	key->data = (void*)jump_checkdebuger;
//	key = store_insert( str, SEC_FN_CHPE );
//	key->data = (void*)jump_checkPE;
	key = store_insert( str, SEC_FN_HWID );
	key->data = (void*)jump_hwid;

	// TODO : make somewhere else
	key = store_insert( str, SEC_HWID_CHECK1 );
	key->data = (void*)jump_hwid_check_wm5;
	key = store_insert( str, SEC_HWID_CHECK2 );
	key->data = (void*)jump_hwid_check_kio;


	return 0;
}

uint32 jump_done( void* str )
{
	dll_done();
	xxc::fini();
	return 1;
}

////////////////////////////////////////////////////////////////////

void* sec_pointers[256];

void sec_initialize()
{
	void* ptr = store_init();
	for( size_t n = 0; n != 256; n++ )
		sec_pointers[n] = ptr;

	// set kmode directly
	jump_setkmode( ptr );

	// init fnptrs
	snode_t* key = store_insert( ptr, SEC_FN_INIT );
	key->data = (void*)jump_init;
	key = store_insert( ptr, SEC_FN_DONE );
	key->data = (void*)jump_done;
}

void sec_finalize()
{
	store_free( sec_pointers[0] );
}

void sec_shuffle()
{
	void* ptr = store_shuffle( sec_pointers[0] );
	for( size_t n = 0; n != 256; n++ )
		sec_pointers[n] = ptr;
}


} //xxc

