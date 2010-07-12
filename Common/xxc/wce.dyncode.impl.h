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
// ARM dynamically created code snipnets
// used to create small piece of code in memory and to be called after
//

#ifndef WCE_DYNCODE_IMPL_H__
#define WCE_DYNCODE_IMPL_H__

namespace xxc {

// creates direct jump fuction
static always_inline 
void cgen_make_jump( void* ptr, void* addr )
{
	BYTE* b = (BYTE*)ptr;
	// 0xe51ff004 - ldr pc, [pc,#04]
	*b++ = 0x04; *b++ = 0xf0; *b++ = 0x1f; *b++ = 0xe5;
	// address
	DWORD* dw = (DWORD*)b;
	*dw = (DWORD)addr;
}

// creates xored jump function, i.e prevents direct exposure
static always_inline
void cgen_make_xjump( void* ptr, void* addr )
{
	BYTE* b = (BYTE*)ptr;
	// 0xe59fa004
	*b++ = 0x04; *b++ = 0xa0; *b++ = 0x9f; *b++ = 0xe5;
	// 0xe1e0a00a
	*b++ = 0x0a; *b++ = 0xa0; *b++ = 0xe0; *b++ = 0xe1;
	// 0xe1a0f00a
	*b++ = 0x0a; *b++ = 0xf0; *b++ = 0xa0; *b++ = 0xe1;
	// addr
	DWORD* dw = (DWORD*)b;
	*dw = ~(DWORD)addr;
}


typedef void (WINAPI *dyn_rebootfn)( );
// create soft/hard reboot routine
static always_inline
void cgen_make_reboot( void* ptr )
{
	BYTE* b = (BYTE*)ptr;
	// 0xE59F0014;
	*b++ = 0x14; *b++ = 0x00; *b++ = 0x9f; *b++ = 0xe5;
	// 0xE59F4014;
	*b++ = 0x14; *b++ = 0x40; *b++ = 0x9f; *b++ = 0xe5;
	// 0xE3A01000;
	*b++ = 0x00; *b++ = 0x10; *b++ = 0xa0; *b++ = 0xe3;
	// 0xE3A02000;
	*b++ = 0x00; *b++ = 0x20; *b++ = 0xa0; *b++ = 0xe3;
	// 0xE3A03000;
	*b++ = 0x00; *b++ = 0x30; *b++ = 0xa0; *b++ = 0xe3;
	// 0xE1A0E00F;
	*b++ = 0x0f; *b++ = 0xe0; *b++ = 0xa0; *b++ = 0xe1;
	// 0xE1A0F004;
	*b++ = 0x04; *b++ = 0xf0; *b++ = 0xa0; *b++ = 0xe1;
	// 0x0101003C;
	*b++ = 0x3c; *b++ = 0x00; *b++ = 0x01; *b++ = 0x01;
	// 0xF000FE74;
	*b++ = 0x74; *b++ = 0xfe; *b++ = 0x00; *b++ = 0xf0;
}


typedef UINT (WINAPI *dyn_crcfn)( UINT seed, void* ptr, UINT nwords );
// calcuates a crc of the given block
static always_inline
void cgen_make_crc( void* ptr )
{
	BYTE* b = (BYTE*)ptr;
	// 0xe4913004;
	*b++ = 0x04; *b++ = 0x30; *b++ = 0x91; *b++ = 0xe4;
	// 0xe2522001;
	*b++ = 0x01; *b++ = 0x20; *b++ = 0x52; *b++ = 0xe2;
	// 0xe0233002;
	*b++ = 0x02; *b++ = 0x30; *b++ = 0x23; *b++ = 0xe0;
	// 0xe08002e3
	*b++ = 0xe3; *b++ = 0x02; *b++ = 0x80; *b++ = 0xe0;
	// 0xe0830e60
	*b++ = 0x60; *b++ = 0x0e; *b++ = 0x83; *b++ = 0xe0;
	// 0x1afffff9
	*b++ = 0xf9; *b++ = 0xff; *b++ = 0xff; *b++ = 0x1a;
	// 0xe1a0f00e
	*b++ = 0x0e; *b++ = 0xf0; *b++ = 0xa0; *b++ = 0xe1;
}

typedef void (WINAPI* dyn_bptfn)( int dummy );
// clear hwbkpt
static always_inline
void cgen_make_bpt( void* ptr )
{
	BYTE* b = (BYTE*)ptr;
	// e1a0500e
	*b++ = 0x0e; *b++ = 0x50; *b++ = 0xa0; *b++ = 0xe1;
	// ee1a0e10
	*b++ = 0x10; *b++ = 0x0e; *b++ = 0x1a; *b++ = 0xee;
	// e3c00102
	*b++ = 0x02; *b++ = 0x01; *b++ = 0xc0; *b++ = 0xe3;
	// ee0a0e10
	*b++ = 0x10; *b++ = 0x0e; *b++ = 0x0a; *b++ = 0xee;
	// ee1e0f14
	*b++ = 0x14; *b++ = 0x0f; *b++ = 0x1e; *b++ = 0xee;
	// e3c0000f
	*b++ = 0x0f; *b++ = 0x00; *b++ = 0xc0; *b++ = 0xe3;
	// ee0e0f14
	*b++ = 0x14; *b++ = 0x0f; *b++ = 0x0e; *b++ = 0xee;
	// e1a0f005
	*b++ = 0x05; *b++ = 0xf0; *b++ = 0xa0; *b++ = 0xe1;
}

} //xxc

#endif //WCE_DYNCODE_IMPL_H__
