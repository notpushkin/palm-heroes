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
// MTwister random number generator
// Knuth et al
//

#ifndef XXC_RND_H__
#define XXC_RND_H__

namespace xxc {

// isaac rand
class rnd
{
public:
	enum { NParam = 624 };
	enum { MParam = 397 };
	enum { state_sz = NParam + 1 };

	void	reset( uint32 val );
	void	reset( const uint8* seed, size_t seedlen );

	uint32  get();

	void	load( const uint32 st[state_sz] );
	void	save( uint32 st[state_sz] );

private:
	void	forward();

    uint32	state[NParam];
    uint32* next;
    size_t	left;
    bool	inited;
};

} //xxc

#endif //XXC_RND_H__
