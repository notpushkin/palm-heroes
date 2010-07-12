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
// Stream ciphers can be built using hash functions. 
// Often this is done by first building a cryptographically 
// secure pseudorandom number generator and then using its stream 
// of random bytes as keystream and XOR that onto the cleartext 
// to get the ciphertext. SEAL is such a stream cipher which is based on SHA-1.
//
// Salsa modified hash transform
// Rijndal modified hash transform
// HMAC signature

#include "stdafx.h"

#include "xxc.cfg.h"
#include "xxc.defs.h"
#include "xxc.bswap.h"

#include "xxc.shash.h"

namespace xxc {

static const hash::word_t hashiv[16] = {
	0x7B961B91,0x556AC084,0x318DCF7F,0xAA7F20E2,
	0x8A6F405E,0xCE150AC1,0x29C5A6EA,0x9853EA38,
	0x37DF1619,0xB42EC9AB,0x5182CE48,0x33B63568,
	0x900F93B0,0xA4C3999A,0x691D991A,0x923CFAE7
};

static const uint8 padding[64] =
{
 0x80, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};


#if 0
///
/// SALSA 20/16 modified hash
///
///
#define R(a,b) XXC_ROTL32C( a, b )

//
// WARN:: be aware of misaligned read and writes!
static void 
transform( hash::word_t* img, const hash::word_t* in )
{
	// salsa12 transform
	hash::word_t x[16];
	size_t i;

	// was x = in
	for (i = 0;i != hash::words_sz;++i) x[i] = img[i];

	for (i = 0; i != 16; i+=2 ) {
		//??!! here
		x[ 0] += be2me_32( in[i] );
		x[ 8] += be2me_32( in[i + 1] );
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
	for (i = 0;i != hash::words_sz;++i) img[i] += x[i];
}

///
/// FISH-256 modified
/// 512 -> 256

#define FR(x,y) XXC_ROTL32C( (x), y )
#define FF(x) ( x + (FR(x,7)  ^ FR(x,22)) )
#define FG(x) ( x ^ (FR(x,13) + FR(x,27)) )

#define ROUND(A,B,C,D,E,F,G,H,M1,M2,D1,D2) \
	temp= H + FR(FG(E+M2),21) ^ FR(FF(E+M2+D2),17); \
	H 	= G + FR(FG(E+M2),9)  ^ FR(FF(E+M2+D2),5); \
	G 	= F + FG(E+M2) ^ FF(E+M2+D2); \
	F 	= E + M2 + D2; \
	E 	= D + FR(FF(A+M1),17) ^ FR(FG(A+M1+D1),21); \
	D 	= C + FR(FF(A+M1),5)  ^ FR(FG(A+M1+D1),9); \
	C 	= B + FF(A+M1) ^ FG(A+M1+D1); \
	B 	= A + M1 + D1; \
	A 	= temp; \
	/* */

static void 
transform2( hash::word_t* CV, const hash::word_t* M )
{
	hash::word_t A1,B1,C1,D1,E1,F1,G1,H1;
	hash::word_t A2,B2,C2,D2,E2,F2,G2,H2;
	hash::word_t A3,B3,C3,D3,E3,F3,G3,H3;
	hash::word_t A4,B4,C4,D4,E4,F4,G4,H4;
	hash::word_t temp;

	A1 = A2 = A3 = A4 = CV[0]; B1 = B2 = B3 = B4 = CV[1];
	C1 = C2 = C3 = C4 = CV[2]; D1 = D2 = D3 = D4 = CV[3];
	E1 = E2 = E3 = E4 = CV[4]; F1 = F2 = F3 = F4 = CV[5];
	G1 = G2 = G3 = G4 = CV[6]; H1 = H2 = H3 = H4 = CV[7];

	// BRANCH1(CV,M)
	ROUND(A1,B1,C1,D1,E1,F1,G1,H1,M[0],M[1],0x428a2f98,0x71374491);
	ROUND(A1,B1,C1,D1,E1,F1,G1,H1,M[2],M[3],0xb5c0fbcf,0xe9b5dba5);
	ROUND(A1,B1,C1,D1,E1,F1,G1,H1,M[4],M[5],0x3956c25b,0x59f111f1);
	ROUND(A1,B1,C1,D1,E1,F1,G1,H1,M[6],M[7],0x923f82a4,0xab1c5ed5);
	ROUND(A1,B1,C1,D1,E1,F1,G1,H1,M[8],M[9],0xd807aa98,0x12835b01);
	ROUND(A1,B1,C1,D1,E1,F1,G1,H1,M[10],M[11],0x243185be,0x550c7dc3);
	ROUND(A1,B1,C1,D1,E1,F1,G1,H1,M[12],M[13],0x72be5d74,0x80deb1fe);
	ROUND(A1,B1,C1,D1,E1,F1,G1,H1,M[14],M[15],0x9bdc06a7,0xc19bf174);
	// BRANCH2(CV,M)
	ROUND(A2,B2,C2,D2,E2,F2,G2,H2,M[14],M[15],0xc19bf174,0x9bdc06a7);
	ROUND(A2,B2,C2,D2,E2,F2,G2,H2,M[11],M[9],0x80deb1fe,0x72be5d74);
	ROUND(A2,B2,C2,D2,E2,F2,G2,H2,M[8],M[10],0x550c7dc3,0x243185be);
	ROUND(A2,B2,C2,D2,E2,F2,G2,H2,M[3],M[4],0x12835b01,0xd807aa98);
	ROUND(A2,B2,C2,D2,E2,F2,G2,H2,M[2],M[13],0xab1c5ed5,0x923f82a4);
	ROUND(A2,B2,C2,D2,E2,F2,G2,H2,M[0],M[5],0x59f111f1,0x3956c25b);
	ROUND(A2,B2,C2,D2,E2,F2,G2,H2,M[6],M[7],0xe9b5dba5,0xb5c0fbcf);
	ROUND(A2,B2,C2,D2,E2,F2,G2,H2,M[12],M[1],0x71374491,0x428a2f98);
	// BRANCH3(CV,M)
	ROUND(A3,B3,C3,D3,E3,F3,G3,H3,M[7],M[6],0x71374491,0x428a2f98);
	ROUND(A3,B3,C3,D3,E3,F3,G3,H3,M[10],M[14],0xe9b5dba5,0xb5c0fbcf);
	ROUND(A3,B3,C3,D3,E3,F3,G3,H3,M[13],M[2],0x59f111f1,0x3956c25b);
	ROUND(A3,B3,C3,D3,E3,F3,G3,H3,M[9],M[12],0xab1c5ed5,0x923f82a4);
	ROUND(A3,B3,C3,D3,E3,F3,G3,H3,M[11],M[4],0x12835b01,0xd807aa98);
	ROUND(A3,B3,C3,D3,E3,F3,G3,H3,M[15],M[8],0x550c7dc3,0x243185be);
	ROUND(A3,B3,C3,D3,E3,F3,G3,H3,M[5],M[0],0x80deb1fe,0x72be5d74);
	ROUND(A3,B3,C3,D3,E3,F3,G3,H3,M[1],M[3],0xc19bf174,0x9bdc06a7);
	// BRANCH4(CV,M)
	ROUND(A4,B4,C4,D4,E4,F4,G4,H4,M[5],M[12],0x9bdc06a7,0xc19bf174);
	ROUND(A4,B4,C4,D4,E4,F4,G4,H4,M[1],M[8],0x72be5d74,0x80deb1fe);
	ROUND(A4,B4,C4,D4,E4,F4,G4,H4,M[15],M[0],0x243185be,0x550c7dc3);
	ROUND(A4,B4,C4,D4,E4,F4,G4,H4,M[13],M[11],0xd807aa98,0x12835b01);
	ROUND(A4,B4,C4,D4,E4,F4,G4,H4,M[3],M[10],0x923f82a4,0xab1c5ed5);
	ROUND(A4,B4,C4,D4,E4,F4,G4,H4,M[9],M[2],0x3956c25b,0x59f111f1);
	ROUND(A4,B4,C4,D4,E4,F4,G4,H4,M[7],M[14],0xb5c0fbcf,0xe9b5dba5);
	ROUND(A4,B4,C4,D4,E4,F4,G4,H4,M[4],M[6],0x428a2f98,0x71374491);

	// output
	CV[0] = CV[0] + ((A1 + A2) ^ (A3 + A4));
	CV[1] = CV[1] + ((B1 + B2) ^ (B3 + B4));
	CV[2] = CV[2] + ((C1 + C2) ^ (C3 + C4));
	CV[3] = CV[3] + ((D1 + D2) ^ (D3 + D4));
	CV[4] = CV[4] + ((E1 + E2) ^ (E3 + E4));
	CV[5] = CV[5] + ((F1 + F2) ^ (F3 + F4));
	CV[6] = CV[6] + ((G1 + G2) ^ (G3 + G4));
	CV[7] = CV[7] + ((H1 + H2) ^ (H3 + H4));
}


/// 8->16->8 salsa16 two branch variant
/// in[16], img[8]
static void 
transform3__( hash::word_t* img, const hash::word_t* in )
{
	hash::word_t x[16];
	size_t i;

	// was x = in
	for (i = 0;i != 16;++i) {
		x[i] = img[i>>1] + be2me_32( in[i] );
	}

		x[ 4] ^= R(x[ 0]+x[12], 7);  x[ 8] ^= R(x[ 4]+x[ 0], 9);	x[ 4] += be2me_32( in[ 5] );
		x[12] ^= R(x[ 8]+x[ 4],13);  x[ 0] ^= R(x[12]+x[ 8],18);	x[12] += be2me_32( in[12] );
		x[ 9] ^= R(x[ 5]+x[ 1], 7);  x[13] ^= R(x[ 9]+x[ 5], 9);	x[ 9] += be2me_32( in[ 1] );
		x[ 1] ^= R(x[13]+x[ 9],13);  x[ 5] ^= R(x[ 1]+x[13],18);	x[ 1] += be2me_32( in[ 8] );
		x[14] ^= R(x[10]+x[ 6], 7);  x[ 2] ^= R(x[14]+x[10], 9);	x[14] += be2me_32( in[14] );
		x[ 6] ^= R(x[ 2]+x[14],13);  x[10] ^= R(x[ 6]+x[ 2],18);	x[ 6] += be2me_32( in[ 0] );
		x[ 3] ^= R(x[15]+x[11], 7);  x[ 7] ^= R(x[ 3]+x[15], 9);	x[ 3] += be2me_32( in[11] );
		x[11] ^= R(x[ 7]+x[ 3],13);  x[15] ^= R(x[11]+x[ 7],18);	x[11] += be2me_32( in[13] );
		x[ 1] ^= R(x[ 0]+x[ 3], 7);  x[ 2] ^= R(x[ 1]+x[ 0], 9);	x[ 1] += be2me_32( in[ 3] );
		x[ 3] ^= R(x[ 2]+x[ 1],13);  x[ 0] ^= R(x[ 3]+x[ 2],18);	x[ 3] += be2me_32( in[10] );
		x[ 6] ^= R(x[ 5]+x[ 4], 7);  x[ 7] ^= R(x[ 6]+x[ 5], 9);	x[ 6] += be2me_32( in[ 9] );
		x[ 4] ^= R(x[ 7]+x[ 6],13);  x[ 5] ^= R(x[ 4]+x[ 7],18);	x[ 4] += be2me_32( in[ 2] );
		x[11] ^= R(x[10]+x[ 9], 7);  x[ 8] ^= R(x[11]+x[10], 9);	x[11] += be2me_32( in[ 7] );
		x[ 9] ^= R(x[ 8]+x[11],13);  x[10] ^= R(x[ 9]+x[ 8],18);	x[ 9] += be2me_32( in[14] );
		x[12] ^= R(x[15]+x[14], 7);  x[13] ^= R(x[12]+x[15], 9);	x[12] += be2me_32( in[ 4] );
		x[14] ^= R(x[13]+x[12],13);  x[15] ^= R(x[14]+x[13],18);	x[14] += be2me_32( in[ 6] );

	for (i = 0; i != 12; i+=2 ) {
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

	// was out = x + in
	//for (i = 0;i != hash::words_sz;++i) img[i] += x[i];
	for( i = 0; i != 8; i++ ) {
		img[i] += (x[0+i] + x[8+i]);
	}
}
#endif


///
/// Rijnadel / RIPEMD 512->256 (modified)
#define F(x, y, z)    (x ^ y ^ z) 
#define G(x, y, z)    (z ^ (x & (y^z)))
#define H(x, y, z)    (z ^ (x | ~y))
#define I(x, y, z)    (y ^ (z & (x^y)))
#define J(x, y, z)    (x ^ (y | ~z))


#define k0 0
#define k1 0x428a2f98UL //0x5a827999UL
#define k2 0x71374491UL //0x6ed9eba1UL
#define k3 0xb5c0fbcfUL //0x8f1bbcdcUL
#define k4 0xe9b5dba5UL //0xa953fd4eUL
#define k5 0x3956c25bUL //0x50a28be6UL
#define k6 0x59f111f1UL //0x5c4dd124UL
#define k7 0x923f82a4UL //0x6d703ef3UL
#define k8 0xab1c5ed5UL //0x7a6d76e9UL
#define k9 0

#define Subround(f, a, b, c, d, x, s, k)        \
	a += f(b, c, d) + be2me_32(x) + k;			\
	a = XXC_ROTL32C((uint32)a, s); 				\
	/* */

static void transform3( hash::word_t* digest, const hash::word_t* X )
{
	uint32 a1, b1, c1, d1, a2, b2, c2, d2, t;
	a1 = digest[0];
	b1 = digest[1];
	c1 = digest[2];
	d1 = digest[3];
	a2 = digest[4];
	b2 = digest[5];
	c2 = digest[6];
	d2 = digest[7];

	Subround(F, a1, b1, c1, d1, X[ 0], 11, k0);
	Subround(F, d1, a1, b1, c1, X[ 1], 14, k0);
	Subround(F, c1, d1, a1, b1, X[ 2], 15, k0);
	Subround(F, b1, c1, d1, a1, X[ 3], 12, k0);
	Subround(F, a1, b1, c1, d1, X[ 4],  5, k0);
	Subround(F, d1, a1, b1, c1, X[ 5],  8, k0);
	Subround(F, c1, d1, a1, b1, X[ 6],  7, k0);
	Subround(F, b1, c1, d1, a1, X[ 7],  9, k0);
	Subround(F, a1, b1, c1, d1, X[ 8], 11, k0);
	Subround(F, d1, a1, b1, c1, X[ 9], 13, k0);
	Subround(F, c1, d1, a1, b1, X[10], 14, k0);
	Subround(F, b1, c1, d1, a1, X[11], 15, k0);
	Subround(F, a1, b1, c1, d1, X[12],  6, k0);
	Subround(F, d1, a1, b1, c1, X[13],  7, k0);
	Subround(F, c1, d1, a1, b1, X[14],  9, k0);
	Subround(F, b1, c1, d1, a1, X[15],  8, k0);

	Subround(I, a2, b2, c2, d2, X[ 5],  8, k5);
	Subround(I, d2, a2, b2, c2, X[14],  9, k5);
	Subround(I, c2, d2, a2, b2, X[ 7],  9, k5);
	Subround(I, b2, c2, d2, a2, X[ 0], 11, k5);
	Subround(I, a2, b2, c2, d2, X[ 9], 13, k5);
	Subround(I, d2, a2, b2, c2, X[ 2], 15, k5);
	Subround(I, c2, d2, a2, b2, X[11], 15, k5);
	Subround(I, b2, c2, d2, a2, X[ 4],  5, k5);
	Subround(I, a2, b2, c2, d2, X[13],  7, k5);
	Subround(I, d2, a2, b2, c2, X[ 6],  7, k5);
	Subround(I, c2, d2, a2, b2, X[15],  8, k5);
	Subround(I, b2, c2, d2, a2, X[ 8], 11, k5);
	Subround(I, a2, b2, c2, d2, X[ 1], 14, k5);
	Subround(I, d2, a2, b2, c2, X[10], 14, k5);
	Subround(I, c2, d2, a2, b2, X[ 3], 12, k5);
	Subround(I, b2, c2, d2, a2, X[12],  6, k5);

	t = a1; a1 = a2; a2 = t;

	Subround(G, a1, b1, c1, d1, X[ 7],  7, k1);
	Subround(G, d1, a1, b1, c1, X[ 4],  6, k1);
	Subround(G, c1, d1, a1, b1, X[13],  8, k1);
	Subround(G, b1, c1, d1, a1, X[ 1], 13, k1);
	Subround(G, a1, b1, c1, d1, X[10], 11, k1);
	Subround(G, d1, a1, b1, c1, X[ 6],  9, k1);
	Subround(G, c1, d1, a1, b1, X[15],  7, k1);
	Subround(G, b1, c1, d1, a1, X[ 3], 15, k1);
	Subround(G, a1, b1, c1, d1, X[12],  7, k1);
	Subround(G, d1, a1, b1, c1, X[ 0], 12, k1);
	Subround(G, c1, d1, a1, b1, X[ 9], 15, k1);
	Subround(G, b1, c1, d1, a1, X[ 5],  9, k1);
	Subround(G, a1, b1, c1, d1, X[ 2], 11, k1);
	Subround(G, d1, a1, b1, c1, X[14],  7, k1);
	Subround(G, c1, d1, a1, b1, X[11], 13, k1);
	Subround(G, b1, c1, d1, a1, X[ 8], 12, k1);

	Subround(H, a2, b2, c2, d2, X[ 6],  9, k6);
	Subround(H, d2, a2, b2, c2, X[11], 13, k6);
	Subround(H, c2, d2, a2, b2, X[ 3], 15, k6);
	Subround(H, b2, c2, d2, a2, X[ 7],  7, k6);
	Subround(H, a2, b2, c2, d2, X[ 0], 12, k6);
	Subround(H, d2, a2, b2, c2, X[13],  8, k6);
	Subround(H, c2, d2, a2, b2, X[ 5],  9, k6);
	Subround(H, b2, c2, d2, a2, X[10], 11, k6);
	Subround(H, a2, b2, c2, d2, X[14],  7, k6);
	Subround(H, d2, a2, b2, c2, X[15],  7, k6);
	Subround(H, c2, d2, a2, b2, X[ 8], 12, k6);
	Subround(H, b2, c2, d2, a2, X[12],  7, k6);
	Subround(H, a2, b2, c2, d2, X[ 4],  6, k6);
	Subround(H, d2, a2, b2, c2, X[ 9], 15, k6);
	Subround(H, c2, d2, a2, b2, X[ 1], 13, k6);
	Subround(H, b2, c2, d2, a2, X[ 2], 11, k6);

	t = b1; b1 = b2; b2 = t;

	Subround(H, a1, b1, c1, d1, X[ 3], 11, k2);
	Subround(H, d1, a1, b1, c1, X[10], 13, k2);
	Subround(H, c1, d1, a1, b1, X[14],  6, k2);
	Subround(H, b1, c1, d1, a1, X[ 4],  7, k2);
	Subround(H, a1, b1, c1, d1, X[ 9], 14, k2);
	Subround(H, d1, a1, b1, c1, X[15],  9, k2);
	Subround(H, c1, d1, a1, b1, X[ 8], 13, k2);
	Subround(H, b1, c1, d1, a1, X[ 1], 15, k2);
	Subround(H, a1, b1, c1, d1, X[ 2], 14, k2);
	Subround(H, d1, a1, b1, c1, X[ 7],  8, k2);
	Subround(H, c1, d1, a1, b1, X[ 0], 13, k2);
	Subround(H, b1, c1, d1, a1, X[ 6],  6, k2);
	Subround(H, a1, b1, c1, d1, X[13],  5, k2);
	Subround(H, d1, a1, b1, c1, X[11], 12, k2);
	Subround(H, c1, d1, a1, b1, X[ 5],  7, k2);
	Subround(H, b1, c1, d1, a1, X[12],  5, k2);

	Subround(G, a2, b2, c2, d2, X[15],  9, k7);
	Subround(G, d2, a2, b2, c2, X[ 5],  7, k7);
	Subround(G, c2, d2, a2, b2, X[ 1], 15, k7);
	Subround(G, b2, c2, d2, a2, X[ 3], 11, k7);
	Subround(G, a2, b2, c2, d2, X[ 7],  8, k7);
	Subround(G, d2, a2, b2, c2, X[14],  6, k7);
	Subround(G, c2, d2, a2, b2, X[ 6],  6, k7);
	Subround(G, b2, c2, d2, a2, X[ 9], 14, k7);
	Subround(G, a2, b2, c2, d2, X[11], 12, k7);
	Subround(G, d2, a2, b2, c2, X[ 8], 13, k7);
	Subround(G, c2, d2, a2, b2, X[12],  5, k7);
	Subround(G, b2, c2, d2, a2, X[ 2], 14, k7);
	Subround(G, a2, b2, c2, d2, X[10], 13, k7);
	Subround(G, d2, a2, b2, c2, X[ 0], 13, k7);
	Subround(G, c2, d2, a2, b2, X[ 4],  7, k7);
	Subround(G, b2, c2, d2, a2, X[13],  5, k7);

	t = c1; c1 = c2; c2 = t;

	Subround(I, a1, b1, c1, d1, X[ 1], 11, k3);
	Subround(I, d1, a1, b1, c1, X[ 9], 12, k3);
	Subround(I, c1, d1, a1, b1, X[11], 14, k3);
	Subround(I, b1, c1, d1, a1, X[10], 15, k3);
	Subround(I, a1, b1, c1, d1, X[ 0], 14, k3);
	Subround(I, d1, a1, b1, c1, X[ 8], 15, k3);
	Subround(I, c1, d1, a1, b1, X[12],  9, k3);
	Subround(I, b1, c1, d1, a1, X[ 4],  8, k3);
	Subround(I, a1, b1, c1, d1, X[13],  9, k3);
	Subround(I, d1, a1, b1, c1, X[ 3], 14, k3);
	Subround(I, c1, d1, a1, b1, X[ 7],  5, k3);
	Subround(I, b1, c1, d1, a1, X[15],  6, k3);
	Subround(I, a1, b1, c1, d1, X[14],  8, k3);
	Subround(I, d1, a1, b1, c1, X[ 5],  6, k3);
	Subround(I, c1, d1, a1, b1, X[ 6],  5, k3);
	Subround(I, b1, c1, d1, a1, X[ 2], 12, k3);

	Subround(F, a2, b2, c2, d2, X[ 8], 15, k9);
	Subround(F, d2, a2, b2, c2, X[ 6],  5, k9);
	Subround(F, c2, d2, a2, b2, X[ 4],  8, k9);
	Subround(F, b2, c2, d2, a2, X[ 1], 11, k9);
	Subround(F, a2, b2, c2, d2, X[ 3], 14, k9);
	Subround(F, d2, a2, b2, c2, X[11], 14, k9);
	Subround(F, c2, d2, a2, b2, X[15],  6, k9);
	Subround(F, b2, c2, d2, a2, X[ 0], 14, k9);
	Subround(F, a2, b2, c2, d2, X[ 5],  6, k9);
	Subround(F, d2, a2, b2, c2, X[12],  9, k9);
	Subround(F, c2, d2, a2, b2, X[ 2], 12, k9);
	Subround(F, b2, c2, d2, a2, X[13],  9, k9);
	Subround(F, a2, b2, c2, d2, X[ 9], 12, k9);
	Subround(F, d2, a2, b2, c2, X[ 7],  5, k9);
	Subround(F, c2, d2, a2, b2, X[10], 15, k9);
	Subround(F, b2, c2, d2, a2, X[14],  8, k9);

	t = d1; d1 = d2; d2 = t;

	digest[0] += a1;
	digest[1] += b1;
	digest[2] += c1;
	digest[3] += d1;
	digest[4] += a2;
	digest[5] += b2;
	digest[6] += c2;
	digest[7] += d2;
}

#undef Subround


////////
////////
//////////////////////////////////////////////////////////////

void
hash::reset()
{
	// use default initialization vector
	reset( hashiv );
}

void
hash::reset( const word_t iv[words_sz] )
{
	wipeout( buf, sizeof(buf) );
	cnt[0] = cnt[1] = 0;

	// copy initialization vector
	for( size_t n = 0; n != words_sz; n++ )
		img[n] = be2me_32( iv[n] );
}

void
hash::add( const void* data, size_t size )
{
//	const size_t bytes_once = sizeof(word_t)*words_sz;
	const uint8* ptr = (const uint8*)data;

	uint32 left = cnt[0] & (block_sz-1);
	uint32 fill = block_sz - left;

	// update bytes counters
	cnt[0] += size;
	if ( cnt[0] < size ) ++cnt[1];

	// process buffer leftovers
	if ( left && size >= fill ) {
		copybytes( buf + left, ptr, fill );
		transform3( img, (const word_t*)buf );
		size -= fill;
		ptr  += fill;
		left  = 0;
	}

	// process full blocks
	while ( size >= block_sz ) {
		transform3( img, (const word_t*)ptr );
		size -= block_sz;
		ptr  += block_sz;
	}

	// copy leftovers
	if ( size ) {
		copybytes( buf + left, ptr, size );
	}
}

void 
hash::finalize( word_t val[words_sz] )
{
	// make 64bit be byte counter
	uint32 counter[2];
	counter[0] = me2be_32( cnt[1] );
	counter[1] = me2be_32( cnt[0] );
	
	// calc leftovers
	uint32 last = cnt[0] & (block_sz-1);
	// pad with [1..64] bytes, at least 1, taking in the account 8byte counter
	// padn must result transformation of one or two blocks
	uint32 padn = (last < (block_sz-8)) 
						? ((block_sz-8) - last)
						: ((2*block_sz-8) - last);
	check( padn != 0 && padn <= 64 );
	add( padding, padn );
	add( counter, 8 );

	// convert digest
	for( size_t n = 0; n != words_sz; n++ )
		val[n] = me2be_32( img[n] );

	// wipe buffers
	wipeout( buf, sizeof(buf) );
	wipeout( img, sizeof(img) );
	cnt[0] = cnt[1] = 0;
}

hash::hash()
{ reset(); }

//
// hmac
void
hmac::reset( const uint8* kstream, size_t keylen )
{
	size_t n;
	// copy initialization vector
	for( n = 0; n != words_sz; n++ )
		img[n] = be2me_32( hashiv[n] );
	
	// prepare plain key
	// normalize : pad with 0 if shorter
	// if longer - hash it directly, copy the reset
	const uint8* kp = (const uint8*)kstream;
	while( keylen > block_sz ) {
		copybytes( buf, kp, block_sz );
		transform3( img, (const word_t*)buf );
		kp  	+= block_sz;
		keylen	-= block_sz;
	}
	// now keylen <= bytes_sz, prepare padded
	wipeout( buf, sizeof(buf) );
	copybytes( buf, kp, keylen );
	transform3( img, (const word_t*)buf );
	// now img contains the (plain) key, copy it
	copybytes( key, img, sizeof(img) );
	// xor it with ipad
	for( n = 0; n != words_sz; n++ ) {
		img[n] ^= 0x36363636;
		key[n] ^= 0x5c5c5c5c;
	}

	// reinitialize hc
	cnt[0] = cnt[1] = 0;
}

void
hmac::add( const void* data, size_t size )
{
//	const size_t bytes_once = sizeof(word_t)*words_sz;
	const uint8* ptr = (const uint8*)data;

	uint32 left = cnt[0] & (block_sz-1);
	uint32 fill = block_sz - left;

	// update bytes counters
	cnt[0] += size;
	if ( cnt[0] < size ) ++cnt[1];

	// process buffer leftovers
	if ( left && size >= fill ) {
		copybytes( buf + left, ptr, fill );
		transform3( img, (const word_t*)buf );
		size -= fill;
		ptr  += fill;
		left  = 0;
	}

	// process full blocks
	while ( size >= block_sz ) {
		transform3( img, (const word_t*)ptr );
		size -= block_sz;
		ptr  += block_sz;
	}

	// copy leftovers
	if ( size ) {
		copybytes( buf + left, ptr, size );
	}
}

void 
hmac::finalize( word_t val[words_sz] )
{
	// make 64bit be byte counter
	uint32 counter[2];
	counter[0] = me2be_32( cnt[1] );
	counter[1] = me2be_32( cnt[0] );
	
	// calc leftovers
	uint32 last = cnt[0] & (block_sz-1);
	// pad with [1..64] bytes, at least 1, taking in the account 8byte counter
	// padn must result transformation of one or two blocks
	uint32 padn = (last < (block_sz-8)) 
						? ((block_sz-8) - last)
						: ((2*block_sz-8) - last);
	check( padn != 0 && padn <= 64 );
	add( padding, padn );
	add( counter, 8 );

	size_t n;
	// rehash results
	word_t* wbuf = (word_t*)buf;
	for( n = 0; n != words_sz; n++ ) {
		// h(ik|m) -> buf
		// iv -> img
		wbuf[2*n]	= img[n];
		wbuf[2*n+1] = key[n];
		img[n] 		= be2me_32( hashiv[n] );
	}

	// H( k | h );
	transform3( img, (const word_t*)buf );

	// convert digits
	for( n = 0; n != words_sz; n++ )
		val[n] = me2be_32( img[n] );

	// wipe buffers
	wipeout( buf, sizeof(buf) );
	wipeout( img, sizeof(img) );
	wipeout( key, sizeof(key) );
	cnt[0] = cnt[1] = 0;
}

hmac::hmac()
{}

//////////////////////////

uint32 
two2one( const uint32 in[2] )
{
	uint32 x[4];
	uint32 y[4];

	// expand to four
	x[0] = in[0] ^ (XXC_ROTL32C( in[0], 17 ) + XXC_ROTL32C( in[1], 21 ));
	x[1] = in[1] + (XXC_ROTL32C( in[0],  5 ) ^ XXC_ROTL32C( in[1],  9 ));
	x[2] = in[1] ^ (XXC_ROTL32C(  x[0],  9 ) + XXC_ROTL32C(  x[1], 21 ));
	x[3] = in[0] + (XXC_ROTL32C(  x[0], 17 ) ^ XXC_ROTL32C(  x[1],  5 ));

	// round one
	x[1] ^= XXC_ROTL32C( x[ 0] + x[3], 7 );
	x[2] ^= XXC_ROTL32C( x[ 1] + x[0], 9 );
	x[3] ^= XXC_ROTL32C( x[ 2] + x[1],13 );
	x[0] ^= XXC_ROTL32C( x[ 3] + x[2],18 );	

	// round two
	uint32 xx = x[0] + ( XXC_ROTL32C( x[2],  7 ) ^ XXC_ROTL32C( x[3], 22 ) );
	uint32 xy = x[2] + ( XXC_ROTL32C( x[0], 13 ) ^ XXC_ROTL32C( x[1], 27 ) );
	
	// round three
	y[3] = x[0] + XXC_ROTL32C( x[0], 21 ) ^ XXC_ROTL32C( xx, 17 );
	y[2] = x[1] + XXC_ROTL32C( x[1],  9 ) ^ XXC_ROTL32C( xy,  5 );
	y[1] = x[2] + XXC_ROTL32C( x[2], 21 ) ^ XXC_ROTL32C( xy, 17 );
	y[0] = x[3] + XXC_ROTL32C( x[3],  9 ) ^ XXC_ROTL32C( xx,  5 );
			
	// now compact into one
	uint32 res;
	res  = ( x[0] + x[1] ) ^ ( y[2] + y[3] );
	res ^= ( x[3] + x[2] ) ^ ( y[1] + y[0] );

	// little wipeout (trash computations)
	y[3] ^= XXC_ROTL32C(x[0], 11);	x[0] += ( y[0] + y[1] ) ^ ( x[2] + x[3] ); 
	x[3] ^= XXC_ROTL32C(y[0],  3);	y[0] += ( x[0] + x[1] ) ^ ( y[2] + y[3] );

	return res;
}

uint32 
four2one( const uint32 in[4] )
{
	uint32 x[4];
	uint32 y[4];

	x[0] = in[0];	x[1] = in[1];
	x[2] = in[2];	x[3] = in[3];

	// round one
	x[1] ^= XXC_ROTL32C( x[ 0] + x[3], 7 );
	x[2] ^= XXC_ROTL32C( x[ 1] + x[0], 9 );
	x[3] ^= XXC_ROTL32C( x[ 2] + x[1],13 );
	x[0] ^= XXC_ROTL32C( x[ 3] + x[2],18 );	

	// round two
	uint32 xx = x[0] + ( XXC_ROTL32C( x[2],  7 ) ^ XXC_ROTL32C( x[3], 22 ) );
	uint32 xy = x[2] + ( XXC_ROTL32C( x[0], 13 ) ^ XXC_ROTL32C( x[1], 27 ) );
	
	// round three
	y[0] = x[3] + XXC_ROTL32C( x[0], 21 ) ^ XXC_ROTL32C( xx, 17 );
	y[1] = x[2] + XXC_ROTL32C( x[1],  9 ) ^ XXC_ROTL32C( xy,  5 );
	y[2] = x[1] + XXC_ROTL32C( x[2], 21 ) ^ XXC_ROTL32C( xy, 17 );
	y[3] = x[0] + XXC_ROTL32C( x[3],  9 ) ^ XXC_ROTL32C( xx,  5 );
			
	// now compact into one
	uint32 res;
	res  = ( x[0] + x[1] ) ^ ( y[2] + y[3] );
	res ^= ( x[2] + x[3] ) ^ ( y[1] + y[0] );

	// little wipeout (trash computations)
	y[3] ^= XXC_ROTL32C(x[0], 11);	x[0] += ( y[0] + y[1] ) ^ ( x[2] + x[3] ); 
	x[3] ^= XXC_ROTL32C(y[0],  3);	y[0] += ( x[0] + x[1] ) ^ ( y[2] + y[3] );

	return res;
}


} //xxc

#ifdef TESTHASH
int main()
{
	xxc::hash h;
	xxc::hmac m;

	uint32 buf[16] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 };

	size_t n;
	for( n = 0; n != 20; n++ ) {
		uint32 res[16];
		buf[0] = n;
		h.reset();
		//h.add( buf, sizeof(uint32)*16 );
		h.add( buf, sizeof(uint32)*8 );
		h.add( &buf[8], sizeof(uint32)*8 );
		h.finalize( res );
		for( size_t m = 0; m != 8; m++ ) {
			printf("%08x", res[m] );
			if ( (m & 7) == 7 ) printf(": %08x\n", res[m] ^ res[m-1] ^ res[m-2] ^ res[m-3]);
		}
		//memcpy( buf, res, sizeof(uint32)*8 );
	}
	printf("\n");

	const uint8* key = (const uint8*)"labudafm 1234567labudafm 1234567labudafm 1234567labudafm 1234567labudafm 1234567";

	for( n = 10; n != 64+8; n++ ) {
		uint32 res[16];
		m.reset( key, n );
		m.add( buf, 16 );
		m.finalize( res );

		for( size_t m = 0; m != 8; m++ ) {
			printf("%08x", res[m] );
			if ( (m & 7) == 7 ) printf(": %08x\n", res[m] ^ res[m-1] ^ res[m-2] ^ res[m-3]);
		}
	}

	return 0;
}
#endif //TESTHASH

