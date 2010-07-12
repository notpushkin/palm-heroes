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
// Hardware identification module
#ifndef XXC_HWID_H__
#define XXC_HWID_H__

#include "xxc.fblock.h"

// following approaches are taken
// 1) Using GetDeviceUniqueID if available
// 2) Using KernelIoControl if available and working
// 3) Using gethostname as last resort(*)
// if none is working - bail out
// check internally if the function was really called (results etc)
// if none - bail out "tamper a constants first"

namespace xxc {

void Text2Bin(LPCWSTR text, size_t sizText, unsigned char* bin, size_t sizBin);
void Bin2Text(const unsigned char* bin, size_t sizBin, LPWSTR text, size_t sizText);

void GetDeviceId(xxc::hash& h);

} //xxc

#endif //XXC_HWID_H__

