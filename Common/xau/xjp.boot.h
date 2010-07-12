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

#ifndef BOBOGOFJOSDFADADNDSDNKNDQW2323
#define BOBOGOFJOSDFADADNDSDNKNDQW2323

typedef unsigned int uint32;
typedef signed   int sint32;

typedef unsigned short uint16;
typedef signed   short sint16;

typedef unsigned char uint8;
typedef signed char sint8;

#include <assert.h>
#include <string.h>

#ifndef check
#define check assert
#endif

#if defined(__GNUC__)
#	define always_inline __attribute__((always_inline)) inline
#	define long64 long long
#elif ( _MSC_VER+0 > 1 )
#	define always_inline __forceinline
#	define long64 __int64
#	define for if(0) {} else for
#else
#	define always_inline __attribute__((always_inline)) inline
#	define long64 long long
#endif


#endif //BOBOGOFJOSDFADADNDSDNKNDQW2323
