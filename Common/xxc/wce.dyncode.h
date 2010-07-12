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
// Windows CE ARM - Dynamic Codegen
// Allows to dynamically generate code stub
// to perform secret function


#ifndef WCE_DYNCODE_H__
#define WCE_DYNCODE_H__

//
// preparing arena for dynamic code depends on VirtualAlloc/etc api
// there is a 3 ways to call these API, ranged by obfuscation level
// 1) Dynamic Linkage - most comatible
// 2) Dynamic Loading Linkage - most compatible
// 3) Export Table HASH dynamic linkage - requre ExpTab compatibility
// 4) Direct call - requre Win32Methods table binary compatibility

#define WCE_CGENARENASIZE	4096

namespace xxc {

// allocates one page for codegen
void*	cgen_alloc();
// prepares arena for writting
void	cgen_lock( void* );
// prepares arena for execution
void	cgen_unlock( void* );
// frees up memory
void	cgen_free( void* );

} //xxc

#endif //WCE_DYNCODE_H__
