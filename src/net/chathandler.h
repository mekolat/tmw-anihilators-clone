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

#ifndef NET_CHATHANDLER_H
#define NET_CHATHANDLER_H

#include "messagehandler.h"

class ChatHandler : public MessageHandler
{
    public:
        ChatHandler();
        
        /**
         * Handle the given message appropriately.
         */
        void handleMessage(MessageIn &msg);
    
    private:
        /**
         * Handle chat messages sent from the game server.
         */
        void handleGameChatMessage(MessageIn &msg);
        
        /**
         * Handle channel entry responses.
         */
        void handleEnterChannelResponse(MessageIn &msg);
        
        /**
         * Handle list channels responses.
         */
        void handleListChannelsResponse(MessageIn &msg);
        
        /**
         * Handle private messages.
         */
        void handlePrivateMessage(MessageIn &msg);
        
        /**
         * Handle announcements.
         */
        void handleAnnouncement(MessageIn &msg);
        
        /**
         * Handle chat messages.
         */
        void handleChatMessage(MessageIn &msg);
        
        /**
         * Handle quit channel responses.
         */
        void handleQuitChannelResponse(MessageIn &msg);
        
        /**
         * Handle list channel users responses.
         */
        void handleListChannelUsersResponse(MessageIn &msg);
        
        /**
         * Handle channel events.
         */
        void handleChannelEvent(MessageIn &msg);
};

#endif
