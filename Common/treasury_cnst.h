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

#ifndef _HMM_COMMON_TREASURY_CONSTRUCTIONS_H_
#define _HMM_COMMON_TREASURY_CONSTRUCTIONS_H_

// Treasury variants container
class iTrVarCtr
{
public:
	struct Item {
		Item(uint32 _probability) : probability(_probability) {}
		uint32		probability;
		iArmy		guards;
		iRewardsCtr	rewards;
	};

public:
	void AddVariant(uint32 probability);
	Item& GetLastVariant() { return m_items.GetLast(); }
	Item& GetVariant(uint32 idx) { return m_items[idx]; }
	const Item& GetVariant(uint32 idx) const { return m_items[idx]; }
	uint32 VariantsCount() const { return m_items.GetSize(); }
	void Serialize(iDynamicBuffer& dbuff) const;
	void Unserialize(iDynamicBuffer& dbuff);

private:
	iSimpleArray<Item>	m_items;
};

#endif //_HMM_COMMON_TREASURY_CONSTRUCTIONS_H_

