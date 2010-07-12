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
#include "xxc.dllfn.h"

namespace xxc {

// offset of the base-ptr from the module origin
// depends on OS version
static size_t	baseOffset;

// placeholder for encryption (if any)
#define NAME_GETVERSIONEXW	L"GetVersionExW"
#define NAME_SETKMODE		L"SetKMode"
// loader function, same as GetProcAddress
// might be used as a substitute in case we want to xor the names
void* getdllfunc( HMODULE dll, const wchar_t* name )
{
#ifdef OS_WINCE
	return (void*)GetProcAddress( dll, name );
#else
	return 0;
#endif
}
//////////////////////////////////////////


bool  dll_init( HMODULE hCoreDLL )
{
	// determine OS version
	OSVERSIONINFO versionInfo;
	versionInfo.dwOSVersionInfoSize = sizeof(versionInfo);
	
	//
	BOOL (WINAPI* func_GetVersionEx)( LPOSVERSIONINFOW ) =
		(BOOL (WINAPI*)( LPOSVERSIONINFOW ))getdllfunc( hCoreDLL, NAME_GETVERSIONEXW );

	if ( NULL == func_GetVersionEx ) return false;
		
	func_GetVersionEx( &versionInfo );

	if ( versionInfo.dwMajorVersion >= 5 ) {
		baseOffset = 0x78;
	} else {
		baseOffset = 0x7c;
	}

	BOOL (WINAPI* func_SetKMode)(BOOL) = 
			(BOOL (WINAPI*)(BOOL))getdllfunc( hCoreDLL, NAME_SETKMODE );
	// initiate kernel mode
	if ( func_SetKMode ) {
		BOOL pm1 = func_SetKMode( TRUE );
		if ( pm1 ) return true;
		BOOL pm2 = func_SetKMode( TRUE );
		return pm2 ? true : false;
	}

	return false;
}

void dll_done()
{
	baseOffset = 0;
}

// Hashing the names
//////////////////////////////////////////////////////////////////

//#define always_inline

always_inline uint32 weird_upcase( uint32 ch )
{
	return ch & ~(0x20);		
}

always_inline uint32 hashname( const char* ptr, uint32 seed = 0x1234abcd )
{
	uint32 h = seed;
	char ch;
	while ( 0 != (ch = *ptr++) ) {
		h ^= ( (h << 5) + weird_upcase(ch) + (h >> 2) );
	}
	return h;
}

always_inline uint32 hashname( const wchar_t* ptr, uint32 seed = 0x1234abcd )
{
	uint32 h = seed;
	wchar_t ch;
	while ( 0 != (ch = *ptr++) ) {
		h ^= ( (h << 5) + (weird_upcase(ch)&0xff) + (h >> 2) );
	}
	return h;
}

//
//////////////////////////////////////////////////////////////////


template< typename T, typename D >
T* get_entry( void* base, D rva, size_t ndx )
{
	void* ptr = (char*)base + (size_t)rva;
	T* item = (T*)ptr + ndx;
	return item;
}

template<typename T>
T get_struct_item( void* base, uint32 struct_rva, size_t ofs )
{
	void* ptr = (char*)base + (size_t)struct_rva + ofs;
	return *(T*)ptr;
}

//
//////////////////////////////////////////////////////////////////

//const size_t addrKDataStruct = 0xffffc800;
//const size_t addrModuleOffset= 0x324;

void* get_kstruct_addr()
{
	static uint32 dummy_unoptimize_ = 0;
	// calculate address on the fly 
	// here 0xffffc800 + 0x324
	uint32 val = dummy_unoptimize_;
	val = (~val)<<8;
	val |= 0xc8;
	val += 0x04;
	val <<= 8;
	val -= 0xdc;		
	return (void*)val;
}

void* dll_module( uint32 hash )
{
#ifndef XXC_CFG_NO_SEH
	__try {
#endif
		// pick up the ptr to the begining of the chain
		// from the ArmHigh structure
		void* pmodule = *( (void**)( get_kstruct_addr() ) );
		// walk through the list
		while ( pmodule ) {
			void* modName = *((void**)((char*)pmodule + 0x08 ));
			// check name
			if ( hashname( (const wchar_t*)modName ) == hash ) {
				return pmodule;
			}
			// get next
			pmodule = *((void**)((char*)pmodule + 0x04 ));
		}
#ifndef XXC_CFG_NO_SEH
	} __except ( 1 )  {
		return NULL;
	}
#endif
	return NULL;
}

template< typename T,typename D >
T rva2addr( void* base, D rva )
{
	return (T)( (char*)base + (uint32)rva );
}

void* dll_function( void* pmodule, uint32 hash )
{
#ifndef XXC_CFG_NO_SEH
	__try {
#endif
		//  find out all important values of the module
		void*	modBase = *((void**)((char*)pmodule + baseOffset ));
		uint32	expRVA  = get_struct_item<uint32>( pmodule, 0, 0x8c );
		//
		uint32	numNames= get_struct_item<uint32>( modBase, expRVA, 0x18 );
		void*	names	= get_struct_item<void*>( modBase, expRVA, 0x20 );
		void*	ordinals= get_struct_item<void*>(modBase, expRVA, 0x24 );
		void* 	rva		= get_struct_item<void*>(modBase, expRVA, 0x1C );
		
		for( size_t n = 0; n != numNames; n++ ) {
			char*	name	= *get_entry<char*>( modBase, names, n );
			short	ordinal	= *get_entry<short>( modBase, ordinals, n );
			void*	function= *get_entry<void*>( modBase, rva, ordinal );
			
			name		= (char*)name + (size_t)modBase;
			function	= (char*)function + (size_t)modBase;

			if ( hashname( name ) == hash ) {
				return function;
				//return rva2addr<void*>( modBase,  rva[ordinals[n]] );
			}
		}
#ifndef XXC_CFG_NO_SEH
	} __except( 1 ) {
		return NULL;
	}
#endif
	return NULL;
}

void* dll_funclist( void* pmodule, uint32* hashes, void** func )
{
#ifndef XXC_CFG_NO_SEH
	__try {
#endif
		//  find out all important values of the module
		void*	modBase = *((void**)((char*)pmodule + baseOffset ));
		uint32	expRVA  = get_struct_item<uint32>( pmodule, 0, 0x8c );
		//
		uint32	numNames= get_struct_item<uint32>( modBase, expRVA, 0x18 );
		void*	names	= get_struct_item<void*>( modBase, expRVA, 0x20 );
		void*	ordinals= get_struct_item<void*>(modBase, expRVA, 0x24 );
		void* 	rva		= get_struct_item<void*>(modBase, expRVA, 0x1C );
		
		for( size_t n = 0; n != numNames; n++ ) {
			char*	name	= *get_entry<char*>( modBase, names, n );
			short	ordinal	= *get_entry<short>( modBase, ordinals, n );
			void*	function= *get_entry<void*>( modBase, rva, ordinal );
			
			name		= (char*)name + (size_t)modBase;
			function	= (char*)function + (size_t)modBase;

			uint32  nhash = hashname( name );
			uint32* pfhash= hashes;
			uint32* paddr = (uint32*)func;
			while ( *pfhash != 0 ) {
				if ( (!*paddr) && (nhash == *pfhash) ) {
					*paddr = (uint32)function;
				}
				pfhash++;
				paddr++;
			}
		}
#ifndef XXC_CFG_NO_SEH
	} __except( 1 ) {
		return NULL;
	}
#endif
	return NULL;
}

} //xxc

