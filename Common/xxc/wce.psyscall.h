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
// Windows CE PSysCall inteface
// Specifies how to call system API directly

#ifndef WCE_PSYSCALL_H__
#define WCE_PSYSCALL_H__

#if !defined(ARM)
//#error "Works only on ARM!"
#endif

#define SYSCALL_STARTADDR 		0xF0010000
#define SYSCALL_STARTADDR_CE6	0xF1020000

// retrieves API Entry address
#define SYSCALL_ADDR(mid)        (SYSCALL_STARTADDR - (mid)*4)
// declares function 
#define SYSCALL_DECL(type, mid, args) (*(type (*)args)SYSCALL_ADDR(mid))

// Important API functions
#define SCAPI_VirtualAlloc		3
#define SCAPI_VirtualFree		4
#define SCAPI_VirtualProtect    5
#define SCAPI_KernelIoControl	99
#define SCAPI_SetKMode			108
#define SCAPI_NKTerminateThread	87

#define SCAPI6_VirtualAlloc		?
#define SCAPI6_VirtualFree		?
#define SCAPI6_VirtualProtect   ?
#define SCAPI6_KernelIoControl	39
#define SCAPI6_SetKMode			?
#define SCAPI6_NKTerminateThread	?

#endif //WCE_PSYSCALL_H__

