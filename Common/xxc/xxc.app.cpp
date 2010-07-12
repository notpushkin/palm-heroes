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
#include "xxc.ecp.h"
#include "xxc.shash.h"
#include "xxc.rnd.h"

#include "xxc.defs.h"
#include "xxc.bloom.h"
#include "xxc.cipher.h"
#include "xxc.hwid.h"

#include "xxc.wbox.h"

#ifndef ECC63
//xxc::limb_t GPx[] = {7U, 0x00e5U, 0x011bU, 0x01dcU, 0x001fU, 0x0168U, 0x0055U, 0x003cU };
//xxc::limb_t GPy[] = {7U, 0x00a5U, 0x01eaU, 0x0134U, 0x0124U, 0x0010U, 0x01f8U, 0x00cfU };
//xxc::limb_t ECn[] = {4U, 0xad67U, 0x3ccbU, 0x3c3cU, 0x003cU };
#define ssmrK 63
#define ssmrK1 27
#define ssmrK2 27
#else
//xxc::limb_t GPx[] = {5U, 0x0324U, 0x0e80U, 0x0272U, 0x01e1U, 0x061fU };
//xxc::limb_t GPy[] =	{5U, 0x0aaeU, 0x007cU, 0x0a3aU, 0x0449U, 0x0511U };
//xxc::limb_t ECn[] = {4U, 0x26cdU, 0x0a48U, 0x0668U, 0x0001U};
#define ssmrK 60
#define ssmrK1 24
#define ssmrK2 24
#endif


namespace xxc {

//
// const maker functions. BTW make more flexible and in autogen style!
always_inline uint32
make_0xDEADBEEF()
{ 
	uint32 res = (uint32)&res;
	while ( res > 0 ) res <<= 8;
	res = (res ^ 0x02);
	res = ((res&0x0f)|(res << 4)) - 1;
	res = (res&0x0f|(res << 4)) + 4;
	res = (res&0x0f|(res << 4)) - 3;
	res |= (res << 16);
	res += 0x2000;
	res = ~(res -= 0x42);
	check( res == 0xDEADBEEF );
	return res;
}

always_inline uint32
make_0xC942D947()
{
	uint32 res = make_0xDEADBEEF() - 0x156ae5a8;
	check( res == 0xC942D947 ); return res;
}

always_inline uint32
make_0xD807AA98()
{
	uint32 res = make_0xDEADBEEF() - 0x06a61457;
	check( res == 0xD807AA98 ); return res;
}

always_inline uint32
make_0x0538A55F()
{
	uint32 res = (make_0xDEADBEEF() >> 4) - 0x8b2368f;
	check( res == 0x0538A55F ); return res;
}

always_inline uint32
make_0x1b63fe59()
{
	uint32 res = (make_0x0538A55F()*5) + 0x148c37e;
	check( res == 0x1b63fe59 ); return res;
}

always_inline uint32
make_0x88EA55C4()
{
	uint32 res= (make_0xDEADBEEF() >> 1)	+ 0x1993764d;
	check( res == 0x88EA55C4 ); return res;
}

always_inline uint32
make_0x5aeef1eb()
{
	uint32 res = (make_0x0538A55F() << 4) + 0x07649bfb;
	check( res == 0x5aeef1eb ); return res;
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


static uint16
lshift( uint16* rp, const uint16* up, size_t n, size_t cnt )
{
	uint16 high, low;
	size_t tnc;
	size_t i;
	uint16 ret;

	check( n >= 1 );
	check( cnt >= 1 );
	check( cnt < 16 );

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
	//printf(">> %08d - %08d / %d / %d\n", sp, dp, mword, mbits );

	lshift( (uint16*)dp, (const uint16*)sp, len - mword, mbits );
	if ( mword ) wipeout( sp, mword*2 );
}

static uint16
rshift( uint16* rp, const uint16* up, size_t n, size_t cnt )
{
	uint16 high, low;
	size_t tnc;
	size_t i;
	uint16 res;

	check( n >= 1 );
	check( cnt >= 1 );
	check( cnt < 16 );

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

/*
static uint32 ___hashF1( uint32 v )
{ 
	v = bloom_mix( v, 0xCAFEBABE ); 
	return v & NEG_USR32( 0xffffffff, ssmrK1 );
}
*/

static uint32 hashF1( uint32 a, uint32 b )
{
//	uint32 v = xxc::bloom_mix( a, b ); 
	uint32 img[8];
	uint32 arg[4];

	arg[0] = me2be_32( make_0xC942D947() );
	arg[1] = me2be_32( a );
	arg[2] = me2be_32( a ^ b );
	arg[3] = me2be_32( a + b );

	hash h;
	h.reset();
	h.add( arg, sizeof(arg) );
	h.finalize( img );

	arg[0] = four2one( img );

	return arg[0] & NEG_USR32( 0xffffffff, ssmrK1 );
}

// URGE: Scramble constants! Obfuscate control-flow
// TODO: Upgrade this function with the key's store update!!
static uint32 hashF2( uint32 v )
{ 
	v = bloom_mix( make_0xDEADBEEF(), v ); 
	return v & NEG_USR32( 0xffffffff, ssmrK2 );
}

void hash_big( hash& hf, const big& b )
{
	// check - relies on what big::word_t is uint16
	// including length limb
	for( size_t n = 0; n <= b[0]; n++ ) {
		uint16 be_short = me2be_16( b[n] );
		hf.add( &be_short, 2 );
	}
}

void hash2big( const uint32 img[8], big& b )
{
	size_t n;
	b.clear();
	for( n = 0; n != 16; n++ ) {
		uint32 iw = img[n>>1];
		uint16 is = (uint16)(iw >> (16*(n&1)));
		// TODO:: replace division!!!
		b[1+(n % limbs_n)] ^= is;
	}
	for( n = limbs_n; n != 0; n-- ) {
		if ( b[n] ) break;
	}
	b[0] = n;
}

template< typename T >
always_inline
void bin_decode( T& p, const uint32* ptr, size_t len )
{
	for( size_t n = 0; n != len; n++ ) {
		p[n+1] = (uint16) wbox_dec( *ptr++ );
	}
	p[0] = len;
}

// SigningWithMessageRecovery - {digest + msg}
// decoding 
// in: 	public key (pubk)
// 		hardware id (mn)
//		sign.r (r)
//		sign.s (s)
// out:	return value - validation
//		serial - mr
struct ssmr_data
{
	uint32	xserial;
	uint32	sign;
	uint32	result;

	inline bool	validate( uint32 hwid )
	{ return (sign = hashF1( xserial, hwid ) ^ sign); }
	
	~ssmr_data() {	wipeout( this, sizeof(ssmr_data) ); }
};


void ssmr_out( const xxc::point_t& pubk, ssmr_data& data, const big& r, const big& s ) 
{
	// 1. c = H( r || ... )
	big c;
	uint32 rhash[hash::words_sz];
	check( hash::words_sz == 8 );
	hash hf;
	hf.reset();
	// hash r
	hash_big( hf, r );
	// hash msg
//	hf.add( msg, mlen );
	hf.finalize( rhash );
	// make c
	hash2big( rhash, c );

	// 2. m = r ^ (sG +cPub).x
	big m;
	point_t sG,cP;
	// REPLACED WITH W-BOXED constants
	wipeout( &sG, sizeof(sG) );
	// NB:: note length!
	bin_decode( sG.x, e_eccX, 7 );
	bin_decode( sG.y, e_eccY, 7 );
	//bin_set( sG.x, GPx );
	//bin_set( sG.y, GPy );
	// multiply
	mul( sG, s );
	//
	copy( cP, pubk );
	// we should invert public key
	neg( cP );
	mul( cP, c );
	// summ
	add( cP, sG );
	// pack
	pack( m, cP.x );
	// xor
	eor( m, r );
	// recover mr
	uint32 m1,m2;
	m2 = shr_u32( m, ssmrK2 );
	m1 = shr_u32( m, ssmrK1 );
	uint32 mr = m2 ^ hashF2( m1 );
	// check if validates
	//return m1 == hashF1( mr, mn );

	data.xserial = mr;
	data.sign	 = m1;

	// what we've got here?
	// finally we have: 
	// mr - recovered serial
	// m1 - signature
	// mn - hwid
	// validation is:
	// m1 == hashF1( serial, hwid )
}

/*
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
*/

always_inline uint32
dec32( char c )
{
	char cd = '0';
	if ( c > '@' ) c &= ~(0x20);
	if ( c > '@' ) cd+= ('A'-':');
	if ( c > 'A' ) cd++;
	if ( c > 'C' ) cd++;
	if ( c > 'H' ) cd++;
	if ( c > 'N' ) cd++;
	if ( (c == '-') || (c > 'Z') ) return 255;
	return c - cd;
}


bool decode_activation( const unsigned char* code, size_t code_len, big& r, big& s, size_t rbits, size_t sbits )
{
	uint8 bitstr[24]; // 120 bit actually

	wipeout( bitstr, sizeof(bitstr) );	

	/*
	size_t bndx = 0;
	uint8* bp	= bitstr;

	iBitStream bs( 128 );
	while ( *code != 0 ) {
		uint32 cc = dec32( *code++ );
		if ( bndx >= 120 ) break;
		if ( cc == 0xff ) break;
		else if ( cc > 31 ) continue;
		// put bits
		bs.Write( cc, 5 );
	}*/

	// flush with zeros
	memcpy( bitstr, code, code_len );

	// analyze
	r.clear(); s.clear();
	get_big_bits( bitstr, r, rbits );
	shr_buf( bitstr, 10, 2 + rbits );
	get_big_bits( bitstr, s, sbits );

	wipeout( bitstr, sizeof(bitstr) );

	return true;
}


// ppuk - packed public key
bool decode_serial( const unsigned char* code, size_t code_len, uint32 hwid, uint32& serial )
{
	// 0. decode activation key
	big ss_r, ss_s;
	if ( !decode_activation( code, code_len, ss_r, ss_s, 63, 55 ) ) {
		return false;
	}

	// 1. make public key from packed
	big ppuk;
	bin_decode( ppuk, e_puk, 4 ); // note length!
	point_t pubk;
	unpack( pubk, ppuk );


	// TODO: allocate dynamically!
	ssmr_data xdata;
	ssmr_out( pubk, xdata, ss_r, ss_s );
	//if ( !ssmr_out( pubk, xserial_out, xhwid, ss_r, ss_s ) ) return false;

	serial = xdata.xserial ^ make_0x0538A55F();

	// 2. prepare be-ordered hwid image to hash
	uint32 xhwid = hwid ^ make_0xD807AA98();

	// result must be zero if validation succeeded
	xhwid = xdata.validate( xhwid );

#ifdef _HMM_GAME_
	memcpy(pSecNum, &hwid, sizeof(hwid));
#endif
	// NOTE:: Normally we would like to hind these constants
	// and postpone validation until, ugh.. better time
	return 0 == xhwid;
}

//const limb_t PUBKEY[] = { 4, 0xb1b4UL, 0x116dUL, 0x7b60UL, 0x6dedUL };
uint32 ProcessActivationKey(LPCWSTR act_key)
{
	size_t keyLen = wcslen(act_key);
	if (keyLen != 24) return 0;
	unsigned char bbuf[24];
	Text2Bin(act_key, keyLen, bbuf, sizeof(bbuf));
	
#ifdef OS_WINCE
	xxc::hash h;
	uint32 res[8];
	h.reset();
	GetDeviceId(h);
	h.finalize( res );
	uint32 hwid = res[0] + res[1] + res[2];
	//hwid = 0x8894950c;
#else
	uint32 hwid = 0xDEADBABE;
	//uint32 hwid = 0x8894950c;
#endif

	uint32 serial;
	//big ppuk;
	//xxc::bin_set( ppuk, PUBKEY );
	bool bRes = decode_serial( bbuf, sizeof(bbuf), hwid, serial);
	if (!bRes) return 0;
	
	return serial;
}

void cipher_block( uint32 seckey, uint32 filekey, void* ptr, uint32 blksize )
{
	// pretransform
	// 1CFACBD2714D1F663AD93B84D808241D
	// k1 = seckey	(+) filekey ^ 104DDD87
	// k2 = filekey (+) blksize ^ 80334D4C
	// k3 = seckey  (#) blksize + AA6F8E28
	// k4 = k1      (#) k2		+ 88EA55C4

	uint32 key[4];
	uint32 tmp[2];

	tmp[0] = seckey;
	tmp[1] = blksize;

	key[0] = (seckey	+ filekey) ^ 0x104DDD87;
	key[1] = (filekey	+ blksize) ^ 0x80334D4C;
	key[2] = two2one( tmp )	+ 0xAA6F8E28;
	key[3] = two2one( key ) + 0x88EA55C4;
	// erase data
	wipeout( tmp, sizeof(tmp) );
	
	cipher cp;
	cp.reset( key );
	wipeout( key, sizeof(key) );

	cp.process( (uint8*)ptr, blksize );
	cp.clear();
}

void scramble_magic( uint32 seckey, uint32 magic, uint32 keys[4] )
{
	uint32 img[	16 ];

	// scramble bits
	uint32 val = make_0x1b63fe59();
	val ^= bloom_mix( seckey, make_0x88EA55C4() );
	val += bloom_mix( make_0x5aeef1eb(), magic );

	// hash job
	bloom_hash( val, img );

	// obfurscation
	val += bloom_mix( seckey, magic );
	seckey ^= val;

	keys[0] = img[0];
	keys[1] = img[1];
	keys[2] = img[2] + img[3];
	keys[3] = img[4] + img[5];

	// obfurscation
	magic -= img[0];
	seckey^= magic;
	val	>>= 13;
	
	wipeout( img, sizeof(img) );
}

} // xxc
