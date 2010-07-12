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
// 2007.01.09 : SiGMan - first library version

#include "stdafx.h"
#include "xau.cfg.h"
#include "xau.audio.h"

// inline implementations
#include "xbs.bs.impl.h"
#include "xau.rice.impl.h"
#include "xau.ppf.impl.h"

#define ORDER 8
#define PREDICTOR1(x, k)	((sint32)((((uint64)x << k) - x) >> k))

// PCM8M Parameters
// K1=2, K2=2, F=9
#define PCM8M_K1 2
#define PCM8M_K2 2
#define PCM8M_SH 9

size_t xau_pcm8m_encode( const sint8* pcm, size_t samples, void* buf, size_t bufsize )
{
	// check preconditions

	xau_codec_t codec;
	init_put_bits( &codec.pb, (uint8*)buf, bufsize );
	rice_init( codec.rice, PCM8M_K1, PCM8M_K2 );
	ppfilter_init( &codec.flt, PCM8M_SH );

	const sint8* p = pcm;
	for( size_t n = 0; n != samples; ++n ) {
		// a) skip sample diffing part as it seems hurting performance a lot on 8bit mode
		// *p -= prev / 2
		// b) apply polyphase filter
		sint32 tmp = ppfilter_encode( &codec.flt, *p++ );
		// c) skip median predictor part - also hurts performance on 8bps
		// *p -= PREDICTOR1( last, 4 );
		// d) encode the 'unsignificated' code
		rice_enc( &codec.pb, codec.rice, xbs_sign_pack( tmp ) );
	}
	align_put_bits( &codec.pb );
	uint32 written = put_bits_count( &codec.pb ) / 8;

	// check for overruns
	return written;
}


size_t xau_pcm8m_decode( const void* buf, size_t bufsize, sint8* pcm, size_t samples )
{
	// check preconditions

	xau_codec_t codec;
	init_get_bits( &codec.gb, (uint8*)buf, bufsize*8 );
	rice_init( codec.rice, PCM8M_K1, PCM8M_K2 );
	ppfilter_init( &codec.flt, PCM8M_SH );

	sint8* p = pcm;
	for( size_t n = 0; n != samples; ++n ) {
		// d) decode rice code & unpack sign
		sint32 tmp = xbs_sign_unpack( rice_dec( &codec.gb, codec.rice ) );
		// c) skip median un-predictor
		// b) apply invert polyphase filter
		tmp = ppfilter_decode( &codec.flt, tmp );
		// a) skip sample de-diffing part
		*p++ = (sint8)tmp; // clipping is not checked, since its lossless codec
	}

	// check for overruns

	return samples;
}
