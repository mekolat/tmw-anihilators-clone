/*
 *  The Mana World
 *  Copyright (C) 2009  The Mana World Development Team
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

#ifndef NET_EA_MAPHANDLER_H
#define NET_EA_MAPHANDLER_H

#include "net/maphandler.h"
#include "net/messagehandler.h"
#include "net/net.h"

class MapHandler : public MessageHandler, public Net::MapHandler
{
    public:
        MapHandler();

        void handleMessage(MessageIn &msg);

        void connect(LoginData *loginData);

        void mapLoaded(const std::string &mapName);

        void who();

        void quit();

        void ping(int tick);
};

extern MapHandler *mapHandler;

#endif // NET_EA_MAPHANDLER_H
