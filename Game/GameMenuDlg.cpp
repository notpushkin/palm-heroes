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
#include "GameMenuDlg.h"


//Settings, Save game, Main menu, Quit, Return to game
iGameMenuDlg::iGameMenuDlg(iViewMgr* pViewMgr)
: iBaseGameDlg(pViewMgr,PID_NEUTRAL)
{ 
}

void iGameMenuDlg::OnCreateDlg()
{
	bool registered = IS_REGISTERED();
	iRect rc = ClientRect();
	rc.h = DEF_BTN_HEIGHT+2;

	AddChild(new iTextButton(m_pMgr, this, rc, TRID_MENU_SAVEGAME, GMRC_SAVE, (registered)?(Visible|Enabled):Visible));
	rc.y+=DEF_BTN_HEIGHT+5;
	AddChild(new iTextButton(m_pMgr, this, rc, TRID_MENU_LOADGAME, GMRC_LOAD, (registered)?(Visible|Enabled):Visible));
	rc.y+=DEF_BTN_HEIGHT+5;
	AddChild(new iTextButton(m_pMgr, this, rc, TRID_MENU_EXITTOMAINMENU, GMRC_MAINMENU, Visible|Enabled));
	rc.y+=DEF_BTN_HEIGHT+5;
	AddChild(new iTextButton(m_pMgr, this, rc, TRID_MENU_EXITGAME, GMRC_QUIT, Visible|Enabled));
	rc.y+=DEF_BTN_HEIGHT+15;
	AddChild(new iTextButton(m_pMgr, this, rc, TRID_MENU_RETURNTOGAME, GMRC_RETURN, Visible|Enabled));
}

iSize iGameMenuDlg::ClientSize() const
{
	return iSize(120,5*DEF_BTN_HEIGHT+3*5+15);
}

void iGameMenuDlg::iCMDH_ControlCommand(iView* pView, CTRL_CMD_ID cmd, sint32 param)
{
	EndDialog(pView->GetUID());
}

