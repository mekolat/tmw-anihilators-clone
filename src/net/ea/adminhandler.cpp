/*
 *  The Mana World
 *  Copyright (C) 2004  The Mana World Development Team
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

#include "net/ea/adminhandler.h"

#include "net/ea/protocol.h"

#include "net/messagein.h"
#include "net/messageout.h"

#include "being.h"
#include "beingmanager.h"
#include "game.h"
#include "player_relations.h"

#include "gui/widgets/chattab.h"

#include "utils/gettext.h"
#include "utils/stringutils.h"
#include "utils/strprintf.h"

#include <string>

#define SERVER_NAME "Server"

AdminHandler *adminHandler;

AdminHandler::AdminHandler()
{
    static const Uint16 _messages[] = {
        0
    };
    handledMessages = _messages;
    adminHandler = this;
}

void AdminHandler::handleMessage(MessageIn &msg)
{
    switch (msg.getId())
    {
        case SMSG_WHISPER_RESPONSE:
            break;
    }
}

void AdminHandler::announce(const std::string &text)
{
    MessageOut outMsg(CMSG_ADMIN_ANNOUNCE);
    outMsg.writeInt16(text.length() + 4);
    outMsg.writeString(text, text.length());
}

void AdminHandler::localAnnounce(const std::string &text)
{
    MessageOut outMsg(CMSG_ADMIN_LOCAL_ANNOUNCE);
    outMsg.writeInt16(text.length() + 4);
    outMsg.writeString(text, text.length());
}

void AdminHandler::hide(bool hide)
{
    MessageOut outMsg(CMSG_ADMIN_HIDE);
    outMsg.writeInt32(0); //unused
}

void AdminHandler::kick(int playerId)
{
    MessageOut outMsg(CMSG_ADMIN_KICK);
    outMsg.writeInt32(playerId);
}

void AdminHandler::kick(const std::string &name)
{
    // Unsupported
}

void AdminHandler::ban(int playerId)
{}

void AdminHandler::ban(const std::string &name)
{}

void AdminHandler::unban(int playerId)
{}

void AdminHandler::unban(const std::string &name)
{}
