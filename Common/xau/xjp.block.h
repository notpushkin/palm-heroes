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
// picture jpeg - alike macroblock coding 
//

#ifndef XJP_BLOCK_H__
#define XJP_BLOCK_H__

#include "xbs.cfg.h"
#include "xjp.huff.h"
#include "xjp.dct.h"

namespace xjp {

extern const uint8 ZAG[64];

void encode_block( const dpel_t* pels, dpel_t& last_dc, const huff_etab_t& dcoder, const huff_etab_t& acoder, put_bits_t* pb );
void profile_block( const dpel_t* pels, dpel_t& last_dc, uint32 freq_dc[], uint32 freq_ac[] );
void decode_block( dpel_t* pels, dpel_t& last_dc, const huff_dtab_t& dcoder, const huff_dtab_t& acoder, const dpel_t* qm, get_bits_t* gb );

//void encode_block( const dpel_t* pels, dpel_t pred, dpel_t& last_dc, const huff_etab_t& dcoder, const huff_etab_t& acoder, put_bits_t* pb );
//void profile_block( const dpel_t* pels, dpel_t pred, dpel_t& last_dc, uint32 freq_dc[], uint32 freq_ac[] );
//void decode_block( dpel_t* pels, dpel_t pred, dpel_t& last_dc, const huff_dtab_t& dcoder, const huff_dtab_t& acoder, get_bits_t* gb );

} // xjp

#endif //XJP_BLOCK_H__

