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

// Secure random number generator
// Modified Mersene Twister
// Warn!! not checked for unaligned access

#include "stdafx.h"

#include "xxc.cfg.h"
#include "xxc.rnd.h"
#include "xxc.defs.h"
#include "xxc.bswap.h"

#define MATRIX_A 0x9908b0df   	// constant vector a
#define UPPER_MASK 0x80000000 	// most significant w-r bits 
#define LOWER_MASK 0x7fffffff 	// least significant r bits
#define MIXBITS(u,v) ( ((u) & UMASK) | ((v) & LMASK) )
#define TWIST(u,v) ((MIXBITS(u,v) >> 1) ^ ((v)&1UL ? MATRIX_A : 0UL))

namespace xxc {

void
rnd::reset( uint32 s )
{
    uint32 pstate = state[0] = s;
	for( size_t j = 1; j != NParam; j++ ) {
		state[j] = (1812433253UL * (pstate ^ (pstate >> 30)) + j); 
		pstate = state[j];
	}	
	left = 1;
	inited = true;
}

void
rnd::reset( const uint8* seed, size_t seedlen )
{
	reset( 19650218UL);

	size_t i, j, k;
	i = 1;
	j = 0;

	k = ( NParam > seedlen ? NParam : seedlen );
	for( ; k; k-- ) {
        state[i] = (state[i] ^ ((state[i-1] ^ (state[i-1] >> 30)) * 1664525UL)) + seed[j] + j;
        i++; j++;
        if (i>=NParam) { state[0] = state[NParam-1]; i=1; }
        if (j>=seedlen) j=0;
	}

	for( k = NParam-1; k; k-- ) {
        state[i] = (state[i] ^ ((state[i-1] ^ (state[i-1] >> 30)) * 1566083941UL)) - i;
        i++;
        if (i>=NParam) { state[0] = state[NParam-1]; i=1; }
	}

    state[0] = 0x80000000UL;
    left = 1; 
    inited = true;
}

void
rnd::forward()
{
    if ( !inited ) reset(5489UL);

    left = NParam;
    next = state;
#if 0
    uint32 *p = state;
	size_t j; 
    for (j=NParam-MParam+1; --j; p++) 
        *p = p[MParam] ^ TWIST(p[0], p[1]);

    for (j=MParam; --j; p++) 
        *p = p[MParam-NParam] ^ TWIST(p[0], p[1]);

    *p = p[MParam-NParam] ^ TWIST(p[0], state[0]);
#else
	uint32 y;
	static uint32 mag01[2] = {0x0, MATRIX_A};

	size_t kk;
	for (kk=0;kk<NParam-MParam;kk++) {
		y = (state[kk]&UPPER_MASK)|(state[kk+1]&LOWER_MASK);
		state[kk] = state[kk+MParam] ^ (y >> 1) ^ mag01[y & 0x1];
	}
	for (;kk<NParam-1;kk++) {
		y = (state[kk]&UPPER_MASK)|(state[kk+1]&LOWER_MASK);
		state[kk] = state[kk+(MParam-NParam)] ^ (y >> 1) ^ mag01[y & 0x1];
	}
	y = (state[NParam-1]&UPPER_MASK)|(state[0]&LOWER_MASK);
	state[NParam-1] = state[MParam-1] ^ (y >> 1) ^ mag01[y & 0x1];
#endif
}

uint32
rnd::get()
{
	if ( --left == 0 ) forward();
	uint32 val = *next++;

	val ^= XXC_ROTR32C( val, 11 );
	val ^= XXC_ROTL32C( val, 7  )  & 0x9d2c5680UL;
	val ^= XXC_ROTL32C( val, 15 )  & 0xefc60000UL;
	val ^= XXC_ROTR32C( val, 18 );
//	val ^= ( val >> 11 );
//	val ^= ( val << 7  )  & 0x9d2c5680UL;
//	val ^= ( val << 15 )  & 0xefc60000UL;
//	val ^= ( val >> 18 );


	return val;
}

void
rnd::load( const uint32 st[state_sz] )
{
	for( size_t n = 0; n != NParam; n++ ) {
		state[n] = be2me_32( st[n] );
	}
	left = be2me_32( st[NParam] );
	next = state + (NParam-left);
}

void
rnd::save( uint32 st[state_sz] )
{
	for( size_t n = 0; n != NParam; n++ ) {
		st[n] = me2be_32( state[n] );
	}
	st[NParam] = me2be_32( left );
}

} //xxc

