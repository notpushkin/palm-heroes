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

#ifndef _GXLIB_PAIR_H
#define _GXLIB_PAIR_H

template< typename T, typename U >
class iPair
{
public:

	/// Initializes first to T() and second to U(). 
	iPair( const T &f, const U& s )
	: first(f)
	, second(s)
	{}

	/// Default constructor
	iPair()
	{}

	/// Copy constructor
	iPair( const iPair& p )
	: first( p.first )
	, second( p.second )
	{}

	/// Assignment operator
	iPair& operator = ( const iPair& other )
	{
		first	= other.first;
		second	= other.second;
		return *this;
	}

	/// Equality operator
	bool operator == ( const iPair& other ) const
	{
		return ( first == other.first ) && ( second == other.second );
	}

	/// Inequality operator
	bool operator != ( const iPair& other ) const
	{
		return !operator==(other);
	}

	T first;
	U second;
};

template< typename T, typename U >
iPair<T,U> MakePair( const T& first, const U& second ) 
{
	return iPair<T,U>( first, second );
}

#endif //_GXLIB_PAIR_H

