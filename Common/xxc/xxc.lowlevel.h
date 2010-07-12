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
// Low level CE stuff
//

#ifndef XXC_LOWLEVEL_H__
#define XXC_LOWLEVEL_H__

namespace xxc {

// device-id related trash
typedef BOOL (*fnKIO_t) (DWORD, LPVOID, DWORD, LPVOID, DWORD, LPDWORD);
#define IOCTL_HAL_GET_DEVICEID 0x01010054
#define SIZE_OF_DEVICEID (5*4)

// wm5 related stuff
#ifdef OS_WINCE
#define DEVICE_ID_LENGTH            20
//#define APPLICATION_DATA            "\0\0\0\0\0\0\0\0"
#define APPLICATION_DATA_LENGTH     8
#ifndef GETDEVICEUNIQUEID_V1
#	define GETDEVICEUNIQUEID_V1 1
#endif
#endif
typedef HRESULT (*fnGDID_t)(LPBYTE,DWORD,DWORD,LPBYTE,DWORD*);



// pointer to the thread TLS
always_inline
uint32* tls_ptr_()
{
	// obfuscate constant: 0xffffc894
	// inverse is 376c
	//
	uint32 addrConst = 0x37;
	void* curThread = (void*)addrConst;
	while ( 1 ) {
		if ( addrConst < 0xff ) {
			addrConst = (addrConst<<8) | addrConst;
		} else if ( (addrConst&0x0f) < 8 ) {
			addrConst += 0x34;
		} else {
			curThread = *(void**)( ~addrConst );
			break;
		}
	}
	return (uint32*)*(void**)( (DWORD)curThread + 0x24 );
}

#define TLSSLOT_MSGQUEUE    0
#define TLSSLOT_RUNTIME     1
#define TLSSLOT_KERNEL      2
#define TLSKERN_TRYINGTODIE     0x00000001
#define TLSKERN_NOFAULT         0x00000002  // Don't break into the debugger if set
#define TLSKERN_CECAP_PROFILING 0x00000004  // CECAP is currently profiling
#define TLSKERN_IN_LOADER       0x00000008  // Holding loader critical section
#define TLSKERN_NOFAULTMSG      0x00000010  // used with TLSKERN_NOFAULT, skip the display of exception message

#define TLSSLOT_NUMRES      4    // number of reserved slots (must be in order from 0)
#define TLSSLOT_RESERVE     0x0F // bitfield - reserve low 3 slots

#define XXC_DISABLEFAULTS()		(xxc::tls_ptr_ () [TLSSLOT_KERNEL] |= (TLSKERN_NOFAULTMSG | TLSKERN_NOFAULT))
#define XXC_ENABLEFAULTS()		(xxc::tls_ptr_ () [TLSSLOT_KERNEL] &= ~(TLSKERN_NOFAULTMSG | TLSKERN_NOFAULT))


struct info {                       /* Extra information header block      */
	unsigned long   rva;            /* Virtual relative address of info    */
	unsigned long   size;           /* Size of information block           */
};

typedef struct e32_lite {           /* PE 32-bit .EXE header               */
	unsigned short  e32_objcnt;     /* Number of memory objects            */
	BYTE            e32_cevermajor; /* version of CE built for             */
	BYTE            e32_ceverminor; /* version of CE built for             */
	unsigned long   e32_stackmax;   /* Maximum stack size                  */
	unsigned long   e32_vbase;      /* Virtual base address of module      */
	unsigned long   e32_vsize;      /* Virtual size of the entire image    */
	unsigned long	e32_sect14rva;  /* section 14 rva */
	unsigned long	e32_sect14size; /* section 14 size */
	unsigned long   e32_timestamp;  /* Time EXE/DLL was created/modified   */
	struct info     e32_unit[7]; /* Array of extra info units     */
} e32_lite, *LPe32_list;

//////////////////////



} // xxc

#endif //XXC_LOWLEVEL_H__
