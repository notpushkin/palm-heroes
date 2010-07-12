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

// XXC Crypto library
// Simple but efficient way to get some protection for your app
// Based on elliptic cryptography and some of the ciphers
// 2007 SiGMan

#ifndef XXC_CFG_H__

#ifdef OS_WINCE
#if defined(ARM) || defined(_ARM_) || defined(MIPS) || defined(SH3)
//#	define XXC_NO_UNALIGNED_ACCESS
#	define XXC_NOLOOKUP
#	define XXC_UNALIGNED_BYTE_READ
#endif
#endif

#define XXC_CFG_NO_SEH

// true for pocketPC and X86 - but better perform a check in startup
#define XXC_CPU_LE

#ifndef always_inline
#ifndef DEBUG1234
#	define always_inline __forceinline
#else
#	define always_inline __inline
#endif
#endif



#endif //XXC_CFG_H__

