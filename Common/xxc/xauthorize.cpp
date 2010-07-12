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

// authorization code generator
//
// given:
// - elliptic curve parameters - fixed
// - private key - from file
// - hwid
// - serial
// - random seed
// produces:
// - authorization code
//

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cassert>
#include <ctime>

#include "xxc.cfg.h"
#include "xxc.bswap.h"
#include "xxc.ecp.h"
#include "xxc.shash.h"
#include "xxc.defs.h"
#include "xxc.bloom.h"
#include "xxc.rnd.h"

typedef uint32 word_t ;

/// elliptic curve parameters
/// 1. global parameters are defined in xxc.params.h
/// 2. G (generator point) and n(order) is defined here
///	should be the same as in xxc.app.h

#ifdef ECC60
xxc::limb_t GPx[] = {5U, 0x0324U, 0x0e80U, 0x0272U, 0x01e1U, 0x061fU };
xxc::limb_t GPy[] =	{5U, 0x0aaeU, 0x007cU, 0x0a3aU, 0x0449U, 0x0511U };

xxc::limb_t ECn[] = {4U, 0x26cdU, 0x0a48U, 0x0668U, 0x0001U};
#else
xxc::limb_t GPx[] = {7U, 0x00e5U, 0x011bU, 0x01dcU, 0x001fU, 0x0168U, 0x0055U, 0x003cU };
xxc::limb_t GPy[] = {7U, 0x00a5U, 0x01eaU, 0x0134U, 0x0124U, 0x0010U, 0x01f8U, 0x00cfU };

xxc::limb_t ECn[] = {4U, 0xad67U, 0x3ccbU, 0x3c3cU, 0x003cU };
#endif


// generates RAND_SIZE words from (seed||cnt) to 'img'
#define RAND_SIZE (xxc::hash::words_sz)
void rand_seq( const uint8* seed, size_t seedlen, word_t cnt, word_t* img )
{
	xxc::hash hf;

	// calculate img = HASH( seed || cnt )
	hf.reset();
	hf.add( seed, seedlen );
	word_t be_cnt = me2be_32( cnt );
	hf.add( &be_cnt, sizeof(be_cnt) );
	hf.finalize( img );
}

void rand_big( xxc::rnd& rnd, xxc::big& val )
{
	size_t nw;
	for( nw = 1; nw <= xxc::limbs_n; nw++ ) {
		uint32 t = rnd.get();
		t ^= XXC_ROTR32C( t, 17);
		t ^= XXC_ROTL32C( t, 5 );
		val[nw] = (xxc::limb_t)t;
	}
	// set correct word length
	for( nw = xxc::limbs_n; nw != 0; nw-- ) {
		if ( val[nw] ) break;
	}
	val[0] = nw;
}


//
//
// the key is the source (seed)
void make_private( const uint8* key, size_t keylen, xxc::big& pkey )
{
	word_t  randval[RAND_SIZE];

	size_t cnt = 0;
	// 1. gen rand
	rand_seq( key, keylen, cnt, randval );

	// 2. fill pkey with rand words
	size_t nw;
	size_t nr = 0;
	for( nw = 1; nw <= xxc::limbs_n; nw++ ) {
		pkey[nw] = randval[nr++];
		// calculate next randval
		if ( ++nr == RAND_SIZE ) {
			cnt++;
			rand_seq( key, keylen, cnt, randval );
			nr = 0;				
		}
	}
	// 3. set correct length
	for( nw = xxc::limbs_n; nw != 0; nw-- ) {
		if ( pkey[nw] ) break;
	}
	pkey[0] = nw;
	
	// 4. calculate modulus (n)
	xxc::big modulus;
	xxc::bin_set( modulus, ECn );
	xxc::mod( pkey, modulus );

	// 5. check pkey > 0
	if ( pkey[0] == 0 ) {
		pkey[0] = 1;	
		pkey[1] = 1;
	}
}

void make_private( xxc::rnd& rnd, xxc::big& pkey )
{
	do {
		rand_big( rnd, pkey );
	} while ( xxc::numbits(pkey) < 32 );
}


//
//
// make public key
void make_public( const xxc::big& pkey, xxc::point_t& pubk )
{
	// init G point
	xxc::point_t Gpt;
	xxc::bin_set( Gpt.x, GPx );
	xxc::bin_set( Gpt.y, GPy );
	// multiply
	xxc::mul( Gpt, pkey );
	// copy result
	xxc::copy( pubk, Gpt );	
}


void hash_big( xxc::hash& hf, const xxc::big& b )
{
	// check - relies on what big::word_t is uint16
	// including length limb
	for( size_t n = 0; n <= b[0]; n++ ) {
		uint16 be_short = me2be_16( b[n] );
		hf.add( &be_short, 2 );
	}
}

void hash2big( const word_t img[8], xxc::big& b )
{
	size_t n;
	b.clear();
	for( n = 0; n != 16; n++ ) {
		uint32 iw = img[n>>1];
		uint16 is = (uint16)(iw >> (16*(n&1)));
		b[1+(n % xxc::limbs_n)] ^= is;
	}
	for( n = xxc::limbs_n; n != 0; n-- ) {
		if ( b[n] ) break;
	}
	b[0] = n;
}

#define NEG_USR32(a,s) (((uint32)(a))>>(32-(s)))
void shl_u32( xxc::big& b, uint32 w, size_t bits )
{
	check( bits <= 32 );

	w &= NEG_USR32( 0xffffffff, bits );

	if ( b[0] == 0 ) {
		b[1] = b[2] = 0;
	} else {
		size_t n = bits;
		while ( n > 16 ) {
			xxc::shl( b, 16 );
			n -= 16;
		}
		xxc::shl( b, n );
	}

	xxc::limb_t* p = (xxc::limb_t*)(b.begin()+1);
	while ( w != 0 ) {
		*p++ |= (xxc::limb_t)w;
		w >>= 16;
	}
	if ( b[0] == 0 ) 
		b[0] = (p - b.begin()) -  1;
}

uint32 shr_u32( xxc::big& b, size_t bits )
{
	check( bits <= 32 );
	size_t nbits = xxc::numbits( b );
	if ( nbits > bits ) nbits = bits;
	uint32 res = 0;
	if (nbits != 0) {
		res = unaligned32_le(b.begin()+1) & NEG_USR32( 0xffffffff, nbits );
		while ( nbits > 16 ) {
			xxc::shr( b, 16 );
			nbits -= 16;
		}
		xxc::shr( b, nbits );
	}
	return res;
}

#ifndef ECC63
#define ssmrK 63
#define ssmrK1 27
#define ssmrK2 27
#else
#define ssmrK 60
#define ssmrK1 24
#define ssmrK2 24
#endif

uint32 ___hashF1( uint32 v )
{ 
	v = xxc::bloom_mix( v, 0xCAFEBABE ); 
	return v & NEG_USR32( 0xffffffff, ssmrK1 );
}

uint32 hashF1( uint32 a, uint32 b )
{
//	uint32 v = xxc::bloom_mix( a, b ); 
	uint32 img[8];
	uint32 arg[4];

	arg[0] = me2be_32( 0xC942D947 );
	arg[1] = me2be_32( a );
	arg[2] = me2be_32( a ^ b );
	arg[3] = me2be_32( a + b );

	xxc::hash h;
	h.reset();
	h.add( arg, sizeof(arg) );
	h.finalize( img );

	arg[0] = xxc::four2one( img );

	return arg[0] & NEG_USR32( 0xffffffff, ssmrK1 );
}

uint32 hashF2( uint32 v )
{ 
	v = xxc::bloom_mix( 0xDEADBEEF, v ); 
	return v & NEG_USR32( 0xffffffff, ssmrK2 );
}


// SigningWithMessageRecovery - {digest + msg}
// 2nd variant with concatenated unrecoverable message part
// encoding takes 
// mr{ssmrK2 bits} - secret message
// msg,mlen - message to sign
// produces: {r,s} pair where r{M} and s{n}+1 bit long
void ssmr_in( xxc::rnd& rnd, const xxc::big& prikey, uint32 mr, uint32 mn, xxc::big& r, xxc::big& s )
{
	do {
		// 2. get random k=[1..n-1]
		xxc::big k;
		rand_big( rnd, k );
		xxc::big modulus;
		xxc::bin_set( modulus, ECn );
		xxc::mod( k, modulus );
		if ( k[0] == 0 ) continue;
//		if (xxc::numbits(k) > 64) continue;

		// 3. compute m' = f1 || (f2(f1) ^ m )
		// 3. compute m' = f1(sk|hw) || ( f2(f1(sk||hw)) ^ mr )
		xxc::big m;
		xxc::clear( m );
		uint32 f1 = hashF1( mr, mn );
		shl_u32( m, f1, ssmrK1 );
		//
		uint32 f2 = hashF2( f1 );
		shl_u32( m, f2 ^ mr, ssmrK2 );

		// 4. r = (kG).x ^ m'
		xxc::point_t kG;
		xxc::bin_set( kG.x, GPx );
		xxc::bin_set( kG.y, GPy );
		// multiply
		xxc::mul( kG, k );
		// pack result
		xxc::pack( r, kG.x );

		// xor with m
		xxc::xor( r, m );
		// 5. c = H( r || S )
		xxc::big c;
		uint32 rhash[xxc::hash::words_sz];
		check( xxc::hash::words_sz == 8 );
		xxc::hash hf;
		hf.reset();
		// hash r
		hash_big( hf, r );
		// hash msg
//		hf.add( msg, mlen );
		hf.finalize( rhash );
		// make c
		hash2big( rhash, c );
		// 6. s = k + c*prikey | n
		xxc::mulmod( s, c, prikey, modulus );
		xxc::add( s, k );
	} while(0);
}

// SigningWithMessageRecovery - {digest + msg}
// decoding 
//
// 
bool ssmr_out( const xxc::point_t& pubk, uint32& mr, uint32 mn, const xxc::big& r, const xxc::big& s ) 
{
	// 1. c = H( r || ... )
	xxc::big c;
	uint32 rhash[xxc::hash::words_sz];
	check( xxc::hash::words_sz == 8 );
	xxc::hash hf;
	hf.reset();
	// hash r
	hash_big( hf, r );
	// hash msg
//	hf.add( msg, mlen );
	hf.finalize( rhash );
	// make c
	hash2big( rhash, c );

	// 2. m = r ^ (sG +cPub).x
	xxc::big m;
	xxc::point_t sG,cP;
	xxc::bin_set( sG.x, GPx );
	xxc::bin_set( sG.y, GPy );
	// multiply
	xxc::mul( sG, s );
	//
	xxc::copy( cP, pubk );
	// we should invert public key
	xxc::neg( cP );
	xxc::mul( cP, c );
	// summ
	xxc::add( cP, sG );
	// pack
	xxc::pack( m, cP.x );
	// xor
	xxc::xor( m, r );
	// recover mr
	uint32 m1,m2;
	m2 = shr_u32( m, ssmrK2 );
	m1 = shr_u32( m, ssmrK1 );
	mr = m2 ^ hashF2( m1 );
	// check if validates
	return m1 == hashF1( mr, mn );
}


// algorithm self-test function
//
void prove( xxc::rnd& rnd, const xxc::big& prikey, const xxc::point_t& pubk )
{
	xxc::big r,s,c,k,rr;
	do {
		rand_big( rnd, k );
		xxc::big modulus;
		xxc::bin_set( modulus, ECn );
		xxc::mod( k, modulus );
		if  ( k[0] == 0 ) continue;

		xxc::point_t kG;
		xxc::bin_set( kG.x, GPx );
		xxc::bin_set( kG.y, GPy );
		// multiply
		xxc::mul( kG, k );
		// pack result
		xxc::pack( r, kG.x );

			uint32 rhash[xxc::hash::words_sz];
			check( xxc::hash::words_sz == 8 );
			xxc::hash hf;
			hf.reset();
			// hash r
		hash_big( hf, r );
			// hash mr
			hf.finalize( rhash );
			// make c
		hash2big( rhash, c );
		
		xxc::mulmod( s, c, prikey, modulus );
		xxc::add( s, k );
	
//		dump( r, "pr" );
//		dump( s, "ps" );

		// decode
		xxc::point_t sG,cP;
		xxc::bin_set( sG.x, GPx );
		xxc::bin_set( sG.y, GPy );
		// multiply
		xxc::mul( sG, s );
		//
		xxc::copy( cP, pubk );
		// we should invert public key
		xxc::neg( cP );
		xxc::mul( cP, c );
		// summ
		xxc::add( cP, sG );
		// pack
		xxc::pack( rr, cP.x );

		if ( xxc::equ( r, rr ) ) {
			printf("proved!\n");
		} else {
			printf("failed proof!\n");
		}

	} while(0);
}


void putbits_word( uint8* p, size_t& index, uint32 val, size_t num )
{
	check( num <= 24 );
    uint32 *ptr= (uint32*)(p+(index>>3));
	// warn!! unaligned write here
    ptr[0] |= be2me_32(val<<(32-num-(index&7) ));
    ptr[1] = 0;
    index+= num;
}

void putbits_zeros( uint8* p, size_t& index, size_t num )
{
	while ( num > 24 ) {
		putbits_word( p, index, 0, 24 );
		num -= 24;
	}
	putbits_word( p, index, 0, num );
}

void putbits_big( uint8* p, size_t& index, const xxc::big& b )
{
	size_t nbits = xxc::numbits( b );
	const xxc::limb_t* w = b.begin() + 1;
	while ( nbits >= 16 ) {
		putbits_word( p, index, *w++, 16 );
		nbits-=16;
	}
	if ( nbits != 0 )
		putbits_word( p, index, *w, nbits );
}

size_t get5bits( const void* buf, size_t ndx )
{
	const uint8* p	= (const uint8*)buf;
	size_t bit_n	= 5 * ndx;
	uint32 word		= unaligned32_le( p+(bit_n>>3) ) >> (bit_n&0x07);
	return word & NEG_USR32(0xffffffff, 5 );
}

void getbits_big( const void* buf, size_t ndx, size_t len, xxc::big& b )
{
	const uint8* p	= (const uint8*)buf;
	b.clear();
	xxc::limb_t* w = b.begin() + 1;
	
	while ( len > 16 ) {
		uint32 word	= unaligned32_le( p+(ndx>>3) ) >> (ndx&0x07);
		*w++ = (xxc::limb_t)word & 0xffff;
		ndx += 16;
		len -= 16;
	}	
	// last one
	assert( len <= 16 );
	uint32 word	= unaligned32_le( p+(ndx>>3) ) >> (ndx&0x07);
	*w = (xxc::limb_t)(word & NEG_USR32(0xffffffff, len));
	// recalc length
	b[0] = w - b.begin();
	while ( b[0] && b[b[0]] == 0 ) b[0]--;
}

uint16
lshift( uint16* rp, const uint16* up, size_t n, size_t cnt )
{
	uint16 high, low;
	size_t tnc;
	size_t i;
	uint16 ret;

	assert( n >= 1 );
	assert( cnt >= 1 );
	assert( cnt < 16 );

	up += n;
	rp += n;

	tnc = 16 - cnt;
	low = *--up;
	ret = low >> tnc;
	high= (low << cnt) & 0xffff;

	for( i = n-1; i != 0; i-- )  {
		low = *--up;
		*--rp = high | (low >> tnc);
		high = (low << cnt) & 0xffff;
	}
	*--rp = high;

	return ret;
}

void shl_buf( void* buf, size_t len, size_t shift )
{
	size_t mword = shift >> 4;
	size_t mbits = shift & 15;
	uint16* sp = (uint16*)buf;
	uint16* dp = (uint16*)buf + mword;
	printf(">> %08d - %08d / %d / %d\n", sp, dp, mword, mbits );

	lshift( (uint16*)dp, (const uint16*)sp, len - mword, mbits );
	if ( mword ) wipeout( sp, mword*2 );
}

uint16
rshift( uint16* rp, const uint16* up, size_t n, size_t cnt )
{
	uint16 high, low;
	size_t tnc;
	size_t i;
	uint16 res;

	assert( n >= 1 );
	assert( cnt >= 1 );
	assert( cnt < 16 );

	tnc = 16 - cnt;

	high = *up++;
	res = (high << tnc) & 0xffff;
	low = high >> cnt;

	for( i = n - 1; i != 0; i-- ) {
		high = *up++;
		*rp++ = low | ((high << tnc) & 0xffff);
		low   = high >> cnt;
	}

	*rp = low;

	return res;
}

void shr_buf( void* buf, size_t len, size_t shift )
{
	size_t mword = shift >> 4;
	size_t mbits = shift & 15;
	uint16* sp = (uint16*)buf + mword;
	uint16* dp = (uint16*)buf;

	rshift( (uint16*)dp, (const uint16*)sp, len - mword, mbits );
	if ( mword ) wipeout( dp + (len - mword), mword*2 );
}

// or with the buffer
// WARN: function does not obey byte order!
void put_big_bits( uint8* buf, const xxc::big& b, size_t pbits )
{
	xxc::limb_t w[ xxc::limbs_n ];
	assert( b[0] <= xxc::limbs_n );
	// copy to temp array
	size_t n;
	for( n = 1; n <= b[0]; n++ )
		w[n-1] = me2le_16( b[n] );
	//
	size_t bbits = xxc::numbits( b );
	assert( bbits <= pbits );
	// copy from temp to buffer
	n = 0;
	uint16* pb = (uint16*)buf;
	while ( bbits >= 16 ) {
		pb[n] = w[n];
		bbits -= 16;
		n++;
	}

	if ( bbits != 0 ) {
		pb[n] = pb[n] | (w[n] & NEG_USR32(0xffffffff, bbits));
	}
	// clean local
	wipeout( w, sizeof(w) );
}

void get_big_bits( const uint8* buf, xxc::big& b, size_t pbits )
{
	xxc::limb_t w[ xxc::limbs_n ];
	// read bytes
	
	size_t n = 0;
	const uint16* pb = (const uint16*)buf;
	// read up to pbits
	while( pbits >= 16 ) {
		w[n] = pb[n];
		pbits -= 16;
		n++;
	}
	if ( pbits != 0 ) {
		w[n] = pb[n] & NEG_USR32(0xffffffff, pbits);
		n++;
	} 

	b[0] = n;
	for( ; n != 0; n-- ) {
		b[n] = le2me_16( w[n-1] );
	}

	while( b[0] && !b[b[0]] ) --b[0];
}



static const
char enc32[] = "0123456789ACEFGHJKLMNPQRSTUVWXYZ-";
//              23456789ACDEFGHJKLMNPRSTUVWXYZ  
//				1234567890ACEFGHJKLMNPQRSTUVWXYZ
//				0123456789ABCDEFGHJKLMNPRSTVWXYZ
//				YBNDRFG8EJKMCPQX0T1UWLSZA345H769***
always_inline uint32 
dec32( char c )
{
	const char* p = enc32;
	while (*p != 0 && *p != c ) ++p;
	if ( *p == 0 ) return 255;
	else return (p - enc32);
}

uint32 htou( const char* str )
{
	uint32 res;
	sscanf( str, "%x", &res );
	return res;
}

#include "hlp.bs.h"

void print_activation( const xxc::big& r, const xxc::big& s, size_t rbits, size_t sbits )
{
	// packs as
	// { r | pad_to(rbits) | 11 | s | pad_to(sbits) }
	uint8 bitstr[20]; // 120 bit actually

	wipeout( bitstr, sizeof(bitstr) );	


	size_t bndx = 0;
	uint8* bp	= bitstr;

#if 0
	// write r, numbits(r)
	putbits_big( bp, bndx, r );
	// write 0, rbits-numbits(r)
	putbits_zeros( bp, bndx, rbits - xxc::numbits(r) );
	// write two space bits
	putbits_word( bp, bndx, 3, 2 );
	// write s, numbits(s)
	putbits_big( bp, bndx, s );
	// write 0, sbits-numbits(s)
	putbits_zeros( bp, bndx, sbits - xxc::numbits(s) );
	// flush with zeros
    putbits_word( bp, bndx, 0, (-(int)bndx) & 7 );
	for( size_t x = 0; x < bndx/8; x++ ) printf("%02x.",bitstr[x] );

    printf("(%d) ", bndx );
#else
	put_big_bits( bitstr, s, sbits );
	shl_buf( bitstr, 10, 2+rbits );
	put_big_bits( bitstr, r, rbits );

	printf("\n>>>");
	for( size_t a = 0; a != 16; a++ ) printf("%02x ",bitstr[a]);
	printf("<<<\n");
#endif

    size_t ndig = 120/5;//(bndx+4)/5;
	iBitStream bs( bitstr, 16 );
	size_t n;
    for( n = 0; n != ndig; n++ ) {
		if ( n && n % 6 == 0 ) printf("-");
    	//printf("%c",enc32[ get5bits( bitstr, n ) ] );
		printf("%c",enc32[ bs.Read(5) ] );
    }
	printf("\n");
	bs.Reset();
    for( n = 0; n != ndig; n++ ) {
		printf("%d ", get5bits( bitstr, n ) );
	}	

    printf("\n");
	size_t z;
	for( z = 0; z <= s[0]; z++ ) printf("%04x ",s[z] );
    printf("\n");
	for( z = 0; z <= r[0]; z++ ) printf("%04x ",r[z] );
}

bool decode_activation( const char* code, xxc::big& r, xxc::big& s, size_t rbits, size_t sbits )
{
	uint8 bitstr[24]; // 120 bit actually

	wipeout( bitstr, sizeof(bitstr) );	
	size_t bndx = 0;
	uint8* bp	= bitstr;

	iBitStream bs( 128 );
//	BitstreamInit( &bs, bitstr, 20 );
	while ( *code != 0 ) {
		uint32 cc = dec32( *code++ );
		if ( bndx >= 120 ) break;
		if ( cc == 0xff ) break;
		else if ( cc > 31 ) continue;
		// put bits
//		putbits_word( bp, bndx, cc, 5 );
//		BitstreamPutBits( &bs, cc, 5 );
		bs.Write( cc, 5 );
		printf("%d ", cc );
	}
	// flush with zeros
//    putbits_word( bp, bndx, 0, (-(int)bndx) & 7 );
//	BitstreamPad( &bs );
	memcpy( bitstr, bs.GetRawData(), bs.GetRawDataSiz() );
	printf("\n");
//	for( size_t x = 0; x < bndx/8; x++ ) printf("%02x.",bitstr[x] );

	// if ( bndx < 120 ) return false;

	printf("\n>>>");
	for( size_t a = 0; a != 16; a++ ) printf("%02x ",bitstr[a]);
	printf("<<<\n");
	// analyze
	r.clear(); s.clear();
//	getbits_big( bp, 0, rbits, r );
//	getbits_big( bp, rbits+2, sbits, s );
	get_big_bits( bitstr, r, rbits );
	shr_buf( bitstr, 10, 2 + rbits );
	get_big_bits( bitstr, s, sbits );

	size_t z;
	for( z = 0; z <= s[0]; z++ ) printf("%04x ",s[z] );
printf("=\n");
	for( z = 0; z <= r[0]; z++ ) printf("%04x ",r[z] );

	return true;
}

void make_code( xxc::rnd& rnd, const xxc::big& pk, uint32 hwid, uint32 serial )
{
	// a) check hwid, serial validity beforehead
	// b) message we are signing is {mr=serial||mn=hwid}, however hwid is not transmitted
	// 

	// 1. make public key from private
	xxc::point_t pubk;
	make_public( pk, pubk );

	// 2. prepare be-ordered hwid image to hash
	uint32 xhwid = hwid ^ 0xD807AA98;

	// 3. xor crypt the serial (justincase)
	uint32 xserial = serial ^ 0x0538A55F;

	// 3. call signature function
	xxc::big ss_r, ss_s;
	ssmr_in( rnd, pk, xserial, xhwid, ss_r, ss_s );

	// 4. verify the algorithm validity
	uint32 xserial_out = 0;
	bool res = ssmr_out( pubk, xserial_out, xhwid, ss_r, ss_s );

	if ( !res || (xserial_out != xserial) ) {
		printf("internal algorithm failture!\n");
		return;
	}

	// 5. print the key
	// 63 - maximum bit length of the _r_ variable
	// 55 - maximum bit length of the _s_ variable
	print_activation( ss_r, ss_s, 63, 55 );

}

bool decode_serial( const xxc::big& pk, const char* code, uint32 hwid, uint32& serial )
{
	// 0. decode activation key
	xxc::big ss_r, ss_s;
	if ( !decode_activation( code, ss_r, ss_s, 63, 55 ) ) {
		printf("invalid code format!\n");
		return false;
	}

	// 1. make public key from private
	xxc::point_t pubk;
	make_public( pk, pubk );

	// 2. prepare be-ordered hwid image to hash
	uint32 xhwid = hwid ^ 0xD807AA98;
	uint32 xserial_out = 0;

	if ( !ssmr_out( pubk, xserial_out, xhwid, ss_r, ss_s ) ) {
		printf("validation failed!\n");
		return false;
	}

	serial = xserial_out ^ 0x0538A55F;
	printf("success: %08x\n", serial );
	return true;
}

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

void entropy_init( xxc::rnd& r, const char* file )
{
	r.reset( GetTickCount() );

	FILE* f = fopen( file, "wb" );
	if ( NULL != f ) {
		fseek( f, 0, SEEK_END );
		size_t fsize = ftell( f );
		fseek( f, 0, SEEK_SET );
		
		if ( fsize > 32768 ) fsize = 32768;
		uint8* ptr = (uint8*)malloc( fsize );
		fread( ptr, fsize, 1, f );
	    fclose(f);
		
		r.reset( ptr, fsize );
		free( ptr );
	}
}

bool entropy_load( xxc::rnd& r, const char* file )
{
	uint32 st[xxc::rnd::state_sz];

	bool inited = false;
	FILE* f = fopen( file, "rb" );
	if ( NULL != f ) {
		size_t bytes = fread( st, sizeof(st), 1, f );
		if ( bytes == sizeof(st) ) {
			r.load( st );
			inited = true;
		}
		fclose(f);
	}

	return inited;
}

void entropy_save( xxc::rnd& r, const char* file )
{
	uint32 st[xxc::rnd::state_sz];

	r.save( st );

	FILE* f = fopen( file, "wb" );
	if ( NULL != f ) {
	
		fwrite( st, sizeof(st), 1, f );
		fclose( f );
	}
	
}

void big_save( FILE* f, xxc::big& b )
{
	uint16 bits[20];
	check( b[0] < 20 );

	size_t sz = b[0];
	for( size_t n = 0; n <= sz; n++ ) 
		bits[n] = me2be_16( b[n] );

	fwrite( bits, (sz+1), 2, f );
}

bool big_load( FILE*f, xxc::big& b )
{
	uint16 bits[20];

	fread( bits, 2, 1, f );
	b[0] = be2me_16( bits[0] );

	if ( b[0] > 10 ) return false;

	size_t readen = fread( bits+1, b[0]*2, 1, f );
//	if ( readen != 2*b[0] ) return false;

	for( size_t n = 1; n <= b[0]; n++ ) {
		b[n] = be2me_16( bits[n] );
	}

	return true;
}

#define ENTROPY_FILE "entropy.bin"

// - generate private & public keys
// xauth k  <pk-filename> <"some long pass phrase">
// - generate code
// xauth s <pk-filename> hwid serial
// - decode serial
// xauth d <pk-filename> code hwid

struct auto_init
{
	auto_init()		{ xxc::init(); }
	~auto_init()	{ xxc::fini(); }
};

int main( int argc, char* argv[] )
{
	auto_init guard;

	if ( argc < 2 ) {
		printf("specify command (k,s,d)...\n");
		return 1;
	}

	char cmd = toupper( *argv[1] );
	if ( cmd == 'K' ) {
		if ( argc < 3 ) {
			printf("syntax: pk-file [\"password\"]\n");
			return 2;
		}

		xxc::big pk;
		const char* 	fname = argv[2];
		// check if password preset
		if ( argc > 3 ) {
			printf("creating private key from password string..\n");
			const char* pwd = argv[3];
			make_private( (const uint8*)pwd, strlen(pwd), pk );
		} else {
			printf("creating private key from random..\n");
			xxc::rnd rnd;
			if ( !entropy_load( rnd, ENTROPY_FILE ) ) 
				rnd.reset( GetTickCount() );
			make_private( rnd, pk );
			entropy_save( rnd, ENTROPY_FILE );
		}

		// save the key
		FILE* f = fopen( fname, "wb" );
		if ( NULL != f ) {
			big_save( f, pk );
			fclose(f);
		}

		// generate public
		xxc::point_t pG;
		xxc::big pub;
		make_public( pk, pG );
		xxc::pack( pub, pG );
		// pretty print public
		printf("{ %d", pub[0]);
		for( size_t nn = 1; nn <= pub[0]; nn++ ) {
			printf( ", 0x%04xUL", pub[nn] );
		}
		printf(" }\n");

	} else if ( cmd == 'S' ) {
		if ( argc < 4 ) {
			printf("synatax: pkfile hwid serial\n");
			return 2;
		}

		FILE* f = fopen( argv[2], "rb" );
		if ( NULL == f ) {
			printf("cannot open pkfile!\n");
			return 2;
		}
	
		xxc::big pk;
		bool readen = big_load( f, pk );
		fclose( f );
		if ( !readen ) {
			printf("problems reading pkfile!\n");
			return 2;
		}

		// parse hw & serial
		uint32 hwid = htou( argv[3] );
		uint32 seri = htou( argv[4] );

		if ( !hwid || !seri ) {
			printf("hwid or serial are probably invalid!\n");
			return 3;
		}
		
		xxc::rnd rnd;
//		if ( !entropy_load( rnd, ENTROPY_FILE ) ) 
//			rnd.reset( GetTickCount() );
	rnd.reset( 1234 );

		printf("{%x,%x}\n",hwid, seri );
		make_code( rnd, pk, hwid, seri );

//		entropy_save( rnd, ENTROPY_FILE );
	} else if ( cmd == 'D' ) {
		if ( argc < 4 ) {
			printf("synatax: pkfile hwid \n");
			return 2;
		}

		FILE* f = fopen( argv[2], "rb" );
		if ( NULL == f ) {
			printf("cannot open pkfile!\n");
			return 2;
		}
	
		xxc::big pk;
		bool readen = big_load( f, pk );
		fclose( f );
		if ( !readen ) {
			printf("problems reading pkfile!\n");
			return 2;
		}

		// parse hw 
		uint32 hwid = htou( argv[4] );

		// decode code
		xxc::big r, s;
		uint32 serial;

		printf("{%s,%04x}\n", argv[3], hwid );
		bool res = 
			decode_serial( pk, argv[3], hwid, serial );
		
	} else {
		printf("invalid command...\n");
	}
	
/*
	//
	xxc::rnd rnd;

	if ( !entropy_load( rnd, "entropy.bin" ) )
		rnd.reset( GetTickCount() );

    xxc::big pk;
	make_private( (const uint8*)"kasjdlkjasldjasld", 10, pk );

//	rnd.reset( 0x343482 );
	make_code( rnd, pk, 0x1234, 0x56789 );

	entropy_save( rnd, "entropy.bin" );
*/
	return 0;
}
