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
//

#ifndef XXL_ARRAY_CFG_H__
#define XXL_ARRAY_CFG_H__

#define XXL_USED 1

#define XXL_TRY 
#define XXL_UNWIND( act )
#define XXL_UNWIND_RET( act, ret ) return (ret);

#ifndef __PLACEMENT_NEW_INLINE
#define __PLACEMENT_NEW_INLINE
inline void *__cdecl operator new(size_t, void *_P) { return (_P); }
inline void __cdecl operator delete(void *, void *) { return; }
#endif

template< typename T >
void swap( T& a, T& b )
{
	T t = a;
	a = b;
	b = t;
}


template< typename T >
T maximum_of_two( T a, T b )
{
	if ( a < b ) return b;
	else return a;
}

#endif //XXL_ARRAY_CFG_H__

