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

// serials & bloom table generator
// ----------------------------------------------
// added array trashing
// testing specific key pattern
// added unique bits per key
// added KEY xoring
// added val redistribution

#include <cstdio>
#include <cstdlib>
#include <cassert>
#include <string.h>
#include "xxc.cfg.h"
#include "xxc.bswap.h"
#include "xxc.ecp.h"
#include "xxc.shash.h"
#include "xxc.defs.h"
#include "xxc.bloom.h"

#include <vector>
#include <set>
#include <algorithm>
#include <time.h>

#define BLOOM_HSIZE	(XXC_BLOOM_PHASES * XXC_BLOOM_BITS)


using namespace xxc;

// encoding bloom functions
/////////////////

#define BMASK( x ) (1UL<<(x))

void bloom_setbit( void* buf, size_t bitn )
{
	uint8* p	= (uint8*)buf + (bitn>>3);
	*p |= BMASK( bitn & 7 );
}

void bloom_resetbit( void* buf, size_t bitn )
{
	uint8* p	= (uint8*)buf + (bitn>>3);
	*p &= ~(BMASK( bitn & 7 ));
}

bool bloom_put_unq( void* buf, uint32 val, uint32 key, void* used, void* unq, bool check_used = true )
{
	uint32 img[16];
	size_t pos[XXC_BLOOM_PHASES];

	bloom_hash( val, img );

	size_t unique = XXC_BLOOM_PHASES;
	size_t phase;

	// hide the key value
	key ^= bloom_mix( val, img[0] );

	// test bits
	for( phase = 0; phase != XXC_BLOOM_PHASES; phase++ ) {
		uint32 ndx = bloom_slice( img, phase );
		assert( ndx < XXC_BLOOM_POWER );
		pos[phase] = ndx;

		if ( bloom_checkbit( used, ndx ) ) {	
//			printf(":%06x", ndx );
			unique--;
		}

	}

	if ( check_used ) {	
//		if ( unique < BLOOM_UNQ ) return false;
		if ( unique < XXC_BLOOM_PHASES ) return false;

		// self intersection test!
		for( size_t i = 0; i != XXC_BLOOM_PHASES-1; i++ ) {
			for( size_t j = i+1; j != XXC_BLOOM_PHASES; j++ ) {
				if ( pos[i] == pos[j] ) return false;
			}
		}
	}

	// mark the bits
	uint32 msk = 1 << (XXC_BLOOM_PHASES-1);
	for( phase = 0; phase != XXC_BLOOM_PHASES; phase++ ) {
		uint32 ndx = pos[phase];
		if ( key & msk ) {
			bloom_setbit( buf, ndx );
		} else {
			bloom_resetbit( buf, ndx );
		}
		msk >>= 1;
		bloom_setbit( used, ndx );
	}

	return true;
}

void bloom_trash( void* buf, const void* used, size_t bytes, uint32 seed = 123 )
{
	uint8* 			p = (uint8*)buf;
	const uint8*    u = (const uint8*)used;


	for( size_t n = 0; n != bytes; n++ ) {
		seed = (1812433253UL * (seed ^ (seed >> 30)) + n);
		// u - marks used bits - do not touch them
		size_t rshift = ((seed ^ 0x37DF1619) >> 11) & 31;
		uint8 trash = XXC_ROTL32C( 0xA3109750, rshift );
		*p++ |= (trash & ~(*u++) );
	}
}

size_t bitcount( uint32 v )
{
	v = v - ((v >> 1) & 0x55555555);                    // reuse input as temporary
	v = (v & 0x33333333) + ((v >> 2) & 0x33333333);     // temp
	return ((v + (v >> 4) & 0xF0F0F0F) * 0x1010101) >> 24; // count
}

size_t no_set( const void* ptr, size_t bytes )
{
	const uint32* p = (const uint32*)ptr;
	size_t count = 0;
	for( size_t n = 0; n != bytes/4; n++ ) {
		count += bitcount( p[n] );
	}	
	return count;
}

class tester
{
public:
	typedef std::vector<uint32> uvec;
	typedef std::set<uint32> uset;
	
	uset  reg;
	void* buf;
	void* usd;
	void* unq;

	size_t	keys_tried;
	size_t	keys_failed;
	size_t	keys_collided;

	uint32	key_img;

	bool is_in( uint32 key )
	{ return reg.find(key) != reg.end(); }
	void ban_( uint32 key )
	{ reg.erase( reg.find(key) ); }

	tester( uint32 k )
	{
		key_img = k;
		//assert( BLOOM_HSIZE <= 512 );
		if ( BLOOM_HSIZE > 512 ) 
			printf("!!! WARN !!! PHASES*BITS is too big, reusing hash\n");
		printf("alloc bloom array of %d Kb\n", XXC_BLOOM_SIZE / 1024 );
		buf = malloc( XXC_BLOOM_SIZE );
		usd = malloc( XXC_BLOOM_SIZE );
		unq = malloc( XXC_BLOOM_SIZE );
		memset( buf, 0, XXC_BLOOM_SIZE );
		memset( usd, 0, XXC_BLOOM_SIZE );
		memset( unq, 0, XXC_BLOOM_SIZE );

		keys_tried = 0;
		keys_failed= 0;
		keys_collided = 0;
	}

	~tester()
	{
		free(unq);
		free(usd);
		free(buf);
	}

	void add( uint32 key )
	{
		keys_tried++;
		if ( is_in( key ) ) {
			keys_collided++;
		} else if ( bloom_put_unq( buf, key, key_img, usd, unq ) ) {
			reg.insert( key );
//			printf("%d submitted\n",key);
		} else {
//			printf("can't add %d key\n", key );
			keys_failed++;
		}
	}

	bool blacklist( uint32 key )
	{
		printf("blacklisting: %08x : ", key );
		if ( !is_in( key ) ) {	
			printf("not found!\n");
			return false;
		}
		bool correct = true;
		while ( correct ) {
			// gen invalid kimg
			uint32 invalid_kimg = clock() & XXC_BLOOM_IMASK;
			// put
			bloom_put_unq( buf, key, invalid_kimg, usd, unq, false );
			// check
			uint32 res = bloom_probe( buf, key );
			correct = (res == key_img );
		}
		printf("success!\n");
		ban_( key );
		return true; 
	}

	bool validate()
	{
		printf("validating...\n");
		// validate existing keys
		size_t errors = 0;
		for( size_t n = 0; n != XXC_BLOOM_POWER; n++ ) {
			uint32 res = bloom_probe( buf, n );
			if ( is_in( n ) ) {
				if ( res != key_img ) {
					errors++;	
					printf("%d decode error %08x = %08x\n", n, res, key_img);
				}
			} else {
				if ( res == key_img ) {
					errors++;
					printf("%d collision :(\n",n);
				}
			}
		}
		if ( !errors ) printf("passed!\n");
		return !errors;
	}

	void trash( uint32 seed = 123 )
	{
		printf("trashing...\n");
		//memcpy( usd, buf, XXC_BLOOM_SIZE );
		bloom_trash( buf, usd, XXC_BLOOM_SIZE, seed );
	}
	
	void stats()
	{
		printf("stats...\n");

		printf("%d keys tried, %d keys failed (%.3f%%)\n", keys_tried, keys_failed, 100.f*keys_failed/keys_tried );
		printf("%d keys collided\n", keys_collided );
		size_t total_bits = XXC_BLOOM_SIZE * 8;
		size_t bits_set   = no_set( buf, XXC_BLOOM_SIZE );
		size_t bits_used  = no_set( usd, XXC_BLOOM_SIZE );
		printf("%dkbi of %dkbi (%.2f%%) is set\n",  bits_set/1024,  total_bits/1024, 100.f*bits_set/total_bits );
    	printf("%dkbi of %dkbi (%.2f%%) is used\n", bits_used/1024, total_bits/1024, 100.f*bits_used/total_bits );

		printf("valid serials %d of total %d (%.3f%%)\n", keys_tried-keys_failed, XXC_BLOOM_POWER, 100.f*(keys_tried-keys_failed)/XXC_BLOOM_POWER );
		printf("or 1 to %.2f (%.5f%%) of 2^28 set\n", 268435456.f / (keys_tried-keys_failed),100.f*(keys_tried-keys_failed) / 4294967296.f );

		FILE* fo = fopen( "dumpset.bits", "wb" );
		fwrite( buf, XXC_BLOOM_SIZE, 1, fo );
		fclose( fo );

//		fo = fopen( "dumpused.bits", "wb" );
//		fwrite( usd, XXC_BLOOM_SIZE, 1, fo );
//		fclose( fo );
	}

	void dump( const char* fname )
	{
		FILE* f = fopen(fname,"wt");
		fprintf( f, "%07x\n\n", key_img );
		uvec lst;
		lst.resize( reg.size() );
		std::copy( reg.begin(), reg.end(), lst.begin() );
		std::random_shuffle( lst.begin(), lst.end() );
		for( uvec::const_iterator it = lst.begin(); it != lst.end(); ++it ) {
			fprintf( f, "%07x\n", *it );
//			fprintf( f, "%07x\n", XXC_ROTL32C(*it,15) ^ 0x3C6EF35F );
		}
		fclose(f);
	}
};

uint32 remix( uint32 n )
{
//	uint32 v = (n*0x0019660D + 0x3C6EF35F);
	uint32 v = (1812433253UL * (n ^ (n >> 30)) + 0x3C6EF35F);
//	return v & (BLOOM_POWER-1);

	// kurrently preimage is fixed at 27 bits (all what we have now)
	return v & 0x7ffffff;
//	return v & ((1<<25)-1);
}

//
//
void do_generate( const char* txtname, const char* bitsname )
{
}

// generate initial serials list
// == xserials s <SEED> <KEY> <out-textfile> <out-binfile>
// generate bitarray from serials list

uint32 htou( const char* str )
{
	uint32 res;
	sscanf( str, "%x", &res );
	return res;
}

int main( int argc, char* argv[] )
{
	if ( argc < 2 ) {
		printf("specify command (s,b)...\n");
		return 1;
	}

	if ( *(argv[1]) == 's' ) {
		if ( argc < 5 ) {
			printf("sgen s <0x-seed> <0x-key> <outtext> <outbin>\n");
			return 1;
		}

		// parse params
		uint32 kseed = htou( argv[2] );
		uint32 kkey  = htou( argv[3] );

		if ( (kkey&XXC_BLOOM_IMASK) != kkey ) {
			printf("key constant is out of range\n");
			return 3;
		}
		
		printf("initializing table with %08x (%d) bit key\n", kkey, XXC_BLOOM_PHASES );
		// do generate
		tester t( kkey );

		// probe keyspace
		printf("probing %d random keys\n", XXC_BLOOM_POWER );
//		for( size_t n = 0; n != XXC_BLOOM_POWER; n++ )
		for( size_t n = 0; n != 0x5fffff; n++ )
			t.add( remix(n) );

		// ban keys
		t.blacklist( 0x2a82a96 );		// E3EFA69A / keren karr key / annie taylor
		t.blacklist( 0x16a5a0b );		// nikolay harbov
		t.blacklist( 0x168509c );		// --
		//t.blacklist( 0x415b7bb );		// --
		t.blacklist( 0x61be6b9 );		// hwid:3AFF1B5B / iddq@safe-mail.net
		t.blacklist( 0x715f885 );		// FCA4BBDC / pairit@yahoo.com
		t.blacklist( 0x6290080 );		// ? / one more iddqd
		t.blacklist( 0x2200586 );		// E3BB0C9C / christoph bromberger 	iddq@safe-mail.net

		// replace unused bits with trash
		t.trash( kseed );
		// validate the function
		t.validate();
		// print stats
		t.stats();
		
		// dump keyfile
		t.dump( argv[4] );

	} else if ( *(argv[1]) == 'b' ) {
		if ( argc < 5 ) {
			printf("sgen s <0x-key> <0x-ban_id> <bin-file>\n");
			return 1;
		}
		
	} else {
		printf("unknown command.\n");
		return 2;
	}

	return 0;
}

