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
// Salsa20 based hash
// ! uses reduced to 10 numbers of rounds
//

#ifndef XXC_SHASH_H__
#define XXC_SHASH_H__

#include "xxc.big.h"

namespace xxc {

// 
class hash
{
public:
	// internally operates in 
	typedef uint32	word_t;

	enum { words_sz = 8 };
	enum { bytes_sz = words_sz*sizeof(word_t) };
	enum { block_sz = 64 };
	typedef fblock<word_t, words_sz> image_t;

	hash();
	void reset();
	void reset( const word_t iv[words_sz] );
	void add( const void* data, size_t length );
	void finalize( word_t val[words_sz] );
	
//	big as_big() const;
private:
	uint32	cnt[2];
	word_t	img[words_sz];
	uint8	buf[block_sz];
};

// HMACk(m) = h( (K^opad) || h( (K^ipad) || m ) )
// where opad = 0x5c5c5c5c..
// and   ipad = 0x36363636..
class hmac
{
public:
	typedef uint32	word_t;

	enum { words_sz = 8 };
	enum { bytes_sz = words_sz*sizeof(word_t) };
	enum { block_sz = 64 };
	typedef fblock<word_t, words_sz> image_t;

	hmac();
	void reset( const uint8* key, size_t keylen );
	void add( const void* data, size_t length );
	void finalize( word_t val[words_sz] );

private:
	uint32	cnt[2];
	word_t	img[words_sz];
	word_t	key[words_sz];
	uint8	buf[block_sz];
};

//
uint32 two2one( const uint32 in[2] );
uint32 four2one( const uint32 in[4] );


} // xxc

#endif //XXC_SHASH_H__

