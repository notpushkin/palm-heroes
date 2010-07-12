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

// X File
//
// NOTE:
// for now we support only one mode at once -
// i.e. its either read- or write-only file


#include "stdafx.h"
#include "ScopePtr.h"
#include "xxc.file.h"
#include "xxc.cipher.h"
#include "xxc.bloom.h"
#include "xxc.shash.h"

namespace xxc {

const uint32 XFILE_ID = 'X' | ('X'<<8) | ('C'<<16) | ('1'<<24);

const size_t BUF_SIZE = 64 * 64; // 4K buf

//
// XFile structure
// 		0 : 'XXC1'	- id string
//		4 : SALT	- salt value
//	 8-12 : SIGN	- two word file signature derived from hash
//		x : contents

#ifdef _IOLIB_HELIUM_STD_H_
class xfile : public iFileI
#else 
class xfile : public ref_countable_impl<iFileI>
#endif
{
public:
	iFilePtr	file;
	uint32		salt;
	uint32		signature[2];

	uint32		fileStarts;
	
	uint8		buffer[ BUF_SIZE ];
	mutable uint32		bufOrg;
	mutable uint8*		bufPtr;
	mutable uint32		bufLeft;

	mutable xxc::cipher	cipher;

	bool		readMode;
	bool		initialized;


	xfile()
	: salt( 0 )
	, readMode( false )
	, initialized( false )
	{
	}

	~xfile()
	{
		if ( initialized ) Close();
	}

	uint32 gen_salt()
	{
		uint32 sa = 104729  * GetTickCount() + 8278991;
		uint32 sb = 99991 * (uint32)this;
		return bloom_mix( sa, sb );
	}

	void init_cipher( uint32 key )
	{
		uint32 cikey[4];
		cikey[0] = key;	
		cikey[1] = salt;
		cikey[2] = key + salt;
		cikey[3] = key ^ salt;
		cipher.reset( cikey );
	}

	bool Create( const iStringT& fname, uint32 key )
	{
		// 1: create physical file
		file = CreateWin32File( fname );
		if ( !file ) return false;
		// 2: write file id
		file->Write( &XFILE_ID, 4 );
		// 3: generate salt
		salt = gen_salt();
		// 4: write salt
		file->Write( &salt, 4 );
		// 5: write empty signature bytes
		signature[0] = 0x19782007;
		signature[1] = 0x012dd977;
		file->Write( signature, 8 );
		// 6: set file offset and file size params
		// 7: initialize cipher
		init_cipher( key );

		fileStarts = file->GetPosition();
		bufOrg	   = 0;
		bufPtr	   = buffer;
		bufLeft	   = BUF_SIZE;
		readMode   = false;
		initialized= true;

		return true;
	}

	bool Open( const iStringT& fname, uint32 key )
	{
		// 1: open physical file
		file = OpenWin32File( fname );
		if ( !file ) return false;
		// 2: read and check file_id
		uint32 readen = 0;
		uint32 fileId = 0;
		readen = file->Read( &fileId, 4 );
		if ( readen != 4 || fileId != XFILE_ID ) return false;
		// 3: read salt
		readen = file->Read( &salt, 4 );
		if ( readen != 4 ) return false;
		// 4: read signature
		readen = file->Read( signature, 8 );
		if ( readen != 8 ) return false;
		// 5: set file offset and file size params
		fileStarts = file->GetPosition();
		// 6: verify signature
		// 7: init cipher
		init_cipher( key );

		bufOrg	   = 0;
		bufPtr	   = buffer;
		bufLeft	   = 0;
		readMode   = true;
		initialized= true;

		return true;
	}

	bool IsOpen() const
	{
		return !!file;
	}

	void Close()
	{
		BufFlush();
	}

	uint32 internalGetPosition() const
	{
		// sync with buffer
		const_cast<xfile*>(this)->BufFlush();
		uint32 currentPos = 0;
		currentPos = file->GetPosition();
		if ( readMode ) {
			// we are reading ahead
			currentPos -= bufLeft;
		} else {
			// but writting 
			currentPos += bufPtr - buffer; 
		}
		return currentPos;
	}

	uint32 Seek( sint32 offset, SeekMode smode )
	{
		check( IsOpen() );

		bool gotPos = false;
		uint32 currentPos = 0;

		// for relative seeks we need get a position
		if ( smode == FSEEK_CUR ) {
			currentPos = internalGetPosition();
			// adjust position
			offset += currentPos;
			offset -= fileStarts;
			if ( offset < 0 ) offset = 0;
			smode	= FSEEK_SET;
			gotPos	= true;
		}

		// not optimized - dumb seek
		// offset still in virtual coordinates
		check( smode != FSEEK_CUR );
		BufFlush(); 
		if ( smode != FSEEK_END )
			offset += fileStarts;
		// note what FSEEK_END still can reach the head
		uint32 newPos = file->Seek( offset, smode );
	
		if ( newPos == ~0 ) return newPos;

		// reset buffer
		bufPtr		= buffer;
		bufLeft 	=  readMode ? 0 : BUF_SIZE;
		
		check( newPos >= fileStarts );
		bufOrg		= newPos - fileStarts;
		
		// clear EOF flag
		return newPos - fileStarts;
	}

	uint32 GetPosition() const
	{
		check( IsOpen() );
	
		uint32 currentPos = internalGetPosition();
		check ( currentPos >= fileStarts );
		// adjust from header
		return currentPos - fileStarts;
	}

	uint32 GetSize() const
	{
		check( IsOpen() );
		// sync
		const_cast<xfile*>(this)->BufFlush();
		uint32 fileSize = file->GetSize();
		check( fileSize >= fileStarts );
		// adjust for header
		return fileSize - fileStarts;
	}

	uint32 Read( void* ptr, uint32 len )
	{
		check( IsOpen() );

		if ( !readMode ) {
			check( 0 == "No read allowed on write-mode file" );
			return 0;
		}

		//return file->Read( ptr, len );

		uint32 residue = len;
		uint8* p = (uint8*)ptr;

		while ( residue != 0 ) {
			if ( !bufLeft  ) {
				if ( BufFill() || (bufLeft == 0) ) {
					return len - residue;
				}
			}
			check( bufLeft != 0 );

			uint32 toRead = min( residue, bufLeft );
			memcpy( p, bufPtr, toRead );
			p		+= toRead;
			bufPtr	+= toRead;
			residue -= toRead;
			bufLeft -= toRead;
		}

		return len;
	}

	uint32 Write( const void* ptr, uint32 len )
	{
		check( IsOpen() );

		if ( readMode ) {
			check( 0 == "No write allowed on read-mode file" );
			return 0;
		}
		//return file->Write( ptr, len );

		uint32 residue = len;
		const uint8* p = (const uint8*)ptr;
		while ( residue != 0 ) {	
			uint32 freeLeft = BUF_SIZE - (bufPtr - buffer);
			uint32 toWrite = min( freeLeft, residue );
			memcpy( bufPtr, p, toWrite );
			//bufLeft -= toWrite;
			bufPtr  += toWrite;
			p		+= toWrite;
			residue -= toWrite;
			if ( !freeLeft ) {
				if ( BufFlush() ) return len - residue;
			}
		}
		return len;
	}

	uint32 BufFill()
	{
		check( readMode );
		// reset read leftovers
		
		bufPtr = buffer;
		bufLeft = file->Read( bufPtr, BUF_SIZE );

		if ( bufLeft ) {
			cipher.resync( bufOrg );	
			cipher.process( buffer, bufLeft );
			bufOrg += bufLeft;
		}
		//if ( readBufLeft == 0 ) return IOEOF;

		return 0;
	}

	uint32 BufFlush() 
	{
		if ( readMode || bufPtr == buffer ) {
			return 0;
		}

		uint32 toWrite = bufPtr - buffer;
	
		cipher.resync( bufOrg );
		cipher.process( (uint8*)buffer, toWrite );

		bufOrg += toWrite;
		uint32 written = file->Write( buffer, toWrite );

		bufPtr = buffer;
		bufLeft= BUF_SIZE;

		if ( written == 0 || written != toWrite ) return ERROR;

		return 0;
	}

	void Flush()
	{
		check( IsOpen() );
		// flush internal buffer
		BufFlush();
		// sync host filesystem
		file->Flush();
	}
};


iFileI* CreateXFile( const iStringT& fname, uint32 key )
{
	//return CreateWin32File( fname );
	ScopedPtr<xfile> file( new xfile() );
	if ( !file->Create( fname, key ) ) 
		return 0;
	return file.Giveup();
}

iFileI* OpenXFile( const iStringT& fname, uint32 key )
{
	//return OpenWin32File( fname );
	ScopedPtr<xfile> file( new xfile() );
	if ( !file->Open( fname, key ) ) 
		return 0;
	return file.Giveup();
}

} // xxc


