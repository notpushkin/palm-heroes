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



typedef uint32 (*constfunc_t)( void* );



#define XXC_DECL_CONST( name ) \
	extern 	constfunc_t name##_fn; \
	/* */

#define XXC_IMPL_CONST( name ) \
	constfunc_t name##_fn = cinit; \
	/* */

#define XXC_GETCONST( name ) \
	((name##_fn)( (void*)&name##_fn )) \
	/* */

uint32 cinit( void* param )
{
	constfunc_t self = (constfunc_t)( *(constfunc_t*)param );
	if ( self == cinit ) {
		//initialization
	} else {
		assert(0);
	}
}

