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
// various crack detection techniques
//

#ifndef WCE_DETECTS_H__
#define WCE_DETECTS_H__

namespace xxc {

// 
// return 0 if no debugger presents, otherwise !0 value
static always_inline
uint32 check_debugger()
{
	uint32 part = 0x189c020;
	// obfuscate 0xffffc800 + 0x0090
	for( uint32 n = 0; n != 4; n++ ) {
		switch(n) {
			case 0 : part = (part << 8) | 0xff; break;
			case 2 : part = (part << 8) | 0xc8; break;
			case 3 : part = (part << 8) | 0x90; break;
			case 1 : part = (part << 8) | 0xff; break;
		}
	}
	// pick the value at
	void* pptr = *((void**)(part));
	return *(uint32*)( (char*)pptr + 0x1c );
}

//
// returns number of sections in PE file
#if 0
static always_inline
uint32 check_sections_pe()
{
	// TODO: replace inline constant with something 
	// non-null as immediate val
	uint32 sysaddr = 0xda012;
	// obfuscate 0xffffc800 + 0x0090
	uint32 n = sysaddr;
	while ( n ) {
		// x -> 2 -> 1 -> 3 -> 4 -> ret
		switch(n) {
			case 1:		sysaddr |= 0x0890; 
						n+=2; 	continue;
			case 4:	{	// final destination
						// pick the value at
						//sysaddr = 0xffffc800+0x090;
						void* pptr = *((void**)(sysaddr));
						void* oeptr = (void*)( (char*)pptr + 0x50 + 0x10 );
						// first uint16 is objcount
						return *((uint16*)oeptr);
					}
			case 2:		sysaddr <<= 16; 
						n--;  	break;
			case 3:		sysaddr ^= 0xc000; 
						n ++; // passthrough
			case 0:		//n ^= n; 
						continue;
			default:	// does sysaddr = 0xffff ffff
						sysaddr *= 3;
						// fn preserves zero / non-zero attr
						sysaddr = (sysaddr) ^ (sysaddr<<1);
						// turns any value into "-1"
						sysaddr = sysaddr ^ (~sysaddr);
						n = 2; break;
		}
	}
	// never returns from here
	return sysaddr + n;
}
#else
static always_inline
uint32 check_sections_pe()
{
	// TODO: replace inline constant with something 
	// non-null as immediate val
	uint32 sysaddr = 0xda012;
	// obfuscate 0xffffc800 + 0x0090
	uint32 n = sysaddr;
	while ( n ) {
		// x -> 2 -> 1 -> 3 -> 4 -> ret
		switch(n) {
			case 1:		sysaddr |= 0x0890; 
						n+=2; 	continue;
			case 4:	{	// final destination
						// pick the value at
						//sysaddr = 0xffffc800+0x090;
						void* pptr = *((void**)(sysaddr));
						void* oeptr = (void*)( (char*)pptr + 0x50 + 0x10 );
						// first uint16 is objcount
						return *((uint16*)oeptr);
					}
			case 2:		sysaddr <<= 16; 
						n--;  	break;
			case 3:		sysaddr ^= 0xc000; 
						n ++; // passthrough
			case 0:		//n ^= n; 
						continue;
			default:	// does sysaddr = 0xffff ffff
						sysaddr *= 3;
						// fn preserves zero / non-zero attr
						sysaddr = (sysaddr) ^ (sysaddr<<1);
						// turns any value into "-1"
						sysaddr = sysaddr ^ (~sysaddr);
						n = 2; break;
		}
	}
	// never returns from here
	return sysaddr + n;
}
#endif

//
// tries to detect an OEP (as from start exe address)
// by walking a stack downwards and searchig magic's
static always_inline
uint32 check_oep( uint32 dummy  )
{
	// find stack top
	const uint32* ptop = (&dummy) + 4;
	// protect from scanning excessive part
	//ptop = (const uint32*)( (((uint32)ptop)&~0xfff)|0xe00);
	ptop += 0x20;
	// scan till next 64k boundary (hope this is enough)
	const uint32  pend = (((uint32)ptop) >> 16)+1; 
	// scan up in obfuscated way
	dummy |= 0xf0000000; 
	// enter with 0 val
	while ( ptop ) {
		dummy+=0x10000000;
		switch ( (dummy >> 28) ) {
			case 2:
				// check if matches ce5 mark 0xc203fe7c
				// if so - go 1, else 3
				if ( (((ptop[1] + 0x1183)<<16)>>16) == 0x0fff) dummy += 0xe0000000; 
				//dummy += 0x20000000; 
				break;
			case 0:
				// check if matches ce4 mark 0xf000fe3c
				// if so - goto 2, else goto 'default'>4
				{
					// swap bits = 0x000ffe3c
					uint32 v = *ptop;
					uint32 x = ((v >> 28) ^ (v >> 16)) & ((1 << 4) - 1);
					uint32 r = v ^ ((x << 28) | (x << 16));
					// append and rotate = 0x00fff
					r = (r + 0xc4);
					r = (r>>8)|(r<<24);
					// test
					if ( r != 0x0fff ) dummy += 0x30000000; 
				}
				dummy += 0x10000000; 
				continue;
			case 3:
				// check if ce4EOP found
				if ( ((ptop[-4] >> 20)<3) && (ptop[-4]>>16) ) { ptop = (const uint32*)(ptop[-4] - 0x10000); goto goout; }
				// not really
				dummy |= 0x80000000; 
				break;

			case 1:
				// check if ce5EOP found
				if ( ((ptop[-24] >> 20)<3) && ((ptop[-24]>>16) ) ) { ptop = (const uint32*)(ptop[-24] - 0x10000); goto goout; }
				dummy |= 0x70000000;
				break;
			case 4:
				// fake
			default:
				ptop++;
				dummy |= 0xf0000000; 
		}
		//uint32 ptopu = (uint32)ptop;
		// if = next 64k, exit cycle - failed to find
		if ( (((uint32)ptop)>>16) == pend ) ptop= 0;//ptop ^= ptop;
	}
goout: // hacky hack!
	return (uint32)ptop;
}

} //xxc

#endif //WCE_DETECTS_H__
