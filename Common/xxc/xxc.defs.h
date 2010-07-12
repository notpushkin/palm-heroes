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

// portability macroses
//

#ifndef XXC_DEFS_H__
#define XXC_DEFS_H__

#include "xxc.cfg.h"

always_inline uint32 rotate_left32( uint32 v, size_t n )
{ 
	return n ? uint32(v << n) | (v >> (32 - (n))) : v;
}

always_inline uint32 rotate_right32( uint32 v, size_t n )
{ 
	return n ? uint32(v << (32 - n)) | (v >> n) : v;
}

always_inline void wipeout( void* p, size_t bytes )
{
	memset( p, 0, bytes );
}

always_inline void copybytes( void* to, const void* from, size_t bytes )
{
	memcpy( to, from, bytes );
}


#define XXC_ROTL32C( v, n )	rotate_left32( v, n )
#define	XXC_ROTR32C( v, n ) rotate_right32( v, n )

#define XXC_ROTL32( v, n ) XXC_ROTL32C( v, n )
#define XXC_ROTR32( v, n ) XXC_ROTR32C( v, n )

#define XXC_LOAD32( ptr )
#define XXC_STORE32( ptr, val )

#endif //XXC_DEFS_H__
