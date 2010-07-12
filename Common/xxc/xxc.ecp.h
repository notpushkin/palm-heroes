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
// Elliptic curve math on GF2
//
#ifndef XXC_ECP_H__
#define XXC_ECP_H__

#include "xxc.params.h"
#include "xxc.big.h"
#include "xxc.gf2.h"

namespace xxc {

typedef big	big_t;

//
struct point_t
{
	poly_t x;
	poly_t y;
};

////////////////////////////////////////////////////////

// confirm y2 + xy = x3 + B for p
bool validate( const point_t& p );

//
bool equ( const point_t& a, const point_t& b );

// 
void copy( point_t& d, const point_t& s );

// wipe out 
void clear( point_t& p );

// given x coord of the p finds out y
bool calc_y( point_t& p, bool ybit );

//
void add( point_t& p, const point_t& a );

//
void sub( point_t& p, const point_t& a );

// -p
void neg( point_t& p );

// 2*p
void dbl( point_t& p );

//
void mul( point_t& p, const big& a );

// 0 if 'x' == 0 or to (y/x) otherwise
bool ybit( const point_t& p );

void pack( big& i, const point_t& p );
void unpack( point_t&, const big& );


} // xxc

#endif //XXC_ECP_H__

