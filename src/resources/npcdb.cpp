/*
 *  The Mana World
 *  Copyright 2008 The Mana World Development Team
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
 *  $Id: monsterdb.cpp 3999 2008-03-23 01:27:13Z b_lindeijer $
 */

#include "npcdb.h"

#include "resourcemanager.h"

#include "../log.h"

#include "../utils/dtor.h"
#include "../utils/xml.h"

namespace
{
    NPCInfos mNPCInfos;
    NPCInfo mUnknown;
    bool mLoaded = false;
}

void NPCDB::load()
{
    if (mLoaded)
        return;

    NPCsprite *unknownSprite = new NPCsprite;
    unknownSprite->sprite = "error.xml";
    unknownSprite->variant = 0;
    mUnknown.push_back(unknownSprite);

    logger->log("Initializing NPC database...");

    XML::Document doc("npcs.xml");
    xmlNodePtr rootNode = doc.rootNode();

    if (!rootNode || !xmlStrEqual(rootNode->name, BAD_CAST "npcs"))
    {
        logger->error("NPC Database: Error while loading items.xml!");
    }

    //iterate <monster>s
    for_each_xml_child_node(npcNode, rootNode)
    {
        if (!xmlStrEqual(npcNode->name, BAD_CAST "npc"))
            continue;

        int id = XML::getProperty(npcNode, "id", 0);
        if (id == 0)
        {
            logger->log("NPC Database: NPC with missing ID in npcs.xml!");
            continue;
        }

        NPCInfo *currentInfo = new NPCInfo;

        for_each_xml_child_node(spriteNode, npcNode)
        {
            if (!xmlStrEqual(spriteNode->name, BAD_CAST "sprite"))
                continue;

            NPCsprite *currentSprite = new NPCsprite;
            currentSprite->sprite = (const char*)spriteNode->xmlChildrenNode->content;
            currentSprite->variant = XML::getProperty(spriteNode, "variant", 0);
            currentInfo->push_back(currentSprite);
        }

        mNPCInfos[id] = currentInfo;

    }

    mLoaded = true;
}

void
NPCDB::unload()
{
    for (   NPCInfosIterator i = mNPCInfos.begin();
            i != mNPCInfos.end();
            i++)
    {
        while (!i->second->empty())
        {
            delete i->second->front();
            i->second->pop_front();
        }
        delete i->second;
    }

    mNPCInfos.clear();

    while (!mUnknown.empty())
    {
        delete mUnknown.front();
        mUnknown.pop_front();
    }

    mLoaded = false;
}

const NPCInfo&
NPCDB::get(int id)
{
    NPCInfosIterator i = mNPCInfos.find(id);

    if (i == mNPCInfos.end())
    {
        logger->log("NPCDB: Warning, unknown NPC ID %d requested", id);
        return mUnknown;
    }
    else
    {
        return *(i->second);
    }
}

