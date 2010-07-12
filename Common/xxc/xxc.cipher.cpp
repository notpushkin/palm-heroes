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
// modified Salsa20 stream cipher
//

#include "stdafx.h"

#include "xxc.cfg.h"
#include "xxc.defs.h"
#include "xxc.bswap.h"

#include "xxc.cipher.h"

#include "xxc.wbox.h"

namespace xxc {

#define R( v, n ) XXC_ROTL32C( v, n )
#ifndef min
#define min( a, b ) (((a)<(b))?(a):(b))
#endif

// salsa12 transform
// in & out should be word_t aligned!
static void 
salsa12( cipher::word_t out[16], const cipher::word_t state[16] )
{
	cipher::word_t x[16];
	size_t i;

	// was x = in
	for (i = 0;i != 16;++i) x[i] = state[i];

	// unrolled twice
	for (i = 0; i != 6; i++ ) {
		x[ 4] ^= R(x[ 0]+x[12], 7);  x[ 8] ^= R(x[ 4]+x[ 0], 9);
		x[12] ^= R(x[ 8]+x[ 4],13);  x[ 0] ^= R(x[12]+x[ 8],18);
		x[ 9] ^= R(x[ 5]+x[ 1], 7);  x[13] ^= R(x[ 9]+x[ 5], 9);
		x[ 1] ^= R(x[13]+x[ 9],13);  x[ 5] ^= R(x[ 1]+x[13],18);
		x[14] ^= R(x[10]+x[ 6], 7);  x[ 2] ^= R(x[14]+x[10], 9);
		x[ 6] ^= R(x[ 2]+x[14],13);  x[10] ^= R(x[ 6]+x[ 2],18);
		x[ 3] ^= R(x[15]+x[11], 7);  x[ 7] ^= R(x[ 3]+x[15], 9);
		x[11] ^= R(x[ 7]+x[ 3],13);  x[15] ^= R(x[11]+x[ 7],18);
		x[ 1] ^= R(x[ 0]+x[ 3], 7);  x[ 2] ^= R(x[ 1]+x[ 0], 9);
		x[ 3] ^= R(x[ 2]+x[ 1],13);  x[ 0] ^= R(x[ 3]+x[ 2],18);
		x[ 6] ^= R(x[ 5]+x[ 4], 7);  x[ 7] ^= R(x[ 6]+x[ 5], 9);
		x[ 4] ^= R(x[ 7]+x[ 6],13);  x[ 5] ^= R(x[ 4]+x[ 7],18);
		x[11] ^= R(x[10]+x[ 9], 7);  x[ 8] ^= R(x[11]+x[10], 9);
		x[ 9] ^= R(x[ 8]+x[11],13);  x[10] ^= R(x[ 9]+x[ 8],18);
		x[12] ^= R(x[15]+x[14], 7);  x[13] ^= R(x[12]+x[15], 9);
		x[14] ^= R(x[13]+x[12],13);  x[15] ^= R(x[14]+x[13],18);
	}

	// output
	for (i = 0;i != 16; ++i) {
		out[i] = me2le_32( x[i] + state[i] );
	}
}


// keyset
// destination must be word-aligned
// copies bytes from the 'bin' to 'key' and zero pads to 'keysize'
// keysize >= binsize
void keyset( cipher::word_t* key, size_t keysize, const void* bin, size_t binsize )
{
	check( keysize >= binsize );
	size_t tocopy = min( binsize, keysize );
	copybytes( key, bin, tocopy );
	wipeout( ((uint8*)key) + tocopy, binsize - keysize );

}

// 128bit key variant
void
cipher::keysetup( const word_t key[4] )
{
	// key
	state[ 1] = key[0];
	state[ 2] = key[1];
	state[ 3] = key[2];
	state[ 4] = key[3];
	// key part two 
	state[11] = key[0];
	state[12] = key[1];
	state[13] = key[2];
	state[14] = key[3];
	// sigma
//	state[ 0] = 0xFAF7281A;
//	state[ 5] = 0x4A2C3773;
//	state[10] = 0x95CFF1BD;
//	state[15] = 0x1B4B9D23;
	// REPLACED with W-BOXED constants
	state[ 0] = wbox_dec( e_cipherKey[0] );
	state[ 5] = wbox_dec( e_cipherKey[1] );
	state[10] = wbox_dec( e_cipherKey[2] );
	state[15] = wbox_dec( e_cipherKey[3] );
	// iv  & counter
	state[ 6] = 0x9BCEF411;
	state[ 7] = 0xE091B719;
	state[ 8] = 0;
	state[ 9] = 0;
}

void 
cipher::ivsetup( const word_t iv0, const word_t iv1 )
{
	// iv defaults in be: 9BCEF411 E091B719
	state[ 6] = iv0;
	state[ 7] = iv1;
}

void
cipher::resync( size_t pos )
{
	state[8] = (word_t)(pos >> 6);
	state[9] = 0;
	size_t kpos = pos & 63;
	// refresh keystream
	salsa12( kstream, state );
	keyleft = 64 - kpos;
	if ( 0 == ++state[8] ) state[9]++;
}

void
cipher::clear()
{
	wipeout( kstream, sizeof(kstream) );
	wipeout( state, sizeof(state) );
	keyleft = 0;
}

void
cipher::reset( const word_t key[4] )
{
	keysetup( key );
	keyleft = 0;
}

cipher::cipher()
{ clear(); }

cipher::~cipher()
{ clear(); }

//
// processing should be optimized with respect to alignment
// could be done as:
// 1) align keystream (process keyleft bytes)
// 2) process number of whole blocks as 
// 2.1) read unaligned into array
// 2.2) word-key array
// 2.3) memcopy (unaligned) to the source
// 3) process the rest of bytes < block size
void
cipher::process( uint8* msg, size_t bytes )
{
	
	while ( bytes != 0 ) {
		// generate keystream if required
		if ( keyleft == 0 ) {
			salsa12( kstream, state );
			if ( 0 == ++state[8] ) state[9]++;
			keyleft = 64;
		}

		// calculate maximum block size based on keys left and input block
		size_t block = min( keyleft, min( bytes, 64 ) );
		check( block <= 64 );
		check( block <= keyleft );
		check( keyleft != 0 );
		const uint8* kptr = ((const uint8*)kstream) + (64-keyleft);
		// duffs device - 16 bytes at once 
		size_t tcnt = (block+15)>>4;
		//printf("%08x [%08x:%08x] : k=%04d b=%04d t=%04d\n", msg, state[9], state[8], keyleft, block, tcnt );
		//for( size_t z = 0; z != 32; z++ ) printf("%02x",kptr[z] );
		switch( block & 15 ) {
			case  0 : do {	*msg++ ^= *kptr++;
			case 15 : 		*msg++ ^= *kptr++;
			case 14 : 		*msg++ ^= *kptr++;
			case 13 : 		*msg++ ^= *kptr++;
			case 12 : 		*msg++ ^= *kptr++;
			case 11 : 		*msg++ ^= *kptr++;
			case 10 : 		*msg++ ^= *kptr++;
			case  9 : 		*msg++ ^= *kptr++;
			case  8 : 		*msg++ ^= *kptr++;
			case  7 : 		*msg++ ^= *kptr++;
			case  6 : 		*msg++ ^= *kptr++;
			case  5 : 		*msg++ ^= *kptr++;
			case  4 : 		*msg++ ^= *kptr++;
			case  3 : 		*msg++ ^= *kptr++;
			case  2 : 		*msg++ ^= *kptr++;
			case  1 : 		*msg++ ^= *kptr++;
					} while ( --tcnt != 0 );
		};

		bytes  -= block;
		keyleft-= block;
	}	
}

} // xxc

#ifdef TESTCIPHER

#define TEST( x ) assert((x))

int main()
{
	uint32 key[4] = { 1, 2, 3, 4 };
	const uint8  msg[256] = "WARNING: the conversions defined below are implemented as macros, and should be used carefully. They should NOT be used with parameters which perform some action. E.g., the following two lines are not equivalent: *** Please do not edit this file. *** 1234";
	uint8 dst1[256];
	uint8 dst2[256];

	TEST( sizeof(dst1) == sizeof(msg) );
	
	xxc::cipher c;

	// check equivalency the basic crypt / decrypt
	memcpy( dst1, msg, sizeof(msg) );
	c.reset( key );
	c.process( dst1, sizeof(dst1) );             
	TEST( 0 != memcmp( msg, dst1, sizeof(dst1) ) );
	c.reset( key );
	memcpy( dst2, dst1, sizeof(dst1) );
	c.process( dst2, sizeof(dst2) );             
	TEST( 0 == memcmp( msg, dst2, sizeof(dst2) ) );
	                                       
	// check block encoding modes          
	for( size_t bs = 1; bs != 128; bs++ ) {
		c.reset( key );
		memcpy( dst2, msg, sizeof(msg) );
		uint8* ptr = dst2;
		size_t length = sizeof(msg);
		while( length != 0 ) {
			size_t blk = min( bs, length );
			c.process( ptr, blk );
			ptr += blk;
			length -= blk;
		}
		TEST( 0 == memcmp( dst2, dst1, sizeof(dst1) ) );
	}

	// print coded text for visual confirmation
	for( size_t nn = 0; nn != 15; nn++ ) {
		key[0] = nn;
		c.reset( key );
		memcpy( dst1, msg, sizeof(msg) );
		c.process( dst1, sizeof(dst1) );
		const uint32* p = (const uint32*)dst1;
		size_t m;
		for( m = 0; m != 10; m++ ) {
			printf("%08x", p[m] );
			if ( m == 9 ) printf("\n");
		}
		/*
		c.resync( 0 );
		c.process( dst1, sizeof(dst1) );
		for( m = 0; m != 10; m++ ) {
			printf("%08x", p[m] );
			if ( m == 9 ) printf("\n");
		}*/
	}

	printf("done!");

	return 0;
}

#endif // TESTCIPHER
