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
// Simplified (non-secure) Diffie-Hellman key exchange
//

#ifndef XXC_SDH_H__
#define XXC_SDH_H__

#include "xxc.defs.h"
#include "xxc.bonus.h"

namespace xxc {

// remainder of division fn 
uint32 div_urem(uint32 dividend, uint32 divisor );

// exp by squaring (used inline)
always_inline
uint32 exp_mod( uint32 n, uint32 p, uint32 m )
{
	uint32 res = 1;
	while ( p != 0 ) {
		if ( p & 1 ) {
			//res = (res * n) % m;
			res = div_urem( res * n, m );
			p--;
		}
		//n = (n*n) % m;
		n = div_urem( n * n, m );
		p /= 2;
	}
	return res;
}

// module and generator constants
const uint32 sdh_mod	= 64019;
const uint32 sdh_gen	= 2007;

/*
 Algorithm to use:
 - generate random 'a'
 - calc A and pass to function
 - - generate random 'b'
 - - calc Kb (A,b)
 - - calc B and pass back
 - calc Ka (B,a) and store
 - Ka == Kb
 - Ka, Kab <> [0..sdh_mod)
 */

/*
// G( q, g )
// Diffie-Hellman key exchange
void ensure( uint a, uint b )
{
	// a & b are secret
	// group
	// 33614 / 1437
	uint p = 64019;
	uint g = 2007; //638;
	uint A = pmod_sq( g, a, p );
	// >> (g, p), A
	uint B = pmod_sq( g, b, p );
	// << B
	uint Kb= pmod_sq( A, b, p );
	uint Ka= pmod_sq( B, a, p );
	printf("Ka:%d Kb:%d\n", Ka, Kb );
}
*/

} //xxc

#endif //XXC_SDH_H__

