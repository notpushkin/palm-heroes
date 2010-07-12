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

#include "stdafx.h"

#include "xxc.cfg.h"
#include "xxc.defs.h"
#include "xxc.bswap.h"
#include "xxc.sdh.h"

namespace xxc {

/*
unsigned long
udivmodsi4(unsigned long num, unsigned long den, int modwanted)
{
	unsigned long bit = 1;
	unsigned long res = 0;

	while (den < num && bit && !(den & (1L<<31))) {
		den <<=1;
		bit <<=1;
	}
	while (bit) {
		if (num >= den) {
			num -= den;
			res |= bit;
		}
		bit >>=1;
		den >>=1;
	}
	if (modwanted) return num;
	return res;
}
*/

uint32 div_urem(uint32 dividend, uint32 divisor )
{
	uint32 t, num_bits;
	uint32 q, bit, d;
	uint32 i;

	uint32 remainder = 0;
	uint32 quotient = 0;

	if (divisor == 0)
		return 0;

	if (divisor > dividend) {
		remainder = dividend;
		return remainder;
	}

	if (divisor == dividend) {
		quotient = 1;
		return 0;
	}

	num_bits = 32;

	while (remainder < divisor) {
		bit = (dividend & 0x80000000) >> 31;
		remainder = (remainder << 1) | bit;
		d = dividend;
		dividend = dividend << 1;
		num_bits--;
	}


	// The loop, above, always goes one iteration too far.
	// To avoid inserting an "if" statement inside the loop
	// the last iteration is simply reversed. 

	dividend = d;
	remainder = remainder >> 1;
	num_bits++;

	for (i = 0; i < num_bits; i++) {
		bit = (dividend & 0x80000000) >> 31;
		remainder = (remainder << 1) | bit;
		t = remainder - divisor;
		q = !((t & 0x80000000) >> 31);
		dividend = dividend << 1;
		quotient = (quotient << 1) | q;
		if (q) {
			remainder = t;
		}
	}
	return remainder;
} 

} //xxc

