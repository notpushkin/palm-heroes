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
// Application security interface
//

#ifndef XXC_APP_H__
#define XXC_APP_H__

namespace xxc {

#define XXC_HMAC_KEYLEN_MAX	16
#define XXC_HMAC_WORDS		8

#define XXC_MAX_SCONSTANTS	32

#define XXC_HASH_WORDS		8
#define XXC_KEY_WORDS		4

// Key manipulation constants
#define XXC_XOR_HWID 	0xD807AA98
#define XXC_XOR_SERIAL  0x0538A55F

// contains all global security info 
// structure must be allocated globally preferably in heap
struct secdata_t
{
	// :: map / game loader part
	// must be preserved and wiped out in time
	//////////////////////////////////////////////
	uint32  maphmac_keylen;	
	uint8	maphmac_key[XXC_HMAC_KEYLEN_MAX];	// wipe out asap

	uint32	maphmac[XXC_HMAC_WORDS];
	uint32	trash01[XXC_HMAC_WORDS];			// change periodicaly
	uint32	maporig[XXC_HMAC_WORDS];			// wipe after check

	// :: sec constants part
	// scattered using across the block using custom random function
	// i.e. each restart specific constant is moved to its own position
	//////////////////////////////////////////////
	uint32	constants[XXC_MAX_SCONSTANTS];		

	// :: key verification part
	/////////////////////////////////////////////
	uint32	hwid_hash[XXC_HASH_WORDS];
	uint32	hwid_key[XXC_KEY_WORDS];

	// :: bloom serial keys
	/////////////////////////////////////////////
	void*	serial_table;					// allocate on load and dispose (wipeout) asap
	uint32	serial_key;
};

uint32 ProcessActivationKey(LPCWSTR act_key);

// cipher / decipher block
// used as the simple cipher for file (blocks)
//
// seckey  - master key constant (program unique)
// filekey - file   key constant (file unique)
// blksize - size of the cipher block
void cipher_block( uint32 seckey, uint32 filekey, void* ptr, uint32 blksize );

// generate keys for ciphering from magic value (bloom constant)
void scramble_magic( uint32 seckey, uint32 magic, uint32 keys[4] );

} //xxc

#endif //XXC_APP_H__

