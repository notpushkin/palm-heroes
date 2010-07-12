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
// Secure dynamic storage class
// implements obuscated encrypted inplace associative storage
// based on binary tree structure (jsw-tree)
// Used to pass secure data between functions

#ifndef XXC_STORE_H__
#define XXC_STORE_H__

#include "xxc.defs.h"

namespace xxc {

typedef size_t 			ptr_t;
typedef unsigned short	wofs_t;

// node structure
struct snode_t
{
	wofs_t				link[2];
	sint32				level;
	uint32				key;
	void*				data;
}; 

const size_t max_snodes = 256 - 1;

// store header
struct sstore_t
{
	uint32		memalloc;	// allocated count (made first to mimic node struct)
	snode_t*	root;		// root node
	snode_t*	nil;		// nil  node (fixed position)
	snode_t*	memfree;	// freelist ptr
};  

// Safety check - both structures should be 16bytes in size
typedef char something_totaly_wrong__[ (sizeof(sstore_t) == 16)*2 - 1 ];
typedef char something_totaly_wrong2_[ (sizeof(sstore_t) == sizeof(snode_t))*2 - 1 ];

void*	store_init();
void	store_free( void* );

snode_t*	store_insert( void* block, uint32 key );
void		store_erase( void* block, uint32 key );
void*		store_shuffle( void* block );
//snode_t* 	store_find( void* block, uint32 key );


} //xxc

#include "xxc.store.inl"

#endif //XXC_STORE_H__

