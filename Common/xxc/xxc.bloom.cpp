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

#if TEST_BLOOM
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cassert>
#endif


#include "xxc.cfg.h"
#include "xxc.bloom.h"

#include "xxc.wbox.h"

namespace xxc {

// BLOOM HASH
////////////////////////////////////////////////////////////
#define XXC_NUSR(a,s)	(((uint32)(a))>>(32-(s)))
#define BMASK( x ) (1UL<<(x))

//always_inline 
static size_t bloom_slice( const void* buf, size_t ndx )
{
	const uint8* p	= (const uint8*)buf;
	size_t bit_n	= XXC_BLOOM_BITS * ndx;
	// this is for hash overuse prevention - should not happen in normal cases
	// replace division !
	//bit_n %= (512-XXC_BLOOM_BITS);
	if ( bit_n >= (512-XXC_BLOOM_BITS) ) bit_n -= (512-XXC_BLOOM_BITS);
	
	uint32 boffset	= bit_n >> 3;
	uint32 bshift	= bit_n & 0x07;
	p += boffset;
	uint32 word		= (((uint32)p[0])) | (((uint32)p[1])<<8) | (((uint32)p[2])<<16) | (((uint32)p[3])<<24);
	word >>= bshift;
	//uint32 word		= unaligned32_le( p+(bit_n>>3) ) >> (bit_n&0x07);
	return word & XXC_NUSR(0xffffffff, XXC_BLOOM_BITS );
}

//always_inline 
static bool bloom_checkbit( const void* buf, size_t bitn )
{
	const uint8* p	= (const uint8*)buf;
	return *(p + (bitn>>3)) & BMASK( bitn & 7 ) ? true : false;
}

#undef BMASK

///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////

#define R(a,b) XXC_ROTL32C( a, b )

static void 
hash_transform( uint32* img, const uint32* in )
{
	// salsa12 transform
	uint32 x[16];
	size_t i;

	// was x = in
	for (i = 0;i != 16;++i) x[i] = in[i];

	for (i = 0; i != 8; i++ ) {
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
	//for (size_t n = 0;n != hash::words_sz;++n) img[n] += x[n];
	}

	// was out = x + in
	for (i = 0;i != 16;++i) {
		img[i] = x[i] + in[i];
		x[i] = i;
	}
}


/*
static const uint32 hash_boot[16] = {
	0xE97625A5UL, 0x0614D1B7UL, 0x0E25244BUL, 0x0C768347UL,
	0x589E8D82UL, 0x0D2059D1UL, 0xA466BB1EUL, 0xF8DA0A82UL,
	0x04F19130UL, 0xBA6E4EC0UL, 0x99265164UL, 0x1EE7230DUL,
	0x50B2AD80UL, 0xEAEE6801UL, 0x8DB2A283UL, 0xEA8BF59EUL
};
*/

void bloom_hash( uint32 word, uint32 res[16] )
{
	uint32 src[16];

	word ^= 0xC9AB93B0;
#if 1
	for( size_t x = 0; x != 16; x++ ) {
		//src[x] = hash_boot[x];
		src[x] = wbox_dec( e_hashBoot[x] );
	}
	src[0] += R( word, 0 );
	src[2] += R( word, 7 );
	src[4] += R( word, 13 );
	src[6] += R( word, 25 );
	hash_transform( res, src );
#else
	memcpy( res, hashiv, sizeof(hashiv) );
	memset( src, 0, sizeof(src) );
	src[0] += R( word, 0 );
	src[1] += R( word, 7 );
	src[2] += R( word, 13 );
	src[3] += R( word, 25 );
	transform2( res, src );
	src[4] += R( word, 0 );
	src[5] += R( word, 7 );
	src[6] += R( word, 13 );
	src[7] += R( word, 25 );
	transform2( res+8, src );
#endif
	// obfurscation
	src[3] = src[0] = e_hashBoot[11];
}


/// Word mix function
/// Variation of the Bob Jenkins hash
////////////////////////////////////////////

uint32 bloom_mix( uint32 a, uint32 c )
{
/*
	uint32 b = 0x27d4eb2d;

	a = (a+0x7ed55d16) + XXC_ROTL32C(a,12);
	a = (a^0xc761c23c) ^ XXC_ROTR32C(a,19);
	a = (a+0x165667b1) + XXC_ROTL32C(a,5);
	a = (a+0xd3a2646c) ^ XXC_ROTR32C(a,9);
	a = (a+0xfd7046c5) + XXC_ROTL32C(a,3);
	a = (a^0xb55a4f09) ^ XXC_ROTR32C(a,16);
*/
	uint32 b = wbox_dec( e_bloomMix[0] );

	a = (a+wbox_dec( e_bloomMix[1] )) + XXC_ROTL32C(a,12);
	a = (a^wbox_dec( e_bloomMix[2] )) ^ XXC_ROTR32C(a,19);
	a = (a+wbox_dec( e_bloomMix[3] )) + XXC_ROTL32C(a,5);
	a = (a+wbox_dec( e_bloomMix[4] )) ^ XXC_ROTR32C(a,9);
	a = (a+wbox_dec( e_bloomMix[5] )) + XXC_ROTL32C(a,3);
	a = (a^wbox_dec( e_bloomMix[6] )) ^ XXC_ROTR32C(a,16);

#if 0
	a=a-b;  a=a-c;  a=a^(c >> 13);
	b=b-c;  b=b-a;  b=b^(a << 8); 
	c=c-a;  c=c-b;  c=c^(b >> 13);
	a=a-b;  a=a-c;  a=a^(c >> 12);
	b=b-c;  b=b-a;  b=b^(a << 16);
	c=c-a;  c=c-b;  c=c^(b >> 5);
	a=a-b;  a=a-c;  a=a^(c >> 3);
	b=b-c;  b=b-a;  b=b^(a << 10);
	c=c-a;  c=c-b;  c=c^(b >> 15);
#else
	a=a-b;  a=a-c;  a=a ^ XXC_ROTR32C(c,13);
	b=b-c;  b=b-a;  b=b ^ XXC_ROTL32C(a,8); 
	c=c-a;  c=c-b;  c=c ^ XXC_ROTR32C(b,13);
	a=a-b;  a=a-c;  a=a ^ XXC_ROTR32C(c,12);
	b=b-c;  b=b-a;  b=b ^ XXC_ROTL32C(a,16);
	c=c-a;  c=c-b;  c=c ^ XXC_ROTR32C(b,5);
	a=a-b;  a=a-c;  a=a ^ XXC_ROTR32C(c,3);
	b=b-c;  b=b-a;  b=b ^ XXC_ROTL32C(a,10);
	c=c-a;  c=c-b;  c=c ^ XXC_ROTR32C(b,15);
#endif

	// obfurscation
	a -=c;	b ^=a-c;

	return c;
}

/// bloom functions
/////////////////////////////////////////////////

static always_inline uint32 
bitreverse( uint32 v )
{
	size_t s 	= 32;
	uint32 mask = ~0;
	while ((s >>= 1) != 0) {
		mask ^= (mask << s);
		v = ((v >> s) & mask) | ((v << s) & ~mask);
	}
	return v;
}

#if 0
uint32 bloom_probe( const void* buf, uint32 val )
{
	uint32 img[16];

	bloom_hash( val, img );

	// note we retrieve bits in reverse way (for security purposes ^^);
	// so after we need to reverse bit! order
	// and shift right on 32-BLOOM_PHASES bits
	// since the going to shift bits out, we can load trash in the res already
	uint32 res = img[0] ^ 0xC1A829DF;
	for( size_t phase = 0; phase != XXC_BLOOM_PHASES; phase++ ) {
		uint32 ndx = bloom_slice( img, phase );
		res = (res << 1) | (bloom_checkbit( buf, ndx ) ? 1 : 0);
	}

	// restore natural bit order of res (reverse & shift)
	//res = XXC_NUSR( bitreverse(res), XXC_BLOOM_PHASES );

	// decrypt the key value
	res ^= bloom_mix( val, img[0] ); 

	return res & XXC_BLOOM_IMASK;	
}
#else
uint32 bloom_probe( const void* buf, uint32 val )
{
	uint32 img[16];

	bloom_hash( val, img );

	uint32 res = 0;
	uint32 msk = 1 << (XXC_BLOOM_PHASES-1);
	for( size_t phase = 0; phase != XXC_BLOOM_PHASES; phase++ ) {
		uint32 ndx = bloom_slice( img, phase );
		res |= bloom_checkbit( buf, ndx ) ? msk : 0;
		msk >>= 1;
	}

	// decrypt the key value
	res ^= bloom_mix( val, img[0] ); 

	return res & XXC_BLOOM_IMASK;	
}

#endif



} //xxc

#if TEST_BLOOM

uint32 htou( const char* str )
{
	uint32 res;
	sscanf( str, "%x", &res );
	return res;
}

int 
main( int argc, char* argv[] )
{
	if ( argc < 2 ) {
		printf("usage: binfile serial\n");
		return 1;
	}

	FILE* f = fopen( argv[1], "rb" );
	if ( 0 == f ) exit(1);
	void* b = malloc( 1024*128 );
	fread( b, 1024*128, 1, f );
	fclose(f);

	const uint32  ser = htou( argv[2] );
	printf("probing %08x : %08x\n", ser, xxc::bloom_probe( b, ser ) );

	free(b);
	return 0;
}

#endif
