/*
 *  The Mana World
 *  Copyright (C) 2008  The Mana World Development Team
 *
 *  This file is part of The Mana World.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "whispertab.h"

#include "localplayer.h"

#include "gui/palette.h"

#include "net/chathandler.h"
#include "net/net.h"

#include "utils/gettext.h"
#include "utils/strprintf.h"

WhisperTab::WhisperTab(const std::string &nick) :
    ChatTab(nick),
    mNick(nick)
{
    setTabColor(guiPalette->getColor(Palette::WHISPER));
}

WhisperTab::~WhisperTab()
{
    chatWindow->removeWhisper(mNick);
}

void WhisperTab::handleInput(const std::string &msg) {
    if (msg.length() == 0) {
        chatLog(_("Cannot send empty chat!"), BY_SERVER, false);
        return;
    }

    Net::getChatHandler()->privateMessage(mNick, msg);

    chatLog(strprintf(_("%s: %s"), player_node->getName().c_str(),
                        msg.c_str()), BY_PLAYER, false);
}

void WhisperTab::handleCommand(std::string msg)
{
    if (msg == "close")
        delete this;
    else
        ChatTab::handleCommand(msg);
}
