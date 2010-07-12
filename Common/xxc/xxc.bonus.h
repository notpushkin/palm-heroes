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
// special macros and stuff
//

#ifndef XXC_BONUS_H__
#define XXC_BONUS_H__

#include "xxc.defs.h"

// interleave in obvious way
always_inline
uint32 bits_interleave( uint32 x )
{
	// split bits 16+16
	uint32 y = x >> 16;
	x ^= (y << 16);

	x = (x | (x << 8)) & 0x00ff00ff;
	y = (y | (y << 8)) & 0x00ff00ff;
	x = (x | (x << 4)) & 0x0f0f0f0f;
	y = (y | (y << 4)) & 0x0f0f0f0f;
	x = (x | (x << 2)) & 0x33333333;
	y = (y | (y << 2)) & 0x33333333;
	x = (x | (x << 1)) & 0x55555555;
	y = (y | (y << 1)) & 0x55555555;

	return x | (y << 1);
}

// reverse order
always_inline
uint32 bits_reverse( uint32 v )
{
	v = ((v >> 1) & 0x55555555) | ((v & 0x55555555) << 1);
	v = ((v >> 2) & 0x33333333) | ((v & 0x33333333) << 2);
	v = ((v >> 4) & 0x0F0F0F0F) | ((v & 0x0F0F0F0F) << 4);
	v = ((v >> 8) & 0x00FF00FF) | ((v & 0x00FF00FF) << 8);
	v = ( v >> 16             ) | ( v               << 16);
	return v;
}

always_inline
uint32 bits_modulus65535( uint32 n )
{
	const uint32 s = 16;				// s > 0
	const uint32 d = (1 << s) - 1;	 // so d is either 1, 3, 7, 15, 31, ...).
	uint32 m;						 // n % d goes here.

	for (m = n; n > d; n = m) {
	  for (m = 0; n; n >>= s)
		m += n & d;
	}
	return m == d ? 0 : m;	
}

always_inline
uint32 bits_parity__( uint32 v )
{
	v ^= v >> 16;
	v ^= v >> 8;
	v ^= v >> 4;
	v &= 0xf;
	return (0x6996 >> v) & 1;
}


//Test if a word x contains an unsigned byte with value > n. Uses 3 arithmetic/logical operations when n is constant.
//Requirements: x>=0; 0<=n<=127
#define xxc_hasmore(x,n) (((x)+~0UL/255*(127-(n))|(x))&~0UL/255*128)
//To count the number of bytes in x that are more than n in 6 operations, use:
#define xxc_countmore(x,n) \
	(((((x)&~0UL/255*127)+~0UL/255*(127-(n))|(x))&~0UL/255*128)/128%255)


#endif //XXC_BONUS_H__
