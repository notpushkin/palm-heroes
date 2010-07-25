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
#include "Dlg_Register.h"

#define MICROSOFT_EMULATOR_ID 0x3940622c
extern uint32 gOemInfoHash;

LPCTSTR const KBD_CHARS = _T("1234567890ACEFGHJKLMNPQRSTUVWXYZ");

// Edit control
class iEditCtrl : public iBaseCtrl 
{
public:
	iEditCtrl(iViewMgr* pViewMgr, IViewCmdHandler* pCmdHandler,  const iRect& rect, uint32 uid, uint32 groups, uint32 cpgroup)
	: iBaseCtrl(pViewMgr, pCmdHandler, rect, GENERIC_VIEWPORT, uid, Visible|Enabled), m_groups(groups), m_cpgroup(cpgroup), m_curPos(0)
	{
		m_pChars = new iCharT[groups*cpgroup];
		memset(m_pChars, 0, groups*cpgroup*sizeof(iCharT));
	}

	~iEditCtrl()
	{ delete[] m_pChars; }

	void OnCompose()
	{
		iRect orc = GetScrRect();
		
		iRect grc (orc.x, orc.y, m_cpgroup*9, 13);

		iCharT* pc = m_pChars;
		for (uint32 xx=0; xx<m_groups; ++xx) {
			gApp.Surface().Darken25Rect(grc);
			ButtonFrame(gApp.Surface(), grc, iButton::Pressed);
			if (xx == m_curPos / m_cpgroup) gApp.Surface().FillRect(iRect(grc.x+1,grc.y+1,grc.w-2,grc.h-2), RGB16(0,0,128), 64);
			/*
			iRect crc(grc.x+3, grc.y, 7, 13);
			for (uint32 cc=0; cc<m_cpgroup; ++cc, ++pc) {
				if (*pc) gTextComposer.TextOut(gApp.Surface(), iPoint(), iStringT(*pc,1),crc,AlignCenter);
				crc.x+=8;
			}*/
			gTextComposer.TextOut(gApp.Surface(), iPoint(), iStringT(pc,m_cpgroup),grc,AlignCenter);
			pc += m_cpgroup;
			grc.x += m_cpgroup*9 + 5;
		}
	}

	void ProcessChar(iCharT sym)
	{
		if (sym == 0) {
			if (m_curPos > 0) {
				m_pChars[--m_curPos] = 0;
				m_pCmdHandler->iCMDH_ControlCommand(this, CCI_EDITCHANGED, m_curPos);
			}
		} else {
			if (m_curPos < (m_groups*m_cpgroup)) {
				m_pChars[m_curPos++] = sym;
				m_pCmdHandler->iCMDH_ControlCommand(this, CCI_EDITCHANGED, m_curPos);
			}
		}
	}

	static iSize CalcCtrlSize(uint32 groups, uint32 cpgroup)
	{
		return iSize( cpgroup*9*groups + 5*(groups-1), 13);
	}

	sint32	m_curPos;
	iCharT*	m_pChars;
	uint32	m_groups;
	uint32	m_cpgroup;

};

// Keyboard button
class iKbdButton : public iButton, public IViewCmdHandler
{
public:
	iKbdButton(iViewMgr* pViewMgr, const iRect& rect, iCharT sym, uint32 uid, iEditCtrl* pEdit)
	: iButton(pViewMgr, this, rect, uid, Visible|Enabled), m_sym(sym), m_pEdit(pEdit)
	{}

	void OnCompose()
	{
		iRect orc = GetScrRect();
		gGfxMgr.BlitTile(PDGG_BKTILE,gApp.Surface(),orc);
		ButtonFrame(gApp.Surface(),orc,GetButtonState());

		if (m_sym != 0) {
			iTextComposer::FontConfig fc(iTextComposer::FS_MEDIUM, iDibFont::ComposeProps(cColor_Gray192));
			gTextComposer.TextOut(fc,gApp.Surface(),orc, iStringT(m_sym, 1),orc,AlignCenter);
		} else {
			gApp.Surface().FillRect(orc, RGB16(128,0,0), 48);
			BlitIcon(gApp.Surface(), PDGG_BTN_BKSP, orc);
		}

		orc.InflateRect(1);
		FrameRoundRect(gApp.Surface(), orc, cColor_Black);
		//ComposeTextButton(gApp.Surface(),GetScrRect(),GetButtonState(),m_TextKey);
	}

	void iCMDH_ControlCommand(iView* pView, CTRL_CMD_ID cmd, sint32 param)
	{
		m_pEdit->ProcessChar(m_sym);
	}

private:
	iCharT		m_sym;
	iEditCtrl*	m_pEdit;
};

//////////////////////////////////////////////////////////////////////////
iDlg_Register::iDlg_Register(iViewMgr* pViewMgr)
: iBaseGameDlg(pViewMgr,PID_NEUTRAL)
{
#ifdef OS_WINCE
	xxc::hash h;
	uint32 res[8];
	h.reset();
	xxc::GetDeviceId(h);
	h.finalize( res );
	m_devId = res[0] + res[1] + res[2];
#else
	m_devId = 0xDEADBABE;
#endif

}

void iDlg_Register::OnCreateDlg()
{
	iRect clRect = ClientRect();

	sint32 ypos = clRect.y;

	// Header
	AddChild(new iPHLabel(m_pMgr, iRect(clRect.x, ypos,clRect.w, 15), gTextMgr[TRID_REG_REGISTRATION], AlignCenter, dlgfc_hdr));
	ypos += 18;

	// Device Id labels
	AddChild(new iPHLabel(m_pMgr, iRect(clRect.x, ypos,clRect.w, 15), gTextMgr[TRID_REG_HWID], AlignCenter, dlgfc_topic));
	ypos += 15;
	AddChild(new iPHLabel(m_pMgr, iRect(clRect.x, ypos,clRect.w, 15), iFormat(_T("%08X"), m_devId), AlignCenter, iTextComposer::FontConfig(iTextComposer::FS_MEDIUM, cColor_White)));
	ypos += 18;

	// Reg Key Label
	AddChild(new iPHLabel(m_pMgr, iRect(clRect.x, ypos,clRect.w, 15), gTextMgr[TRID_REG_REGKEY], AlignCenter, dlgfc_topic));
	ypos += 17;

	// Edit control
	iSize edSiz = iEditCtrl::CalcCtrlSize(4,6);
	m_pEditCtrl = new iEditCtrl(m_pMgr, this, iRect(ClientRect().x+11, ypos, edSiz.w, edSiz.h), 200, 4, 6);
	AddChild(m_pEditCtrl);
	ypos += 20;

	// Keyboard
	sint32 kw = 19;
	sint32 xpos = clRect.x + 16;
	uint32 cid;
	for (cid = 0; cid<32; ++cid) {
		AddChild(new iKbdButton(m_pMgr, iRect(xpos, ypos, kw, 15), KBD_CHARS[cid], 100 + cid, m_pEditCtrl));
		if ( ((cid+1) % 11) == 0) {
			xpos = clRect.x + 16;
			ypos += 16;
		} else xpos += kw+1;
	}
	AddChild(new iKbdButton(m_pMgr, iRect(xpos, ypos, kw, 15), 0, 100 + cid, m_pEditCtrl));

	// Buttons
	iRect btnRc(clRect.x + clRect.w/2-100, clRect.y2()-DEF_BTN_HEIGHT, 80, DEF_BTN_HEIGHT);
	AddChild(m_pRegBtn = new iTextButton(m_pMgr, this, btnRc, TRID_REG_REGISTER, DRC_OK, Visible));
	btnRc.x+=85; btnRc.w=55;
	AddChild(new iTextButton(m_pMgr, this, btnRc, TRID_REG_LATER, DRC_CANCEL));
	btnRc.x+=60;
	AddChild(new iTextButton(m_pMgr, this, btnRc, TRID_HELP, 300));
}

void iDlg_Register::DoCompose(const iRect& clRect)
{
	iRect rc(clRect);

	// title
	gTextComposer.TextOut(dlgfc_hdr, gApp.Surface(),rc.point(),gTextMgr[TRID_REG_REGISTRATION], iRect(rc.x,rc.y,rc.w,15),AlignCenter);
	rc.y+=17;
}

iSize iDlg_Register::ClientSize() const
{
	return iSize(255, 145 + DEF_BTN_HEIGHT);
}

void iDlg_Register::iCMDH_ControlCommand(iView* pView, CTRL_CMD_ID cmd, sint32 param)
{

	uint32 uid = pView->GetUID();
	if (uid == DRC_OK ) {
		gSettings.SetActivationKey(iStringT(m_pEditCtrl->m_pChars, m_pEditCtrl->m_groups*m_pEditCtrl->m_cpgroup));
		// this would prevent setting activation key
		gEnterNewKey = (bool)(gOemInfoHash != MICROSOFT_EMULATOR_ID);
		EndDialog(DRC_OK);
	} else if (uid == DRC_CANCEL){
		EndDialog(DRC_CANCEL);
	} else if (uid == 200){
		// edit
		if (cmd == CCI_EDITCHANGED) {
			m_pRegBtn->SetEnabled(param == 24);
		}
	} else if (uid == 300){
#define HANDANGO_VER
#if defined(POCKETLAND_VER)
		iIconDlg tdlg(m_pMgr, gTextMgr[TRID_REG_REGISTRATION], gTextMgr[TRID_REG_HELP_POCKETLAND], PDGG_BANNER_POCKETLAND, PID_NEUTRAL);
#elif defined(HANDANGO_VER)
		iTextDlg tdlg(m_pMgr, gTextMgr[TRID_REG_REGISTRATION], gTextMgr[TRID_REG_HELP_HANDANGO], PID_NEUTRAL);
#else
		iTextDlg tdlg(m_pMgr, gTextMgr[TRID_REG_REGISTRATION], gTextMgr[TRID_REG_HELP], PID_NEUTRAL);
#endif
		tdlg.DoModal();
		Invalidate();
	}
}


