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
// Polyphase filter 

#ifndef XAU_PPF_H__
#define XAU_PPF_H__

#include "xau.cfg.h"

#define XAU_FILTER_MAX_ORDER 16

struct ppfilter_t {
	sint32	shift;
	sint32	round;
	sint32	error;
	sint32	mode;

	sint32	qm[XAU_FILTER_MAX_ORDER];
	sint32	dx[XAU_FILTER_MAX_ORDER];
	sint32	dl[XAU_FILTER_MAX_ORDER];
};

#endif //XAU_PPF_H__

