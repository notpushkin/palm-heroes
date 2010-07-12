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

// xxc fixed memory block
// aka pod C array
// 2007.02.20 SiGMan

#ifndef XXC_FBLOCK_H__
#define XXC_FBLOCK_H__

namespace xxc {

template< typename T, size_t SZ >
class fblock
{
public:
	typedef	T			 value_type;
	typedef fblock<T,SZ> this_type;

    typedef T&    		reference;
    typedef const T&	const_reference;
    typedef T*    		iterator;
    typedef const T*   	const_iterator;

	typedef T			param_type;
public:
	fblock()
	{}

//	fblock( const this_type& b )
//	{ memcpy( data_, b.data_, sizeof(data_) ); return *this; }

//	this_type& operator=( const this_type& b )
//	{ memcpy( data_, b.data_, sizeof(data_) ); return *this; }
	void assign( const T* s )
	{ memcpy( data_, s, sizeof(data_) ); }

	void clear()
	{ memset( data_, 0, sizeof(data_) ); }


	reference		operator[]( size_t pos )
	{ return data_[pos]; }
	const_reference operator[]( size_t pos )  const
	{ return data_[pos]; }
	
	iterator begin()
	{ return &data_[0]; }
	const_iterator begin() const
	{ return &data_[0]; }
	
	iterator end()
	{ return &data_[SZ]; }
	const_iterator end() const
	{ return &data_[SZ]; }

	bool operator==( const this_type& v ) const
	{ return memcmp( data_, v.data_, sizeof(data_) ) == 0; }

	size_t max_size() const
	{ return SZ-1; }

protected:
	T		data_[SZ];

};

} // xxc

#endif //XXC_FBLOCK_H__

