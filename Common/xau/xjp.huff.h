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
// jpeg-alike huffman coder interface (O(2) lookup decoder)
// 2007-12-05 : SiGMan - initial revision

#ifndef XJP_HUFF_H__
#define XJP_HUFF_H__

#include "xbs.cfg.h"

namespace xjp {

const size_t huff_maxlookahead	= 8;
const size_t huff_maxcodelen	= 16;
const size_t huff_maxinitclen	= 32;

const uint32 huff_maxsym = 256;
const uint32 huff_hisize = huff_maxsym + 1;

struct huff_params_t
{
 	uint8	bits[17];
 	uint8	vals[257];
}; // 273

struct vlc_t
{
	sint16	bits;
	uint16	code;
};

struct huff_etab_t
{
	vlc_t	vlc[ 256 ];
}; // 1024

struct huff_dtab_t
{
	uint32	allocated;
	vlc_t	vlc[ 256 * 3 ];
}; // 4 + 4096

//
//

// initialize encoder table
void init_huff_enc( bool isdc, const uint8* bits, const uint8* vals, huff_etab_t& etab );
// initialize decoder table
void init_huff_dec( bool isdc, const uint8* bits, const uint8* vals, huff_dtab_t& dtab );
// build huffman table
bool build_huff( uint32 freq[], huff_params_t& res );


} //xjp

#endif //XJP_HUFF_H__
