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

#include "stdafx.h"
#include "serialize.h"

static sint32 EventFreqDays[iTimeEvent::EFREQ_COUNT] = {
	0, 1, 2, 3, 4, 5, 6, 7, 14, 21, 28
};

bool iTimeEvent::IsConform(PLAYER_ID pid, uint32 curDay) const
{
	// player
	if ((m_playerMask & (1<<pid)) == 0) return false;
	//
	if (curDay == m_fTime) return true;
	else if (m_repTime == EFREQ_NEVER) return false;
	else return ((curDay-m_fTime)%EventFreqDays[m_repTime]) == 0;
}



