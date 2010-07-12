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
// 6ae38d4d 487066bd 2f8abb18


#ifndef XXC_HWIDFN_H__
#define XXC_HWIDFN_H__

#include "xxc.cfg.h"
//
// Name hash values 
#define XXC_HV_COREDLL		0x6ae38d4d
#define XXC_HV_KERNELIOCTRL 0x487066bd
#define XXC_HV_GETUNIQUEID	0x2f8abb18
#define XXC_HV_GETFILEATTEX	0xf9ba6d32
#define XXC_HV_GETMODULEINF	0xcc43d203
#define XXC_HV_VIRTPROTECT	0x1000c04c

namespace xxc {

// initialize module
// should be called far from the usage point
// i.e. at the program start
bool  dll_init( HMODULE hCoreDLL );
void  dll_done();

// find module handle by name hash
// returns NULL if fails
void* dll_module( uint32 hash );
// finds function by module handle and name hash
// returns NULL if fails
void* dll_function( void* module, uint32 hash );
void* dll_funclist( void* module, uint32* hashes, void** func );

} // xxc

#endif //XXC_HWIDFN_H__

