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

#undef OVERFLOW

#include "xxc.cfg.h"
#include "xxc.big.h"
#include "xxc.defs.h"


namespace xxc {

//
// might be replaced with templated static_log2 function

typedef char test_if_limb_is_uint16[ sizeof(limb_t) == 2 ];

#define BITS		16
#define LOG2_BITS	4
#define BITMASK		0xffff
#define HIGH_BIT	0x8000
#define OVERFLOW	0x10000


size_t len( const big& a )
{ return a[0]; }

bool is_zero( const big& a )
{ return a[0] == 0; }

bool is_one( const big& a )
{ return a[0] == 1 && a[1] == 1; }

void clear( big& a )
{
	a.clear();
}

void copy( big& d, const big& s )
{
	copybytes( d.begin(), s.begin(), (len(s) + 1)*sizeof(limb_t) );
}

void word_set( big& d, limb_t w )
{
	d[0] = 1;
	d[1] = w;
}

void bin_set( big& v, const limb_t* w )
{
	size_t n = v[0] = *w++;
	for ( size_t j = 0; j != n; j++ ) {
		v[1+j] = *w++;
	}
}

bool equ( const big& a, const big& b )
{
	return memcmp( a.begin(), b.begin(), (len(a)+1)*sizeof(limb_t) ) == 0 ? true : false;
}

bool greater( const big& a, const big& b )
{
	if ( len(a) > len(b) ) return true;
	else if ( len(a) < len(b) ) return false;

	for( size_t n = len(a); n > 0; n-- ) {
		if ( a[n] > b[n] ) return true;
		if ( a[n] < b[n] ) return false;
	}
	return false;
}

size_t numbits( const big& a )
{
	if ( len(a) == 0 ) return 0;

	limb_t w = a[ len(a) ];
	limb_t m = HIGH_BIT;
	for( size_t n = (size_t)( len(a) << LOG2_BITS); m; n--, m >>= 1 ) {
		if ( w & m ) return n;
	}
	return 0;
}

bool bit( const big& a, size_t n )
{
	if ( n >= (len(a) << LOG2_BITS) ) return 0;
	return (bool)((a[(n >> 4) + 1] >> (n & 15)) & 1);
}

//
//
void add( big& a, const big& b )
{
	size_t i;
	wide_t t;

	// clear high limbs
	for( i = len(a) + 1; i <= len(b); i++ )
		a[i] = 0;

	// set longest
	if ( len(a) < len(b) )
		a[0] = b[0];
	
	t = 0;
	for( i = 1; i <= len(b);	i++ ) {
		t += (wide_t)a[i] + (wide_t)b[i];
		a[i] = (limb_t)( t & BITMASK );
		t >>= BITS;
	}
	
	i = len(b) + 1;
	// insert the rest
	while ( t ) {
		check( i <= limbs_n+1 ); // ???
		if ( i > len(a) ) {
			a[i] = 0;
			a[0] += 1;
		}
		t = (wide_t)a[i] + 1;
		a[i] = (limb_t)(t & BITMASK );
		t >>= BITS;
		i++;
	}
}

void add( big& r, const big& a, const big& b )
{
	copy( r, a );
	add( r, b );
}

// assumes the u >= v
void sub( big& u, const big& v )
{
	wide_t carry = 0;
	wide_t t;
	size_t i;

	check( !greater( v, u ) );

	for( i = 1; i <= len(v); i++ ) {
		t = OVERFLOW + (wide_t)u[i] - (wide_t)v[i] - carry;
		carry = 1;
		if ( t >= OVERFLOW ) {
			t -= OVERFLOW;
			carry = 0;
		}
		u[i] = (limb_t)t;
	}
	//
	if ( carry ) {
		while ( u[i] == 0 )
			i++;
		u[i]--;
	}
	while( u[ len(u) ] == 0 && len(u) )
		u[0]--;
}

void sub( big& r, const big& a, const big& b )
{
	copy( r, a );
	sub( r, b );
}

void shl( big& p, size_t n )
{
	if ( n == 0 || len(p) == 0 ) return;

	check( n <= BITS );
	if ( p[ len(p) ] >> (BITS - n)) {
		// check space
		if ( len(p) <= limbs_n + 1) {
			++p[0];
			p[ len(p) ] = 0;
		}
	}
	for( size_t i = len(p); i > 1; i-- ) {
		p[i] = ( p[i] << n ) | ( p[i-1] >> (BITS - n) );
	}
	p[1] <<= n;
}

void shr( big& p, size_t n )
{
	if ( n == 0 || len(p) == 0 ) return;
	check( n <= BITS );

	for( size_t i = 1; i < p[0]; i++ ) {
		p[i] = (p[i+1] << (BITS - n)) | (p[i] >> n);
	}
	p[ len(p) ] >>= n;
	// update length
	if ( p[ len(p) ] == 0 )
		--p[0];
}

void eor( big& v, const big& x )
{
	size_t p;
	size_t sl = len(x);
	if ( len(v) < sl ) sl = len(v);

	for( p = 1 ; p <= sl; p++ ) {
		v[p] ^= x[p];		
	}
	for( ; p <= len(x); p++ ) {
		v[p] = x[p];
		v[0]++;
	}
	// contract size
	while ( v[ len(v) ] == 0 )
		--v[0];
}


// multiplies n-limb long number u on single limb u
// returns carry 
limb_t mul_1( limb_t* r, const limb_t* u, size_t n, limb_t v )
{
	check( n >= 1 );
	
	limb_t cl = 0;
	do {
		limb_t ul = *u++;
		// umul( hp, lp, ul, v )
		wide_t mp = (wide_t)ul * (wide_t)v;
		limb_t lp = (limb_t)mp;
		limb_t hp = (limb_t)(mp >> BITS);

		lp += cl;
		cl = (lp < cl) + hp;
		
		*r++ = lp;
	} while( --n != 0 );

	return cl;	
}

// multiplies n-limb long number u on single limb u
// add n least significant limbs of the product to the vector p
// and return most significant limb of the product adjusted for
// carry-out from addition.
// assert if p is same or separate to u
limb_t add_mul_1( limb_t* p, const limb_t* u, size_t n, limb_t v )
{
	check( n >= 1 );
	// carry
	limb_t cl = 0;
	do {
		limb_t ul = *u++;

		//limb_t hp, lp;
		//umul( hp, lp, ul, v );
		wide_t mp = (wide_t)ul * (wide_t)v;
		limb_t lp = (limb_t)mp;
		limb_t hp = (limb_t)(mp >> BITS);
	
		lp += cl;				// add carry
		cl = (lp < cl) + hp;	// correct carry
		
		limb_t r = *p;			
		lp = r + lp;
		cl += lp < r;
		*p++ = lp;		

	} while (--n != 0 );
	return cl;
}

// basecase multiplication - { u, usize } vs { v, vsize }
// must have usize >= vsize
// usize+vsize limbs is stored
void mul_base( limb_t* r, const limb_t* u, size_t ulen, const limb_t* v, size_t vlen )
{
	check( ulen >= vlen );
	check( vlen >= 1 );

	// first we multiply the low order limb
	// and store directly in r
	r[ulen] = mul_1( r, u, ulen, v[0] );
	r++;
	v++;
	vlen--;

	// now accumulate the product of u[] and the next limb of v[]
	while ( vlen >= 1 ) {
		r[ulen] = add_mul_1( r, u, ulen, v[0] );
		//if ( ? == 1 ) return;
		r++, v++, vlen--;
	} 
}

bool short_mul( big& p, const big& q, limb_t d )
{
//	if ( len(q) > limbs_n ) {
//		return false;
//	}

	if ( d > 1 ) {
/*
		wide_t t = 0;
		for( size_t i = 1; i <= len(q); i++ ) {
			t += (wide_t)q[i] * (wide_t)d;
			p[i] = (limb_t)( t & BITMASK );
			t >>= BITS;
		}
		// carry left
		if ( t ) {
			p[0] = q[0] + 1;
			p[ len(p) ] = (limb_t)( t & BITMASK );
		} else {
			p[0] = q[0];
		}
*/
		limb_t t = mul_1( p.begin()+1, q.begin()+1, q[0], d );
		if ( t ) {
			p[0] = q[0] + 1;
			p[ len(p) ] = t;
		} else {
			p[0] = q[0];
		}
		
	} else if ( d /*==1*/ ) {
		copy( p, q );
	} else {
		p[0] = 0;
	}
	return true;
}

void mul( big& p, const big& u, const big& v )
{
	check( (len(u) + len(v)) <= u.max_size() );

	const limb_t* up = u.begin();
	const limb_t* vp = v.begin();

	// rearrange
	if ( vp[0] > up[0] ) {
		const limb_t* tp = up;
		up = vp; vp = tp;
	}

	if ( vp[0] == 0 ) {
		p[0] = 0;
	} else if (vp[0] == 1 && vp[1] == 1 ) {
		p.assign( up );
	} else {
		mul_base( p.begin()+1, up+1, up[0], vp+1, vp[0] );
		p[0] = up[0] + vp[0];
		if ( p[ len(p) ] == 0 )
			--p[0];
	}
}

//
// division is not optimized
//

void mod( big& u, const big& v )
{
	big t;
	size_t shift = 0;

	check( len(v) != 0 );

	copy( t, v );

	while( greater( u, t ) ) {
		shl( t, 1 );
		shift ++;
	}

	while( 1 ) {
		if ( greater( t, u ) ) {
			if ( shift ) {
				shr( t, 1 );
				shift --;
			} else break;
		} else {
			sub( u, t );
		}
	}
	clear( t );
}

// q - quotient or a / v
void div( big& q, const big& a, const big& b )
{
	big u;
	big v;
	big s;
	size_t shift = 0;

	check( len(b) != 0 );

	copy( v, b );
	copy( u, a );
	clear( q );
	s[0] = 1; s[1] = 1; 

	while( greater( u, v ) ) {
		shl( v, 1 );
		shl( s, 1 );
		shift ++;
	}

	while( 1 ) {
		if ( greater( v, u ) ) {
			if ( shift ) {
				shr( v, 1 );
				shr( s, 1 );
				shift --;
			} else break;
		} else {
			sub( u, v );
			add( q, s );
		}
	}
	clear( u );
	clear( v );
	clear( s );
}

void divrem( big& q, big& u, const big& a, const big& b )
{
	big v;
	big s;
	size_t shift = 0;

	check( len(b) != 0 );

	copy( v, b );
	copy( u, a );
	clear( q );
	s[0] = 1; s[1] = 1; 

	while( greater( u, v ) ) {
		shl( v, 1 );
		shl( s, 1 );
		shift ++;
	}

	while( 1 ) {
		if ( greater( v, u ) ) {
			if ( shift ) {
				shr( v, 1 );
				shr( s, 1 );
				shift --;
			} else break;
		} else {
			sub( u, v );
			add( q, s );
		}
	}
	clear( v );
	clear( s );
}


void mulmod( big& u, const big& v, const big& w, const big& m )
{
	big t;

	check( len(m) != 0 );
	clear( u );
	copy( t, w );

	// naive multiply by bits
	for( size_t i = 1; i <= len(v); i++ ) {
		for( size_t j = 0; j < BITS; j++ ) {
			if ( v[i] & (1U << j) ) {
				add( u, t );
				mod( u, m );
			}
			shl( t, 1 );
			mod( t, m );
		}
	}
}


// calculates modular inverse, 
// such r, as r * v = 1 mod m
// r, v in [ 1 .. m-1]
void modinv( big& r, const big& v, const big& m )
{
	check( !is_zero(m) );
	check( !is_zero(v) );
//	check( greater( m, v ) );

	big b, t0, t1, c, q, tt;

	clear( r );
	copy( b, v );
	word_set( t1, 1 );
	if ( is_one(b) ) {
		word_set( r, 1 );	
		return;
	}

	// t0 = a / b	
	//  c = a % b
	divrem( t0, c, m, b );

	while( !is_one(c) ) {
		// q = b / c
		// b %= c
		//printf(":%d\n", c[0]);
		//check( c[0] != 0 );
		if ( is_zero(c) ) return;
		divrem( q, tt, b, c );
		copy( b, tt );
		// t1 += q * t0
		mul( tt, q, t0 );
		add( t1, tt );
		// if ( b == 1 ) return t1
		if ( is_one(b) ) {
			copy( r, t1 );
			return;
		}
		// q = c / b
		// c %= b
		//check( b[0] != 0 );
		if ( is_zero(b) ) return;
		divrem( q, tt, c, b );
		copy( c, tt );
		// t0 += q * t1
		mul( tt, q, t1 );
		add( t0, tt );
	}
	// return a - t0
	copy( r, m );
	sub( r, t0 );
}

//
//
//

#ifdef TESTBIG

void rand( big& u )
{
	size_t n = 0;
	for( n = 1; n <= limbs_n; n++ ) 
		u[n] = (limb_t)::rand();
	for( n = limbs_n; n != 0; n-- )
		if( u[n] ) break;
	u[0] = n;
}

#define TEST( expr, desc ) \
	if ( !(expr) ) printf( "%s failed\n", desc ); \
	/* */

void big_test( size_t test_count )
{
	srand ((unsigned)(time(NULL) % 65521U));

	for( size_t n = 0; n != test_count; n++ ) {
		// mult add test
		big m, p, q, r, s;
		rand( m );
		short_mul( p, m, 3 );
		clear( q );
		add( q, m );
		add( q, m );
		add( q, m );
		TEST( equ( q, p ), "mul add" );
		// shift test
		copy( p, m );
		shl( p, n % 17 );
		shr( p, n % 17 );
		TEST( equ( p, m ), "shift" );
		// addition vs shift
		copy( p, m );
		add( p, p );
		copy( q, m );
		shl( q, 1 );
		TEST( equ( p, q ), "add shift" );
		// comparision	
		copy( p, m );
		add( p, p );
		TEST( !equ( p, m ), "equ" );
		TEST( greater( p, m ), "greater" );
		TEST( !greater( m, p ), "!greater" );
		TEST( !greater( m, m ), "!!greater" );
		// sub - shift
		copy( p, m );
		shl( p, 1 );
		sub( p, m );
		TEST( equ( p, m ), "sub shift" );
		// mulmod
		copy( p, m );
		word_set( r, 3 );
		mulmod( q, p, r, m ); // q = m*3 | m
		copy( p, m );
		add( p, p );
		add( p, m ); // p = m*3
		while ( greater( p, m ) || equ( p, m ) ) 
			sub( p, m );
		TEST( equ( q, p ), "mulmod" );
		// div1
		limb_t mult = (::rand() & 32767) + 1;
		word_set( r, mult );
		short_mul( p, m, mult );
		div( q, p, r );
		TEST( equ( q, m ), "div1" );
		// div2
		//limb_t mul1 = (::rand() & 32767) + 1;
		//limb_t mul2 = (::rand() & 32767) + 1;
		//r[0] = 2; r[1] = mul1; r[2] = mul2;
		//short_mul( p, m, mult );
		//div( q, p, r );
		//TEST( equ( q, m ), "div2" );

		// mul1
		q[0] = 1;
		q[1] = (::rand() & 32767) + 1;
		mul( r, m, q );
		short_mul( p, m, q[1] );
		TEST( equ( p, r ), "mul1" );
		// mul2
		limb_t mult2 = (::rand() & 32767) + 1;
		q[0] = 1;
		q[1] = mult2;
		short_mul( p, q, 54321 );
		mul( r, m, p );
		short_mul( p, m, 54321 );
		short_mul( q, p, mult2 );
		TEST( equ( q, r ), "mul2" );
		// mul div
		q[0] = 2;
		q[1] = mult2;
		q[2] = mult2+1;
		mul( p, m, q );
		div( r, p, q );
		TEST( equ( m, r ), "muldiv" );
		// divrem
		q[0] = 1;
		q[1] = mult2;
		divrem( r, p, m, q );
		short_mul( q, r, mult2 );
		add( q, p );
		TEST( equ( m, q ), "divrem" );
		// modinv 1
		//
		rand( q );
		mod( q, m ); 
		modinv( r, q, m );
		if ( r[0] != 0 ) {
			mulmod( p, r, q, m );
			TEST( (p[0] == 1 && p[1] == 1), "modinv" );
		}
		// modinv 65537
		q[0] = 2;
		q[1] = 1;
		q[2] = 1; // 65537
		copy( p, m );
		mod( p, q );
		if ( p[0] != 0 ) {
			modinv( r, p, q );
			mulmod( s, p, r, q );
			TEST( (s[0] == 1 && s[1] == 1), "modinv65537" );
		}
	}
}
#endif

} // xxc

#ifdef TESTBIG
int main()
{
	xxc::big_test( 3000 );
	printf("done..\n");
	return 0;
}
#endif //TESTBIG

