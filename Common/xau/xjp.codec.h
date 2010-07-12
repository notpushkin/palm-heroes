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

// jpeg alike compression with opt for speed and 5.6.5 color
// 2007-12-10 : SiGMan - lets do it

#ifndef XJP_CODEC_H__
#define XJP_CODEC_H__

#include "xbs.cfg.h"
#include "xbs.bs.h"
#include "xjp.huff.h"
#include "xjp.dct.h"

namespace xjp {

static const uint32 PIX_RGB16	= 2;
static const uint32 PIX_RGB24	= 3;
static const uint32 PIX_RGB32	= 4;
static const uint32 PIX_RGBA32	= 5;
static const uint32 PIX_RGB16A	= 6;

struct encoder_t
{
	// coder internals
	huff_params_t	params[6];
	huff_etab_t		enc[6];
	put_bits_t		pbits;

	// picture desc
	uint32	width;
	uint32	height;
	uint32	channels;
	uint32	qfactor;
};

void enc_init( encoder_t* enc, uint32 width, uint32 height, uint32 chnl, uint32 qfact );
//void enc_prof( encoder_t* enc, const void* bits );
uint32 enc_do( encoder_t* enc, const void* in, void* out );
void enc_done( encoder_t* enc );


struct decoder_t
{
	// decoder internals
	huff_dtab_t		dec[6];
	get_bits_t		gbits;

	// picture desc
	uint32	width;
	uint32	height;
	uint32	channels;
	uint32	qfactor;

	void*	data_in;
	uint32  body_pos;
};

struct desc_t
{
	uint32	width;
	uint32	height;
	uint32	mode;
};

void dec_init( decoder_t* dec, desc_t* desc, const void* in );
void dec_do( decoder_t* dec, void* out, uint32 bpp );
void dec_done( decoder_t* dec );


} // xjp

#endif //XJP_CODEC_H__
