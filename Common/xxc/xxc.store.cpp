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
// Secure store implementation
// based on JSW tree (variation of RBtree without explicit marks)
// modifications made:
// 1) fixed block randomized allocator
// - allocates memory pseudo-randomly within given block
// 2) packed node pointers
// - instead pointers we use indices from the begining of the block
// 3) polymorfic keys
// - tree keys are randomized 
// 4) direct access
// - access macroses are provided to help generate inline code
// 5) table permutations
// - mutate table into new block
// x) self validation
// - not implemented yet

#include "stdafx.h"

#include "xxc.cfg.h"
#include "xxc.defs.h"
#include "xxc.bswap.h"

#define XXC_STORE_INTERNAL
#include "xxc.store.h"

#define MAX_DEPTH 16

#undef assert
#define assert(x)

namespace xxc {


always_inline
uint32 randgen( uint32 in )
{
	return  ((0xc87a7197+in) >> 3) ^ (in << 5);
}


//
// Memory slab / freelist allocator
// Works within storage block (storage capacity is capped by 
// constants and fixed in this implementation)
// [! slab size is strictly fixed on 16 bytes !]
//    +-------------
// #0 | header  
//    +-------------
// #1 | nil node
//    +-------------
// #n | ... etc 
//    +---
//////////////////////////////////////////////////

always_inline
snode_t*
mem_alloc( void* block )
{
	sstore_t* t = mem_store( block );
	snode_t* n = t->memfree;
	if ( 0 != n ) {
		// unlink freelist
		snode_t* nf = *reinterpret_cast<snode_t**>( n );
		t->memfree = nf;
	} else {
		n = mem_block( block, t->memalloc++ );
	}
//	printf("alloc: %08x : %08x\n", n, t->memalloc-1 );
	return n;
}

always_inline
void
mem_free( void* block, snode_t* n )
{
	sstore_t* t = mem_store( block );
	snode_t** nf = reinterpret_cast<snode_t**>( n );
	*nf = t->memfree;
	t->memfree = n;
//	printf("free: %08x : %08x\n", n, t->memfree );
}

always_inline
void
mem_init( void* block )
{
	sstore_t* t = mem_store( block );
		
	// block #0 is reserved for header
	t->memfree	 = 0;
	t->memalloc	 = 1;
}


// Remove left horizontal links 
#define skew(r,t) \
do {                                      						\
  if ( nptr(r,t->link[0])->level == t->level && t->level != 0 ) { \
    snode_t *save = nptr(r,t->link[0]);                       	\
    t->link[0] = save->link[1];                          		\
    save->link[1] = nofs(r,t);                                  \
    t = save;                                             		\
  }                                                       		\
} while(0)

// Remove consecutive horizontal links 
#define split(r,t) \
do {                                              				\
  if ( nptr(r, nptr(r,t->link[1])->link[1])->level == t->level && t->level != 0 ) { \
    snode_t *save = nptr(r,t->link[1]);                           \
    t->link[1] = save->link[0];                                    \
    save->link[0] = nofs(r,t);                                     \
    t = save;                                                      \
    ++t->level;                                                    \
  }                                                                \
} while(0)

// Allocates new node
static always_inline
snode_t*
new_node( void* block, uint32 key )
{
	snode_t* rn = mem_alloc( block );

	rn->level	= 1;
	rn->key		= key;
//	rn->data	= data;
//	rn->link[0] = rn->link[1] = nofs(block,mem_block(block,1));
	rn->link[0] = rn->link[1] = nofs(block,mem_store(block)->nil);

	return rn;
}

//
////////////////////////////////////////////////////////////

#define ROTL(val,nb) ((val<<nb)|(val>>(32-nb)))

always_inline
uint32 wipe_rec( snode_t* nptr, uint32 rval )
{
	uint32* bptr = (uint32*)nptr;
	uint32 tmp = (rval = randgen( rval )) & 0xff;
	*bptr++ = (tmp << 16) + (tmp ^ 0xa5);
	rval = ROTL( rval, 3 );
	*bptr++ = rval & 7;
	rval = ROTL( rval, 7 );
	*bptr++ = rval;
	rval = ROTL( rval, 13 );
	*bptr++ = rval;
	return rval;
}

always_inline
void wipe_blk( void* block, uint32 kk = 0x7ac082e5 )
{
	uint32 rval = randgen( kk ^ (uint32)block );
	snode_t* bptr = (snode_t*)block;
	for( size_t n = 0; n != 256; n++ ) {
		rval = wipe_rec( bptr, rval );
		bptr++;
	}
}

// initialize new storage
void*
store_init()
{
	void* block = malloc( 256 * 16 );
	// randomize it first (make to look like the original one)
	wipe_blk( block );

	sstore_t* tree = mem_store( block );

	// init tree mem management
	mem_init( block );
	
	// allocate nil node
	snode_t* nil = mem_alloc( block );
	nil->level	= 0;
	nil->key    = 0; // ?
	nil->data	= 0;
	assert( tree->memalloc == 2 );
	// links to itself
//	nil->link[0]= nil->link[1] = nofs( block, mem_block( block, 1 ) );
	nil->link[0]= nil->link[1] = nofs( block, nil );
	tree->nil = nil;
	// init tree root
	tree->root = tree->nil;

	return block;
}

void
store_free( void* block )
{
/*
	sstore_t* tree = mem_store( block );
	snode_t* it = tree->root;
	snode_t* save;

	// destruct by rotation
	//jsw_node* nil = tree->nil;
	while ( it != tree->nil ) {
		if ( it->link[0] == nofs(block,tree->nil) ) {
			save = nptr(block,it->link[1]);
			// release data here ...
			mem_free( tree, it );
		} else {
			save = nptr(block,it->link[0]);
			it->link[0] = save->link[1];
			save->link[1] = nofs( block, it );
		}
		it = save;
	}
*/
	// wipe
	wipe_blk( block, 0x9ade13c2 );
	// finalize
	free( block );
}

///
/// Insert new node
snode_t*
store_insert( void* block, uint32 key )
{
	sstore_t* tree = mem_store( block );
	snode_t*  it = tree->root;
	if ( tree->root == tree->nil ) {
		it = tree->root = new_node( block, key );
	} else {
		snode_t* path[ MAX_DEPTH ];
		int top = 0;
		int dir;

		for( ;; ) {
			path[ top++ ] = it;
			dir = node_cmp( it, key ) < 0;

			snode_t* go = nptr( block, it->link[dir] );
			if ( go == tree->nil ) break;
			it = go;
		}
		// create new
		snode_t* ntmp = new_node( block, key );
		it->link[dir] = nofs( block, ntmp );
		it = ntmp;
		// walk back and fix tree
		while (--top >= 0) {
			if ( top != 0 ) {
				dir = nptr( block, path[top-1]->link[1] ) == path[top];
			}
			skew( block, path[top] );	
			split( block, path[top] );
			
			if ( top != 0 ) {
				path[top-1]->link[dir] = nofs( block, path[top] );
			} else {
				tree->root = path[top];
			}
			// cleanup path
			path[top] = 0;
		}
		
	}
	return it;
}

///
/// 
void
store_erase( void* block, uint32 key )
{
	sstore_t* tree = mem_store( block );
	if ( tree->root == tree->nil ) return;

	snode_t* it = tree->root;
	snode_t* path[ MAX_DEPTH ];
	int top = 0, dir, cmp;

	// find and save path
	for( ;; ) {
		path[top++] = it;
		if ( it == tree->nil ) return;
		cmp = node_cmp( it, key );
		if ( cmp == 0 ) break;
		dir = cmp < 0;
		it = nptr( block, it->link[dir] );
	}

	// remove found node
	wofs_t nilofs = nofs( block, tree->nil );
	if ( it->link[0] == nilofs || it->link[1] == nilofs ) {
		int dir2 = it->link[0] == nilofs;

		if ( --top != 0 ) {
			path[top-1]->link[dir] = it->link[dir2];
		} else {
			tree->root = nptr( block, it->link[1] );
		}
		// remove node
		wipe_rec( it, (uint32)it );
		mem_free( block, it );
	} else {
		snode_t* heir = nptr( block, it->link[1] );	
		snode_t* prev = it;

		while ( heir->link[0] != nilofs ) {
			path[top++] = prev = heir;
			heir = nptr( block, heir->link[0] );
		}

		// remove in order
		// release(  it->data )
		it->data = heir->data;
		it->key	 = heir->key;
		prev->link[prev == it] = heir->link[1];
		wipe_rec( heir, (uint32)heir );
		mem_free( block, heir );
	}

	// walk back and rebalance
	while (--top >= 0 ) {
		snode_t* up = path[top];
		if ( top != 0 ) {
			dir = nptr( block, path[ top - 1 ]->link[1] ) == up;
		}
		if ( nptr( block, up->link[0] )->level < up->level-1 || nptr( block, up->link[1] )->level < up->level-1 ) {
			if ( nptr( block, up->link[1])->level > --up->level ) {
				nptr( block, up->link[1])->level = up->level;
			}
			skew( block, up );
			snode_t* uplink = nptr( block, up->link[1] );
			skew( block, uplink );
			up->link[1] = nofs( block, uplink );

			uplink = nptr( block, nptr( block, up->link[1] )->link[1] );
			skew( block, uplink );//up->link[1]->link[1] );
			nptr( block, up->link[1] )->link[1] = nofs( block, uplink );

			split( block, up );
			uplink = nptr( block, up->link[1] );
			split( block, uplink );
			up->link[1] = nofs( block, uplink );
		}

		if ( top != 0 ) {
			path[top-1]->link[dir] = nofs( block, up );
		} else {
			tree->root = up;
		}
		// clear the path
		path[ top ] = 0;
	}
}

//
// tree traversal support
////////////////////////////////////

struct straverse_t
{
	void*		block;
	snode_t*	it;
	snode_t*	path[ MAX_DEPTH ];
	uint32		top;
};

static
snode_t*
walk_start( straverse_t& cw, void* block, int dir )
{
	sstore_t*	tree = mem_store(block);
	cw.block	= block;
	cw.it		= tree->root;
	cw.top		= 0;

	wofs_t nilofs = nofs( block, tree->nil );
	if ( cw.it	!= tree->nil ) {
		while( cw.it->link[dir] != nilofs ) {
			cw.path[ cw.top++ ] = cw.it;
			cw.it = nptr( block, cw.it->link[dir] );
		}
	}

	return cw.it;
}

static
snode_t*
walk_move( straverse_t& cw, int dir )
{
	void* 	 block = cw.block;
	sstore_t* tree = mem_store( block );
	snode_t* nil =  tree->nil;
	wofs_t 	 nilofs = nofs( block, nil );

	if ( cw.it->link[dir] != nilofs ) {
		cw.path[ cw.top++ ] = cw.it;
		cw.it = nptr( block, cw.it->link[dir] );

		while ( cw.it->link[!dir] != nilofs ) {
			cw.path[ cw.top++ ] = cw.it;
			cw.it = nptr( block, cw.it->link[!dir] );
		}
	} else {
		snode_t* last;
		do {
			if ( cw.top == 0 ) {
				cw.it = nil;
				break;
			}
			last = cw.it;
			cw.it = cw.path[ --cw.top ];
			// wipe out 'path'
		} while ( last == nptr( block, cw.it->link[dir] ) );
	}	
	return cw.it;
}

void*
store_shuffle( void* block )
{
	void* brand = store_init();
	
	straverse_t iter;
	snode_t* nil = mem_store(block)->nil;
	for( snode_t* node = walk_start( iter, block, 0 ); node != nil; node = walk_move( iter, 1 ) ) {
//		printf(">> %d : %d (%d)\n", node->key, (int)(node->data), node->level );
		snode_t* nn = store_insert( brand, node->key );
		nn->data = node->data;
	}
	
	store_free( block );
	return brand;
}  

always_inline snode_t* 
store_nil( void* block )
{
	return mem_store(block)->nil;
}


} // xxc

#ifdef TEST_STORE

void dump( void* t, const char* name )
{
	FILE* f = fopen( name, "wb" );
	fwrite( t, 4096, 1, f );
	fclose(f);
}



#ifdef UNDER_CE
int WINAPI WinMain(HINSTANCE hInstance,
                   HINSTANCE hPrevInstance,
                   LPTSTR    lpCmdLine,
                   int       nCmdShow)

{
#else
int main() {
#endif
	void* tree = xxc::store_init();
	dump( tree, "tree.000" );

	printf("storing...\n");
	for( int n = 1; n != 100; n++ ) {
		xxc::snode_t* node = xxc::store_insert( tree, n );
		assert( node->key == n );
		node->data = (void*)(n+100);
		if ( n > 50 ) xxc::store_erase( tree, n - 50 );
	}

	printf("traversing...\n");
	xxc::straverse_t iter;
	for( xxc::snode_t* node = xxc::walk_start( iter, tree, 0 ); node != xxc::store_nil(tree); node = xxc::walk_move( iter, 1 ) ) {
		printf("%d : %d (%d)\n", node->key, (int)(node->data), node->level );
	}
	printf("--\n");
	dump( tree, "tree.001" );
	tree = xxc::store_shuffle( tree );
	dump( tree, "tree.002" );
	for( int k = 50; k != 100; k++ ) {
		xxc::snode_t* node = xxc::store_find( tree, k );
		printf("%d : %d (%d)\n", node->key, (int)(node->data), node->level );
	}
	// insert 
	int z;
	for( z = 1; z != 50; z++ ) {
		unsigned zz = 0x8ac401e3 ^ (1979 * z + 0x7367198);
		xxc::snode_t* node = xxc::store_insert( tree, zz );
		node->data = (void*)z;
	}
	// validate
	for( z = 1; z != 50; z++ ) {
		unsigned zz = 0x8ac401e3 ^ (1979 * z + 0x7367198);
		xxc::snode_t* node = xxc::store_find( tree, zz );
		assert( node && node->data == (void*)z );
	}
	// shuffle
	tree = xxc::store_shuffle( tree );
	dump( tree, "tree.003" );
	// validate
	for( z = 1; z != 50; z++ ) {
		unsigned zz = 0x8ac401e3 ^ (1979 * z + 0x7367198);
		xxc::snode_t* node = xxc::store_find( tree, zz );
		assert( node && node->data == (void*)z );
	}
	// erase
	for( z = 1; z != 50; z++ ) {
		unsigned zz = 0x8ac401e3 ^ (1979 * z + 0x7367198);
		xxc::store_erase( tree, zz );
	}

	xxc::store_free( tree );
	//
#ifdef UNDER_CE
	char ch[12];
	scanf("%c", ch );
#endif
	return 0;
}

#endif // TEST_STORE


