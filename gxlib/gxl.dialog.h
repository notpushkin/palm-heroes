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

#ifndef _GXLIB_GENERIC_DIALOG_H_
#define _GXLIB_GENERIC_DIALOG_H_

enum DLG_RETCODE {
	DRC_UNDEFINED = -1,
	DRC_OK,
	DRC_CANCEL,
	DRC_YES,
	DRC_NO
};

class iDialog : public iView
{
public:
	iDialog(iViewMgr* pViewMgr);
	virtual ~iDialog() {}

	sint32 DoModal();

	// Pure virtuals
	virtual iSize GetDlgMetrics() const =0;
	virtual void OnCreateDlg() =0;
	virtual void OnPlace(iRect& rect) {}

	// Virtuals
	virtual bool KeyDown(sint32 key) { return false; }
	virtual bool KeyUp(sint32 key) { return false; }

protected:
	inline bool IsValidDialog() const { return m_retCode == DRC_UNDEFINED; }
	void Center();
	bool EndDialog(sint32 retCode);
	void Invalidate();

private:
	sint32	m_retCode;
};

#endif //_GXLIB_GENERIC_DIALOG_H_
