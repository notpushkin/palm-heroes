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
// Highspeed lossless audio codec v.1
// Based on TTA (True Audio) Copyright 1999-2006 Aleksander Djuric
// 2007-01-09 : SiGMan - functionality separation on low level & middleware

#ifndef XAU_AUDIO_H__
#define XAU_AUDIO_H__

#include "xau.cfg.h"

#include "xbs.bs.h"
#include "xau.rice.h"
#include "xau.ppf.h"

struct xau_codec_t 
{
	put_bits_t	pb;
	get_bits_t	gb;
	rice_ctx_t	rice;
	ppfilter_t	flt;
};

// encode given number of sint8 mono samples to the buffer (which size at least 'samples' bytes)
// returns size of the bytes written into the buffer
size_t xau_pcm8m_encode( const sint8* pcm, size_t samples, void* buf, size_t bufsize );
// decode given number of samples
// returns number of samples decoded
size_t xau_pcm8m_decode( const void* buf, size_t bufsize, sint8* pcm, size_t samples );

#endif //XAU_AUDIO_H__

