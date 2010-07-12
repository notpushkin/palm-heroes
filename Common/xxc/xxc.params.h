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

#ifndef XXC_PARAMS_H__
#define XXC_PARAMS_H__

namespace xxc {
namespace cfg {

typedef uint16 unit_t;
typedef uint32 wide_t;	

/*
	GF_M	dimension of the large finite field (GF_M = GF_L*GF_K)
	GF_L	dimension of the small finite field

	x^K + x^T + 1, T < K/2
	primitive if irreducible (p=q*r, either q or r has to be 1)
	and order of x is 2^K-1
	GF_K	degree of the large field reduction trinomial - m
	GF_T	intermediate power of the reduction trinomial - k

	GF_RP	reduction polynomial for the small field (truncated) {aka root}

	GF_NZT	element of the large field with nonzero trace
	GF_TM0	size of trace mask
	GF_TM1	1st nonzero element of trace mask
	GF_TM2	2nd nonzero element of trace mask
	EC_B	scalar term of elliptic curve equation (y^2 + xy = x^3 + EC_B)

	M,	L,  K, a.T, a.root, a.b, 	a.nso, a.ntf (opt)
	60  12, 5, 2, 	83, 	2079, 	14, 	1  = {60,48}
	63	9	7  1	17		43		48		1  = {63,54}

*/

#ifdef ECC60
static const wide_t	M	= 60;
static const wide_t L	= 12;
static const wide_t K	= 5;
static const wide_t T	= 2;
static const wide_t RP	= 0x0053;

static const wide_t NZT	= 0x0800;
static const wide_t TM0	= 4;
static const wide_t TM1	= 0x0800;
static const wide_t TM2 = 0x0800;

static const wide_t ECB	= 0x081F;

static const wide_t NSO = 14;
static const wide_t NTF = 1;
#else
static const wide_t	M	= 63;
static const wide_t L	= 9;
static const wide_t K	= 7;
static const wide_t T	= 1;
static const wide_t RP	= 0x0011;

static const wide_t NZT	= 0x0000;
static const wide_t TM0	= 1;
static const wide_t TM1	= 0x0021;
static const wide_t TM2 = 0x0000;

static const wide_t ECB	= 0x002b;

static const wide_t NSO = 48;
static const wide_t NTF = 1;
#endif


//extern static const ::xxc::gf2::point_t prime_order;
//extern static const ::xxc::ecc::point_t curve_point;

} //cfg
} //xxc

#endif //XXC_PARAMS_H__

