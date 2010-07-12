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
// bitstream interface (only objects to lower coupling pressure)

#ifndef XBS_BS_H__
#define XBS_BS_H__

#include "xbs.cfg.h"

#ifdef XBS_CPU_LE
//#	define XBS_READER_LE
#endif

struct put_bits_t
{
    uint8 *buf;
    uint8 *end;
    uint32 index;
};

struct get_bits_t
{
    const uint8 *buf;
    const uint8 *end;
	uint32 index;
    uint32 bits;
};


#endif //XBS_BS_H__

