/*
 *  The Mana World
 *  Copyright (C) 2008  Lloyd Bryant <lloyd_bryant@netzero.net>
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

#include "being.h"
#include "localplayer.h"
#include "party.h"

#include "gui/widgets/chattab.h"
#include "gui/chat.h"
#include "gui/confirm_dialog.h"
#include "gui/partywindow.h"

#include "net/messageout.h"
#include "net/ea/protocol.h"
#include "net/ea/gui/partytab.h"

#include "utils/gettext.h"
#include "utils/strprintf.h"

void eAthena::Party::respond(const std::string &command, const std::string &args)
{
    if (command == "new" || command == "create")
    {
        create(args);
        return;
    }
    if (command == "leave")
    {
        leave(args);
        return;
    }
    if (command == "settings")
    {
        partyTab->chatLog(_("Not yet implemented!"), BY_SERVER);
        return;
        /*
        MessageOut outMsg(CMSG_PARTY_SETTINGS);
        outMsg.writeInt16(0); // Experience
        outMsg.writeInt16(0); // Item
        */
    }
    partyTab->chatLog(_("Party command not known."), BY_SERVER);
}

void eAthena::Party::create(const std::string &party)
{
    if (party.empty())
    {
        localChatTab->chatLog(_("Party name is missing."), BY_SERVER);
        return;
    }
    MessageOut outMsg(CMSG_PARTY_CREATE);
    outMsg.writeString(party.substr(0, 23), 24);
}

void eAthena::Party::leave(const std::string &args)
{
    MessageOut outMsg(CMSG_PARTY_LEAVE);
    localChatTab->chatLog(_("Left party."), BY_SERVER);
    player_node->setInParty(false);
}

void eAthena::Party::createResponse(bool ok)
{
    if (ok)
    {
        localChatTab->chatLog(_("Party successfully created."), BY_SERVER);
        player_node->setInParty(true);
    }
    else
    {
        localChatTab->chatLog(_("Could not create party."), BY_SERVER);
    }
}

void eAthena::Party::inviteResponse(const std::string &nick, int status)
{
    switch (status)
    {
        case 0:
            partyTab->chatLog(strprintf(_("%s is already a member of a party."),
                        nick.c_str()), BY_SERVER);
            break;
        case 1:
            partyTab->chatLog(strprintf(_("%s refused your invitation."),
                        nick.c_str()), BY_SERVER);
            break;
        case 2:
            partyTab->chatLog(strprintf(_("%s is now a member of your party."),
                        nick.c_str()), BY_SERVER);
            break;
    }
}

void eAthena::Party::respondToInvite(bool accept)
{
    MessageOut outMsg(CMSG_PARTY_INVITED);
    outMsg.writeInt32(player_node->getId());
    outMsg.writeInt32(accept ? 1 : 0);
    player_node->setInParty(player_node->getInParty() || accept);
}

void eAthena::Party::leftResponse(const std::string &nick)
{
    localChatTab->chatLog(strprintf(_("%s has left your party."), nick.c_str()),
                               BY_SERVER);
    partyWindow->removePartyMember(nick);
}

void eAthena::Party::receiveChat(Being *being, const std::string &msg)
{
    if (!being)
    {
        return;
    }
    if (being->getType() != Being::PLAYER)
    {
        localChatTab->chatLog(_("Party chat received, but being is not a player"),
                       BY_SERVER);
        return;
    }
    being->setSpeech(msg, SPEECH_TIME);
    localChatTab->chatLog(being->getName() + " : " + msg, BY_PARTY);
}

void eAthena::Party::help(const std::string &args)
{
    // Strip "party " from the front
    std::string msg = args.substr(6, args.length());

    if (msg.empty())
    {
        localChatTab->chatLog(_("Command: /party <command> <args>"), BY_SERVER);
        localChatTab->chatLog(_("where <command> can be one of:"), BY_SERVER);
        localChatTab->chatLog(_("   /new"), BY_SERVER);
        localChatTab->chatLog(_("   /create"), BY_SERVER);
        localChatTab->chatLog(_("   /prefix"), BY_SERVER);
        localChatTab->chatLog(_("   /leave"), BY_SERVER);
        localChatTab->chatLog(_("This command implements the partying function."),
                       BY_SERVER);
        localChatTab->chatLog(_("Type /help party <command> for further help."),
                       BY_SERVER);
        return;
    }
    if (msg == "new" || msg == "create")
    {
        localChatTab->chatLog(_("Command: /party new <party-name>"), BY_SERVER);
        localChatTab->chatLog(_("Command: /party create <party-name>"), BY_SERVER);
        localChatTab->chatLog(_("These commands create a new party <party-name."),
                BY_SERVER);
        return;
    }
    if (msg == "prefix")
    {
        localChatTab->chatLog(_("Command: /party prefix <prefix-char>"), BY_SERVER);
        localChatTab->chatLog(_("This command sets the party prefix character."),
                       BY_SERVER);
        localChatTab->chatLog(_("Any message preceded by <prefix-char> is sent to "
                         "the party instead of everyone."), BY_SERVER);
        localChatTab->chatLog(_("Command: /party prefix"), BY_SERVER);
        localChatTab->chatLog(_("This command reports the current party prefix "
                         "character."), BY_SERVER);
        return;
    }
    //if (msg == "settings")
    //if (msg == "info")
    if (msg == "leave")
    {
        localChatTab->chatLog(_("Command: /party leave"), BY_SERVER);
        localChatTab->chatLog(_("This command causes the player to leave the party."),
                BY_SERVER);
        return;
    }
    localChatTab->chatLog(_("Unknown /party command."), BY_SERVER);
    localChatTab->chatLog(_("Type /help party for a list of options."), BY_SERVER);
}
