/*
 *  The Mana World
 *  Copyright 2004 The Mana World Development Team
 *
 *  This file is part of The Mana World.
 *
 *  The Mana World is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  any later version.
 *
 *  The Mana World is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with The Mana World; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 *  $Id$
 */

#include "network.h"

#include "messagehandler.h"
#include "messagein.h"
#include "messageout.h"

#include "../log.h"

static Network::State mState;

/**
 * The local host.
 */
static ENetHost *mClient;

/**
 * An array holding the peers of the account, game and chat servers.
 */
static ENetPeer *mServers[3];

typedef std::map<unsigned short, MessageHandler*> MessageHandlers;
typedef MessageHandlers::iterator MessageHandlerIterator;
static MessageHandlers mMessageHandlers;

Network::State Network::getState() { return mState; }

void Network::initialize()
{
    // Initialize server peers
    for (int i = 0; i < 3; ++i)
        mServers[i] = NULL;

    mClient = enet_host_create(NULL, 3, 0, 0);

    if (!mClient)
    {
        logger->error(
                "An error occurred while trying to create an ENet client.");
        mState = NET_ERROR;
    }
}

void Network::finalize()
{
    clearHandlers();

    disconnect(ACCOUNT);
    disconnect(GAME);
    disconnect(CHAT);
}

bool
Network::connect(Server server, const std::string &address, short port)
{
    logger->log("Network::connect(%d, %s, %i)", server, address.c_str(), port);

    if (address.empty())
    {
        logger->log("Network::connect() got empty address!");
        mState = NET_ERROR;
        return false;
    }

    if (mServers[server] != NULL)
    {
        logger->log("Network::connect() already connected (or connecting) to "
                "this server!");
        return false;
    }

    ENetAddress enetAddress;

    enet_address_set_host(&enetAddress, address.c_str());
    enetAddress.port = port;

    // Initiate the connection, allocating channel 0.
    mServers[server] = enet_host_connect(mClient, &enetAddress, 1);

    if (mServers[server] == NULL)
    {
        logger->log("Unable to initiate connection to the server.");
        mState = NET_ERROR;
        return false;
    }

    return true;
}

void
Network::disconnect(Server server)
{
    if (mServers[server])
    {
        enet_peer_disconnect(mServers[server], 0);
        enet_host_flush(mClient);
        enet_peer_reset(mServers[server]);

        mServers[server] = NULL;
    }
}

void
Network::registerHandler(MessageHandler *handler)
{
    for (const Uint16 *i = handler->handledMessages; *i; i++)
    {
        mMessageHandlers[*i] = handler;
    }
}

void
Network::unregisterHandler(MessageHandler *handler)
{
    for (const Uint16 *i = handler->handledMessages; *i; i++)
    {
        mMessageHandlers.erase(*i);
    }
}

void
Network::clearHandlers()
{
    mMessageHandlers.clear();
}

bool
Network::isConnected(Server server)
{
    return mServers[server] != NULL &&
           mServers[server]->state == ENET_PEER_STATE_CONNECTED;
}

/**
 * Dispatches a message to the appropriate message handler and
 * destroys it afterwards.
 */
static void
dispatchMessage(ENetPacket *packet)
{
    MessageIn msg((const char *)packet->data, packet->dataLength);

    MessageHandlerIterator iter = mMessageHandlers.find(msg.getId());

    if (iter != mMessageHandlers.end()) {
        logger->log("Received packet %x (%i B)",
                    msg.getId(), msg.getLength());
        iter->second->handleMessage(msg);
    }
    else {
        logger->log("Unhandled packet %x (%i B)",
                    msg.getId(), msg.getLength());
    }

    // Clean up the packet now that we're done using it.
    enet_packet_destroy(packet);
}

void Network::flush()
{
    if (mState == NET_ERROR)
    {
        return;
    }

    ENetEvent event;

    // Wait up to 10 milliseconds for an event.
    while (enet_host_service(mClient, &event, 10) > 0)
    {
        switch (event.type)
        {
            case ENET_EVENT_TYPE_CONNECT:
                logger->log("Connected.");
                // Store any relevant server information here.
                event.peer->data = 0;
                break;

            case ENET_EVENT_TYPE_RECEIVE:
                dispatchMessage(event.packet);
                break;

            case ENET_EVENT_TYPE_DISCONNECT:
                logger->log("Disconnected.");
                // Reset the server information.
                event.peer->data = 0;
                break;

            case ENET_EVENT_TYPE_NONE:
                logger->log("No event during 10 milliseconds.");
                break;

            default:
                logger->log("Unhandled enet event.");
                break;
        }
    }
}

void Network::send(Server server, const MessageOut &msg)
{
    if (mState == NET_ERROR)
    {
        logger->log("Warning: attempt to send a message while network not "
                    "ready.");
        return;
    }
    else if (!isConnected(server))
    {
        logger->log("Warning: cannot send message to not connected server %d!",
                    server);
        return;
    }

    logger->log("Sending message of size %d to server %d...",
                msg.getDataSize(), server);

    ENetPacket *packet = enet_packet_create(msg.getData(),
                                            msg.getDataSize(),
                                            ENET_PACKET_FLAG_RELIABLE);
    enet_peer_send(mServers[server], 0, packet);
}

char *iptostring(int address)
{
    static char asciiIP[16];

    sprintf(asciiIP, "%i.%i.%i.%i",
            (unsigned char)(address),
            (unsigned char)(address >> 8),
            (unsigned char)(address >> 16),
            (unsigned char)(address >> 24));

    return asciiIP;
}
