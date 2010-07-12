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
// Elliptic curve algebra

#include "stdafx.h"

#include "xxc.cfg.h"
#include "xxc.ecp.h"

namespace xxc {

void clear( point_t& p )
{
	clear( p.x );
	clear( p.y );
}

void copy( point_t& d, const point_t& s )
{
	copy( d.x, s.x );
	copy( d.y, s.y );
}

bool equ( const point_t& a, const point_t& b )
{
	return equ( a.x, b.x ) && equ( a.y, b.y );
}

// given x find y such as y^2 + x*y = x^3 + B;
bool calc_y( point_t& p, bool yb )
{
	poly_t a, b, t;
	
	// set b
	b[0] = 1;
	b[1] = cfg::ECB;
	// simple case x=0 -> y^2 = B
	if ( p.x[0] == 0 ) {	
		sqrt( p.y, cfg::ECB );
		return true;
	}
	// find alpha = x^3 + b = (x^2)*x + b
	sqr( t, p.x );
	mul( a, t, p.x );
	add( a, a, b );
	// if a == 0 -> y = 0
	if ( a[0] == 0 ) {
		p.y[0] = 0;
		clear( a );
		clear( t );
		return true;
	}
	// find beta = alpha/x^2 = x + ECB/x^2
	small_div( t, cfg::ECB );
	inv( a, t );
	add( a, p.x, a );
	// check solution existance
	if ( trace( a ) != 0 ) {
		// no solution
		clear( a );
		clear( t );
		return false;
	}
	// solve t^2 + t + beta = 0 so ybit(t) == ybit
	qsolve( t, a );
	if ( ybit( t ) != yb ) {
		t[1] ^= 1;
	}
	// y = x * t
	mul( p.y, p.x, t );
	clear( a );
	clear( t );
	return true;
}

void add( point_t& p, const point_t& q )
{
	poly_t lambda, t, tx, ty, x3, y3;
	
	// first check if q != 0
	if ( q.x[0] != 0 || q.y[0] != 0 ) {
		if ( p.x[0] != 0 || p.y[0] != 0 ) {
			// p != 0 && q != 0
			if ( equ( p.x, q.x ) ) {
				// either p == q or p == -q
				if ( equ( p.y, q.y ) ) {
					// equal
					dbl( p );
				} else {
					// inverse
					// assert q.y = p.x + p.y
					p.x[0] = p.y[0] = 0;
				}
			} else {
				// p != 0, q != 0, p != q, p != -q
				// lambda = (y1 + y2)/(x1 + x2)
				add( tx, p.x, q.x );
				add( ty, p.y, q.y );
				inv( t, tx );
				mul( lambda, ty, t );
				// x3 = lambda^2 + lambda + x1 + x2
				sqr( x3, lambda );
				add( x3, x3, lambda );
				add( x3, x3, tx );
				// y3 = lambda*(x1 + x3) + x3 + y1
				add( tx, p.x, x3 );
				mul( t, lambda, tx );
				add( t, t, x3 );
				add( p.y, t, p.y );
				// set p.x
				copy( p.x, x3 );
			}
		} else {
			// p = q
			copy( p.x, q.x );
			copy( p.y, q.y );
		}
	}
}

void sub( point_t& p, const point_t& q )
{
	// p - q = p + {q.x, q.x+q.y }
	point_t t;	
	copy( t.x, q.x );
	add( t.y, q.x, q.y );
	add( p, t );
}

void neg( point_t& p )
{
	add( p.y, p.x, p.y );
}

void dbl( point_t& p )
{
	poly_t lambda, t1, t2;
	// lambda = x + y / x
	inv( t1, p.x );
	mul( lambda, p.y, t1 );
	add( lambda, lambda, p.x );
	// x3 = lambda^2 + lambda
	sqr( t1, lambda );
	add( t1, t1, lambda );
	// y3 = x^2 + lambda*x3 + x3
	sqr( p.y, p.x );
	mul( t2, lambda, t1 );
	add( p.y, p.y, t2 );
	add( p.y, p.y, t1 );

	copy( p.x, t1 );
}

void dump( const poly_t& b, char* tag ) 
{
	printf("%s : ", tag );
	for( size_t n = b[0]; n != 0; n-- ) 
		printf("%04x", b[n] );
	//printf(" : %d\n", numbits(b) );
}

// poor man bitwise multiplication
void mul( point_t& p, const big_t& k )
{
	big_t h;
	point_t r;

	// check integer to be in normalized form
	check( k[k[0]] || (k[0] ==0));
	
	copy( r, p );
	p.x[0] = 0;
	p.y[0] = 0;
	short_mul( h, k, 3 );

	size_t z = numbits( h ) - 1;
	size_t i = 1;
	for( ;; ) {
		size_t hi = bit( h, i );
		size_t ki = bit( k, i );
		if ( hi == 1 && ki == 0 ) {
			add( p, r );
		} else if ( hi == 0 && ki == 1 ) {
			sub( p, r );
		}
		if ( i >= z ) break;
		i++;
		dbl( r );
	}
}

bool ybit( const point_t& p )
{
	poly_t t1, t2;

	if ( p.x[0] == 0 ) {
		return false;
	} else {
		inv( t1, p.x );
		mul( t2, p.y, t1 );
		return ybit( t2 );
	}
}

void pack( big& k, const point_t& p )
{
	big a;

	if ( p.x[0] ) {
		pack( k, p.x );
		shl( k, 1 );
		word_set( a, (limb_t)ybit(p) );
		add( k, a );
	} else if ( p.y[0] ) {
		word_set( k, 1 );
	} else {
		k[0] = 0;
	}
}

void unpack( point_t& p, const big& k )
{
	bool yb;
	big a;

	copy( a, k );
	yb = a[0] ? bit( a, 0 ) : 0;
	shr( a, 1 );
	unpack( p.x, a );

	if ( p.x[0] || yb ) {
		calc_y( p, yb );
	} else {
		p.y[0] = 0;
	}
}

//
///////////////////////////////////////////////////////////

#ifdef TESTECP

void rand( point_t& p )
{
	bool check;

	do {
		rand( p.x );
		check = calc_y( p, 0 );
	} while ( !check );
}

void dump( big& b, char* tag )
{
	printf("%s : ", tag );
	for( size_t n = b[0]; n != 0; n-- ) 
		printf("%04x", b[n] );
	printf(" : %d\n", numbits(b) );
}

#define TEST( expr, desc ) \
	if ( !(expr) ) printf( "%s failed\n", desc ); \
	/* */

point_t eP;


#ifndef ECC70
limb_t eppx[] = {5U, 0x0324U, 0x0e80U, 0x0272U, 0x01e1U, 0x061fU };
limb_t eppy[] =	{5U, 0x0aaeU, 0x007cU, 0x0a3aU, 0x0449U, 0x0511U };

limb_t prime_order[] = {4U, 0x26cdU, 0x0a48U, 0x0668U, 0x0001U};
#else
limb_t eppx[] = {7U, 0x0034U, 0x01bcU, 0x027cU, 0x00faU, 0x021cU, 0x0007U, 0x016eU, };
limb_t eppy[] =	{7U, 0x02c0U, 0x036bU, 0x025eU, 0x0166U, 0x00b1U, 0x0379U, 0x01b4U, };

limb_t prime_order[] = {4U, 0x244bU, 0x1227U, 0x1010U, 0x1010U};
#endif

int pnt_selftest( size_t count )
{
	srand ((unsigned)(time(NULL) % 65521U));
	printf("doing ecp %d times\n", count );
	for( size_t n = 0; n != count; ++n ) {
		point_t 	 f,g,x,y;
		big_t 		 m,k;
		rand( f );
		rand( g );
		rand( m );
		rand( k );
		bin_set( eP.x, eppx );
		bin_set( eP.y, eppy );

		// neg test
		copy( x, f );
		neg( x );
		neg( x );
		TEST( equ( x, f ), "neg" );

		// add
		copy( x, f );
		add( x, g );
		copy( y, g );
		add( y, f );
		TEST( equ( x, y ), "add" );

		// sub
		copy( x, f );
		sub( x, g );
		copy( y, g );
		neg( y );
		add( y, f );
		TEST( equ( x, y ), "sub" );

		// quad
		copy( x, f );
		dbl( x );
		dbl( x );
		clear( y );
		add( y, f );
		add( y, f );
		add( y, f );
		add( y, f );
		TEST( equ( x, y ), "dbl" );

		// commutativity
		copy( x, f );
		copy( y, f );
		mul( x, k );
		mul( x, m );
		mul( y, m );
		mul( y, k );
		TEST( equ( x, y ), "commutativity" );

		// y test
		bool yb = ybit( f );
		clear( x );
		copy( x.x, f.x );
		calc_y( x, yb );
		TEST( equ( x, f ), "y bit" );

		// pack unpack
		pack( m, f );
		//dump( m, "val" );
		unpack( x, m );
		TEST( equ( f, x ), "pack" );

		// crypt - decrypt IES
		big pk, puk;
		point_t tp;
		rand( pk );
		copy( tp, eP );
		mul( tp, pk );
		pack( puk, tp ); 

		point_t ss, sr, S, D, tt;
		rand( S );
		copy( tt, eP );
		mul( tt, pk );
		copy( ss, tt );	//
		copy( tt, tp ); 
		mul( tt, pk );
		add( tt, S );
		copy( sr, tt );
		// S, ss, sr
		copy( tt, ss );
		mul( tt, pk );
		copy( D, sr );
		sub( D, tt );
		//
		TEST( equ( S, D ), "codec" );

//		pack( m, sr );
//		dump( m, "sr" );
//		pack( m, ss );
//		dump( m, "ss" );

		// ecDSA
		//modulus( mn );
		big hashE, mn;
		bin_set( mn, prime_order );
		// e1: e=hash
		word_set( hashE, 12345 );
		shl( hashE, 2 );
		//copy( k, mn );
		//sub( k, hashE );
		//copy( hashE, k );

		// e2: k = [1..n-1]
		big kval;
		point_t exy;
		kval[0] = 0;
		while( kval[0] == 0 ) rand( kval );
		// e3: exy = kG
		bin_set( eP.x, eppx );
		bin_set( eP.y, eppy );
		copy( tt, eP );
		mul( tt, kval );
		// e4: r = exy.x | n
		big r,s,ttt,tmp;
		pack( r,tt.x );
		mod( r, mn );
		// mod?
		check( r[0] != 0 );
		// e5: s = k^-1( e + r*pk )
		mulmod( tmp, pk, r, mn );
		copy( s, hashE );
		add( s, tmp );
		mod( s, mn );
		modinv( tmp, kval, mn );
		mulmod( ttt, s, tmp, mn );
		copy( s, ttt );
		check( s[0] != 0 );
		// e6: (r,s)

		// d1: e=hash
		// d2: w = s^-1 | n
		big w;
		modinv( w, s, mn );
		// d3: u1 = ew | n
		big u1, u2;
		mulmod( u1, hashE, w, mn );
		// d3: u2 = rw | n
		mulmod( u2, r, w, mn );
		// d4: (x,y) = u1G + u2Qa, Qa = unpack(puk)
		//point_t tp
		unpack( tp, puk );
		mul( tp, u2 );
		copy( tt, eP );
		mul( tt, u1 );
		add( tt, tp );
		// d5: test tt.x == r | n
		pack( tmp, tt.x );
		mod( tmp, mn );
		TEST( equ( r, tmp ), "DSA" );

		if ( n < 20 ) {
			dump( r, "sr" );
			dump( s, "ss" );
		}
	}	
	printf("done...\n");
	return 0;
}

#endif //TESTECP

} // xxc

#ifdef TESTECP
int main()
{
	xxc::init();
	xxc::pnt_selftest( 1234 );
	xxc::fini();
	return 0;
}
#endif //TESTECP
