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

#ifndef XXC_BITSREAM_H_
#define XXC_BITSREAM_H_

class iBitStream 
{
public:
	iBitStream(size_t bit_count)
	{
		// init bit stream buffer
		m_bRead = false;
		m_buffSiz = (bit_count+7) / 8;
		m_pBuff = (unsigned char*)calloc( m_buffSiz, 1 );
		m_bsPos = 0;
	}

	iBitStream(const unsigned char* buff, size_t buff_siz)
	{
		// init bit stream buffer
		m_bRead = true;
		m_buffSiz = buff_siz;
		m_pBuff = (unsigned char*)malloc( m_buffSiz );
		memcpy(m_pBuff, buff, m_buffSiz);
		m_bsPos = 0;
	}


	~iBitStream() { free(m_pBuff); }

	inline void Write(unsigned int val)
	{
		if (val) {
			unsigned char* ptr = m_pBuff + (m_bsPos>>3);
			size_t bitPos = m_bsPos & 0x7;
			*ptr |= (1<<bitPos);
		}
		m_bsPos++;
	}

	inline void Write(unsigned int val, size_t bits)
	{
		while (bits--) {
			Write(val&0x1);
			val >>= 1;
		}
	}

	inline unsigned int Read()
	{
		unsigned char* ptr = m_pBuff + (m_bsPos>>3);
		size_t bitPos = m_bsPos & 0x7;
		m_bsPos++;
		return ((*ptr) >> bitPos) & 0x1;
	}

	inline unsigned int Read(size_t bits)
	{
		unsigned int res = 0;
		for (size_t bb=0; bb<bits; ++bb) {
			res |= (Read() << bb);
		}
		return res;
	}

	inline void Reset()
	{
		m_bsPos = 0;
	}

	inline const unsigned char* GetRawData() const { return m_pBuff; }
	inline size_t GetRawDataSiz() const { return m_buffSiz; }

private:
	bool			m_bRead;
	size_t			m_buffSiz;
	unsigned char* 	m_pBuff;
	size_t			m_bsPos;
};


#endif //XXC_BITSREAM_H_

