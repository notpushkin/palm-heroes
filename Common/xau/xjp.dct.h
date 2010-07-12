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
// basic dct / idct transfroms
// credits goes to Pascal Massimino
// based on "Fast and precise" LLM implementation of FDCT/IDCT paper
// 2007-12-07 : SiGMan - initial revision

#ifndef XJP_DCT_H__
#define XJP_DCT_H__

#include "xbs.cfg.h"

namespace xjp {

typedef sint16 dpel_t;

// inplace fdct transformation
void fdct16( dpel_t* val );
// inplace idct transformation
void idct16( dpel_t* val );


} //xjp

#endif //XJP_DCT_H__

