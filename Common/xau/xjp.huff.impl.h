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
// huffman coder mechanics
// 2007-12-05 : SiGMan - initial revision

#ifndef XJP_HUFF_IMPL_H__
#define XJP_HUFF_IMPL_H__

#include "xbs.cfg.h"
#include "xbs.bs.impl.h"
#include "xjp.huff.h"

namespace xjp {

// encode single huffman symbol into bitstream
static always_inline
void put_huffman( put_bits_t* pb, const huff_etab_t& et, uint32 sym )
{
	check( (1 << (1+et.vlc[sym].bits)) > et.vlc[sym].code );
	put_bits( pb, et.vlc[sym].bits, et.vlc[sym].code );
}

// decode single huffman symbol from bitstream
// if symb length < max-lookup, uses fast path
static always_inline
uint32 get_huffman( get_bits_t* gb, const huff_dtab_t& dt )
{
	register uint32 code;
	register sint32 bits;	
	register uint32 index;

	OPEN_READER( reader, gb );
	UPDATE_CACHE( reader, gb );

	index = SHOW_UBITS( reader, gb, huff_maxlookahead ); 
	bits  = dt.vlc[index].bits;
	code  = dt.vlc[index].code;

	if ( bits < 0 ) {
		// rewind
		SKIP_BITS( reader, gb, 8 );
		// reload cache?
		// UPDATE_CACHE
		bits = -bits;
		index = SHOW_UBITS( reader, gb, bits ) + code;
		bits  = dt.vlc[index].bits;
		code  = dt.vlc[index].code;
		check( bits > 0 );
	}

	LAST_SKIP_BITS( reader, gb, bits );
	CLOSE_READER( reader, gb );

	return code;
}


} // xjp

#endif //XJP_HUFF_IMPL_H__
