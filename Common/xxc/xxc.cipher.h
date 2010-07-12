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
// stream cipher based on salsa12
// NOTE: we use 32bit stream positions, 
// so files with >4Gb size cannot be handled properly!
#ifndef XXC_CIPHER_H__
#define XXC_CIPHER_H__

//#include "xxc.?"

namespace xxc {

//
class cipher
{
public:
	typedef uint32 word_t;

	cipher();
	~cipher();
	
	void clear();
	//void reset( const uint8* key, size_t keylen );
	void reset( const word_t key[4] );

	void process( uint8* stream, size_t length );
	void resync( size_t pos );

protected:
	void keysetup( const word_t key[4] );
	void ivsetup( const word_t iv0, const word_t iv1 );

	size_t	keyleft;
	word_t	kstream[64];
	word_t	state[16];
};


} //xxc

#endif //XXC_CIPHER_H__

