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
// Galois multiplicative group base 2m algebra

#include "stdafx.h"

#include "xxc.cfg.h"
#include "xxc.gf2.h"

namespace xxc {

static const size_t BASE	= 1U << cfg::L;
static const unit_t TOGGLE  = BASE - 1;

static unit_t* etab = NULL;
static unit_t* ltab = NULL;

static poly_t tmask;
static poly_t nzt;

size_t slow_trace( const poly_t& p );

void init()
{
	if ( etab != NULL && ltab != NULL ) return;

	etab = (unit_t*)malloc( BASE * sizeof( unit_t ) );
	ltab = (unit_t*)malloc( BASE * sizeof( unit_t ) );

	wide_t root = BASE | cfg::RP;
	etab[0] = 1;

	size_t i,j;
	for( i = 1; i != BASE; i++ ) {
		j = (wide_t)etab[i-1] << 1;
		if ( j & BASE ) j ^= root;
		etab[i] = (unit_t)j;
	}

	for( i = 0; i != TOGGLE; i++ ) {
		ltab[ etab[i] ] = (unit_t)i;
	}
	ltab[0] = TOGGLE;

	// calculate trace mask
	poly_t x;
	for( i = 1; i <= cfg::K; i++ ) {
		x[0] = i;
		size_t m = 1;
		size_t w = 0;
		for( j = 0; j < cfg::L; j++ ) {
			x[i] = m;
			if ( slow_trace( x ) ) 
				w ^= m;
			m *= 2;
		}
		x[i] = 0;
		tmask[i] = w;
		if ( w ) tmask[0] = i;
	}

	// find nzt
	if ( (cfg::M&1) == 0 ) {
		nzt[0] = 1;
		nzt[1] = 1;
	    while ( (nzt[1] & tmask[1]) == 0 )
    	  nzt[1] *= 2;
	}
}

void fini()
{
	if ( etab != NULL ) {
		free( etab ); etab = NULL;
	}
	if ( ltab != NULL ) {
		free( ltab ); ltab = NULL;
	}
}

void dump( const big& v, const char* tag )
{
	big val,ten, digit, quotient;
	copy( val, v );
	word_set( ten, 10 );
	do {
		divrem( quotient, digit, val, ten );
		copy( val, quotient );
		printf("%d", digit[0] ? digit[1] : 0 );
	} while ( val[0] != 0 );
	printf("[%d] - %s\n", v[0], tag );
}

void dump2( const big& v, const char* tag )
{
	if ( v[0] == 0 ) {
		printf("{0} ");
	} else {
		for( size_t n = 0; n != v[0]; ++n ) {
			printf("%04x.",v[v[0]-n] );
		}
	}
	printf("[%d] - %s\n", v[0], tag );
}

#if 0
void luca( big& r, const big& P, const big& Z, size_t ik )
{
	big V0, V2, tmp;
	word_set( V0, 2 );
	copy( r, P );

	while( ik > 1 ) {
		mul( V2, P, r );   // v2 = P*V1
		mul( tmp, Z, V0 ); // tmp = Z*V0
		dump2( V2, "V2"); 
		dump2( tmp, "tmp");
		check( greater( V2, tmp ) );
		sub( V2, tmp );   // v2 -= tmp
		dump( V2, "sub");
		copy( V0, r );
		copy( r, V2 );
		ik--;
	}

	clear( tmp );
	clear( V2 );
}

void pow2( big& p, size_t n )
{
	//if ( n >= (len(a) << LOG2_BITS) ) return 0;
	p.clear();
	// bits aware!
	p[(n >> 4) + 1] |= 1 << (n&15);
	p[0] = 1+(n>>4);
}

void modulus( big& p )
{
	const size_t SQRT_BASE = 1<<((cfg::L+1)/2);
	const size_t soMin = BASE - 2*SQRT_BASE;
	const size_t so = soMin +2*cfg::NSO;
	big tmp,sf;
	word_set( tmp, cfg::NTF );
	short_mul( sf, tmp, so );

	big Z, Zp;
	pow2( Z, cfg::L );
	check( (so-1) < 65536 );
	word_set( tmp, so-1 );
	copy( Zp, Z );
	//printf("%d %d, %d\n", cfg::L, Zp[1], (so-1) );
	sub( Zp, tmp );
	luca( tmp, Zp, Z, cfg::K );

	pow2( p, cfg::M );
	p[1]++;
	sub( p, tmp );
	div( tmp, p, sf );

	copy( p, tmp );

	clear( tmp );
	clear( Zp );
	clear( sf );
}
#endif

//
//
bool equ( const poly_t& a, const poly_t& b )
{
	return memcmp( a.begin(), b.begin(), sizeof(unit_t)*(a[0] + 1) ) ? false : true;
}

void clear( poly_t& p )
{
	p.clear();
}

void copy( poly_t& d, const poly_t& s )
{
	memcpy( d.begin(), s.begin(), (s[0] + 1)*sizeof( unit_t ) );
}

void bin_set( poly_t& v, const unit_t* w )
{
	memcpy( v.begin(), w, sizeof(unit_t)*(w[0] + 1) );
//	size_t n = v[0] = *w++;
//	for ( size_t j = 0; j != n; j++ ) 
//		v[j] = *w++;
}

//
//
void add( poly_t& r, const poly_t& a, const poly_t& b )
{
	check( ltab != NULL && etab != NULL );

	if ( a[0] > b[0] ) {
		// xor common-degree coeff
		size_t n;
		for( n = 1; n <= b[0]; n++ )
			r[n] = a[n] ^ b[n];
		// n == b[0] + 1;
		memcpy( &r[n], &a[n], (a[0] - b[0]) * sizeof(unit_t) );
		// deg(p) gets the deg(r)
		r[0] = a[0];
	} else if ( a[0] < b[0] ) {
		// xor common-degree coeff
		size_t n;
		for( n = 1; n <= a[0]; n++ )
			r[n] = a[n] ^ b[n];
		// n == a[0] + 1;
		memcpy( &r[n], &b[n], (b[0] - a[0]) * sizeof(unit_t) );
		// deg(p) gets the deg(r)
		r[0] = b[0];
	} else {
		// scan to get deg(r)
		size_t n;
		for( n = a[0]; n > 0; n-- ) {
			if ( a[n] ^ b[n] ) break;
		}
		// xor the common part
		for( r[0] = (unit_t)n; n > 0; n-- ) 
			r[n] = a[n] ^ b[n];
	}
}

// reduces p to irreducible trinominal x^K + x^T + 1
static void
reduce( poly_t& p )
{
	for( size_t n = p[0]; n > cfg::K; n-- ) {
		p[n - cfg::K] ^= p[n];
		p[n + cfg::T - cfg::K] ^= p[n];
		p[n] = 0;
	}
	if ( p[0] > cfg::K ) {
		// scan contract length
		p[0] = cfg::K;
		while (p[0] && p[ p[0] ] == 0 ) 
			p[0]--;
	}
}

//
void
mul( poly_t& r, const poly_t& a, const poly_t& b )
{
	check( ltab != NULL && etab != NULL );
	//check( r != b );
	//check( r != a );
	size_t i,j;
	// protected stack vars, should be cleared
	wide_t x, log_ai, log_bj;
	unit_t lg[ cfg::K + 2 ];
	// 

	if ( a[0] && b[0] ) {
		// precompute ltab[b[j]]
		for( j = b[0]; j; j-- ) 
			lg[j] = ltab[ b[j] ];
		// multiply
		clear( r );
		for( i = a[0]; i; i-- ) {
			if ( (log_ai = ltab[a[i]]) != TOGGLE ) {
				for( j = b[0]; j; j-- ) {
					if ( (log_bj = lg[j]) != TOGGLE ) {
						x = log_ai + log_bj;
						r[i+j-1] ^= etab[ x >= TOGGLE ? x - TOGGLE : x ];
					}
				}
			}
		}
		r[0] = a[0] + b[0] - 1;
		// reduce 
		reduce( r );
	} else {
		// = 0
		r[0] = 0;
	}

	// zap data
	x = log_ai = log_bj = 0;
	memset( lg, 0, sizeof(lg) );
}

void
sqr( poly_t& r, const poly_t& p )
{
	check( ltab != NULL && etab != NULL );

	if ( p[0] ) {
		wide_t x;
		size_t i = p[0];
		// in what follows, note that (x != 0) => (x^2 = exp((2 * log(x)) % TOGGLE)):
		if ( (x = ltab[ p[i] ]) != TOGGLE ) {
			r[2*i - 1] = etab[(x +=x) >= TOGGLE ? x - TOGGLE : x ];
		} else {
			r[2*i - 1] = 0;
		}
		for( i = p[0] - 1; i; i-- ) {
			r[2*i] = 0;
			if ( (x = ltab[ p[i] ]) != TOGGLE ) {
				r[2*i - 1] = etab[ (x+=x) >= TOGGLE ? x - TOGGLE : x ];
			} else {
				r[2*i - 1] = 0;
			}
		}
		r[0] = 2*p[0] - 1;
		//
		reduce( r );
	} else {
		r[0] = 0;
	}
}

void
small_div( poly_t& p, unit_t b )
{
	check( ltab != NULL && etab != NULL );
	check( b != 0 );
	check( b < BASE );

	unit_t lb = ltab[b];
	for( size_t i = p[0]; i; i-- ) {
		wide_t x;
		if (( x = ltab[ p[i] ] ) != TOGGLE ) {
			p[i] = etab[ (x+= TOGGLE - lb) >= TOGGLE ? x - TOGGLE : x ];
		}
	}	
}

static
void addmul( poly_t& a, wide_t alpha, wide_t j, poly_t& b )
{
	check( alpha < BASE );
	wide_t x, la = ltab[alpha];
	unit_t* aj = &a[j];
	// extend 'a'
	while( a[0] < j + b[0] ) {
		a[0]++;
		a[a[0]] = 0;
	}
	// mul add
	for( size_t i = b[0]; i; i-- ) {
		if ( (x = ltab[ b[i] ]) != TOGGLE ) {
			aj[i] ^= etab[ (x+=la) >= TOGGLE ? x - TOGGLE : x ];
		}
	}
	// contract
	while( a[0] && a[ a[0] ] == 0 ) 
		a[0]--;
}

bool
inv( poly_t& b, const poly_t& a )
{
	check( ltab != NULL && etab != NULL );
	//check( a != b );

	// 0 is non invertible element in GF
	if ( a[0] == 0 ) {
		check( 0 );
		return false;	
	}

	// setup
	poly_t	c, f, g;
	b[0] = 1;	b[1] = 1;
	c[0] = 0;
	copy( f, a );
	clear( g );
	g[0] = cfg::K + 1;
	g[1] = 1;
	g[cfg::T+1] = 1;
	g[cfg::K+1] = 1;

	wide_t  x, j, alpha;
	while (1) {
		// is 'f' small enough? divide directly!		
		if ( f[0] == 1  ) {
			check( f[1] != 0 );
			small_div( b, f[1] );
			// cleanup data
			clear( c ); clear( f );	clear( g );
			x = j = alpha = 0;
			return true;
		}
		// exchange b,c,f,g
		if ( f[0] < g[0] ) {	
			while (1) {
				check( g[0] >= f[0] );
				j = g[0] - f[0];
				x = ltab[ g[g[0]] ] - ltab[ f[f[0]] ] + TOGGLE;
				alpha = etab[ x >= TOGGLE ? x - TOGGLE : x ];

				addmul( g, alpha, j, f );
				addmul( c, alpha, j, b );
				// we're done?
				if ( g[0] == 1 ) {
					check( g[1] != 0 );
					small_div( c, g[1] );	
					copy( b, c );
					// cleanup data
					clear( c ); clear( f );	clear( g );
					x = j = alpha = 0;
					return true;
				}
				// should we swap again?
				if ( f[0] > g[0] ) break;
			}
		}
		//
		check( f[0] >= g[0] );
		j = f[0] - g[0];
		x = ltab[ f[f[0]] ] - ltab[ g[g[0]] ] + TOGGLE;
		alpha = etab[ x >= TOGGLE ? x - TOGGLE : x ];
		addmul( f, alpha, j, g );
		addmul( b, alpha, j, c );
	}
	// unreachable
}


void
sqrt( poly_t& p, unit_t b )
{
	check( ltab != NULL && etab != NULL );
	check( b < BASE );

	if ( b == 0 ) {
		clear( p );
		return;
	}

	poly_t q;
	q[0] = 1;
	q[1] = b;	

	size_t i;
	if ( (cfg::M - 1) & 1 ) {
		// M-1 is odd
		sqr( p, q );
		i = cfg::M - 2;
	} else {
		// M-1 is even
		copy( p, q );
		i = cfg::M - 1;
	}

	check( (i&1) == 0 );
	while( i ) {
		sqr( p, p );
		sqr( p, p );
		i -= 2;
	}
}

// George Barwood fast trace algorithm
size_t __trace( const poly_t& p )
{
	check( ltab != NULL && etab != NULL );

	// ifdef
	if ( cfg::TM0 == 1 && cfg::TM1 == 1 ) {
		return p[0] ? p[1] & 1 : 0;
	}	
	
	if ( p[0] ) {
		unit_t w;

		w = p[1] & cfg::TM1;
		// ifdef
		if ( cfg::TM0 != 1 && p[0] <= cfg::TM0 ) {
			w ^= p[cfg::TM0] & cfg::TM2;
		}

		// compute parity of w (assume its uint16)
#if 1
		for ( size_t i = sizeof(unit_t)*4; i != 0; i >>= 1) {
			w ^= w >> i;
		}
		return w & 1;
#elif 0
		 w ^= w >> 8;
		 w ^= w >> 4;
		 w &= 0x0f;
		 w = (0x6996 >> w) & 1;
		 return w;
#else
		size_t parity = 0;
		while( w ) {
			parity = !parity;
			w = w & (w-1);
		}
		return parity;
#endif
	} else {
		return 0;
	}
}


size_t trace( const poly_t& p )
{
	check( ltab != NULL && etab != NULL );

	size_t m = p[0];
	if ( m > cfg::TM0 ) m = cfg::TM0;

	unit_t w = 0;
	size_t i;
	for( i = 1; i <= m; i++ )
		w ^= p[i] & tmask[i];

	for ( i = sizeof(unit_t)*4; i != 0; i >>= 1) 
		w ^= w >> i;
	return w & 1;
}

// p^2 + p = beta
bool
qsolve( poly_t& p, const poly_t& beta )
{
	check( ltab != NULL && etab != NULL );
	//check( p != beta );

	// check if solution exists
	if ( trace( beta ) != 0 ) 
		return false; // no solution!

	// ifdef
	if ( (cfg::M&1) == 0 ) {
		poly_t d, t;
		p[0] = 0;
		copy( d, beta );
//		nzt[0] = 1;
//		nzt[1] = cfg::NZT;
		check( trace(nzt) != 0 );

		for( size_t n = 1; n < cfg::M; n++ ) {
			sqr( p, p );
			sqr( d, d );
			mul( t, d, nzt );
			add( p, p, t );
			add( d, d, beta );
		}
		// zap temps
		clear( d );
		clear( t );
	} else { 
		copy( p, beta );
		// M is odd - compute half-trace
		poly_t t1, t2;
		for( size_t n = 0; n < cfg::M/2; n++ ) {
			sqr( t1, p );
			reduce( t1 );
			sqr( t2, t1 );
			reduce( t2 );
			add( p, t2, beta );
		}
		clear( t1 );
		clear( t2 );
	}
	return true;	
}

bool
ybit( const poly_t& p ) 
{
	return p[0] ? (bool)(p[1] & 1) : 0;
}


void	pack( big& k, const poly_t& p )
{
	check( cfg::L <= 16 );
	clear( k );

	big a;
	a[0] = 1;
	for( size_t n = p[0]; n != 0; n-- ) {
		shl( k, cfg::L );
		a[1] = p[n];
		add( k, a );
	}
}

void	unpack( poly_t& p, const big& k )
{
	check( cfg::L <= 16 );
	big x;
	
	copy( x, k );
	size_t n = 0;
	for( ; x[0]; n++ ) {
		p[n+1] = (unit_t)( x[1] & TOGGLE);
		shr( x, cfg::L );
	}	
	p[0] = (unit_t)n;
}

// slow trace is used in intitializatoin
size_t slow_trace( const poly_t& p )
{
	poly_t t;

	copy( t, p );
	for( size_t n = 1; n < cfg::M; n++ ) {
		poly_t t2;
		sqr( t2, t );
		reduce( t2 );
		add( t, t2, p );
	}
	return t[0] != 0;
}


//
// SELF testing rountines

#ifdef TESTGF2
void rand( poly_t& p )
{
	p[0] = cfg::K;
	for( size_t n = 1; n <= cfg::K; n++ ) {
		p[n] = (unit_t)( ::rand() & TOGGLE );
	}	
	while( p[0] && p[ p[0] ] == 0 )
		p[0]--;
}

#define TEST( expr, desc ) \
	if ( !(expr) ) printf( "%s failed\n", desc ); \
	/* */

//
int gf2_selftest( size_t numtest )
{
	poly_t	f, g, h, x, y, z;
	unit_t  b;

	srand( (unsigned)(time(NULL) % 65521U) );
	for( size_t n = 0; n != numtest; n++ ) {
		rand( f );
		rand( g );
		rand( h );

		// addition
		add( x, f, g );
		add( y, g, f );
		TEST( equ( x, y ), "addition" );

		// multiplication
		mul( x, f, g );
		mul( y, g, f );
		TEST( equ( x, y ), "multiplication" );

		// mult test
		mul( z, f, g );
		mul( x, z, f ); // f*g*f
		mul( z, f, f ); 
		mul( y, z, g ); // f*f*g
		TEST( equ(x,y), "multiplication2" );

		// distribution
		mul( x, f, g );
		mul( y, f, h );
		add( y, x, y );
		add( z, g, h );
		mul( x, f, z );
		TEST( equ( x, y ), "distribution" );

		// squaring
		sqr( x, f );
		mul( y, f, f );
		TEST( equ( x, y ), "squaring" );

		// inversion
		if ( g[0] ) {
			inv( x, g );
			mul( y, f, x );
			mul( x, g, y );
			TEST( equ( x, f ),  "inversion" );
		}

		// square root
		b = (unit_t)::rand() & TOGGLE;
		if ( b ) {
			z[0] = 1; z[1] = b;
		} else {
			z[0] = 0;
		}
		sqrt( y, b );
		sqr( x, y );
		TEST( equ( x, z ), "square root" );

		// trace
		TEST( (trace(f) == slow_trace(f)), "trace" );

		// quad test
		if ( trace(f) == 0 ) {
			qsolve( x, f );
			sqr( y, x );
			add( y, y, x );
			TEST( equ( y, f ), "quadric" );
		}
	}

	return 0;
}
#endif //TESTGF2

} // xxc


#ifdef TESTGF2
int main()
{
	xxc::init();
	xxc::gf2_selftest( 10000 );
	xxc::fini();
	printf("done..\n");

	return 0;
}
#endif //TESTGF2

