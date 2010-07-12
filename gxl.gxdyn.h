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
// GX library dynamic loader
//

#ifndef GXL_GXDYN_H__
#define GXL_GXDYN_H__

#define GXProps GXDisplayProperties

struct iGAPILoader
{
	HMODULE	gxDLL_;

	int		(*GXOpenDisplay)(HWND hWnd, DWORD dwFlags );
	int		(*GXCloseDisplay)();
	void*	(*GXBeginDraw)();
	int		(*GXEndDraw)();
	int		(*GXSuspend)();
	int		(*GXResume)();
	GXDisplayProperties	(*GXGetDisplayProperties)();
	int		(*GXSetViewport)( DWORD top, DWORD height, DWORD, DWORD );
	BOOL	(*GXIsDisplayDRAMBuffer)();
	int		(*GXOpenInput)();
	int		(*GXCloseInput)();
	GXKeyList (*GXGetDefaultKeys)(int);

	iGAPILoader();
	~iGAPILoader();

	bool Init( bool forceDefault );

	bool IsInitialized() const
	{ return gxDLL_ != 0; }
};

extern iGAPILoader gGapi;


#endif //GXL_GXDYN_H__

