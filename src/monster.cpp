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

#include "monster.h"

#include "animatedsprite.h"
#include "game.h"
#include "sound.h"

#include "resources/monsterdb.h"

#include "utils/tostring.h"


Monster::Monster(Uint16 id, Uint16 job, Map *map):
    Being(id, job, map)
{
    mSprites[BASE_SPRITE] = new AnimatedSprite(
            "graphics/sprites/" + MonsterDB::get(job - 1002).getSprite());
}

Being::Type
Monster::getType() const
{
    return MONSTER;
}

void
Monster::setAction(Action action)
{
    SpriteAction currentAction = ACTION_INVALID;

    switch (action)
    {
        case WALK:
            currentAction = ACTION_WALK;
            break;
        case DEAD:
            currentAction = ACTION_DEAD;
            sound.playSfx(MonsterDB::get(mJob - 1002).getSound(EVENT_DIE));
            break;
        case ATTACK:
            currentAction = ACTION_ATTACK;
            sound.playSfx(MonsterDB::get(mJob - 1002).getSound(EVENT_HIT));
            mSprites[BASE_SPRITE]->reset();
            break;
        case STAND:
            currentAction = ACTION_STAND;
            break;
        case HURT:
            // Not implemented yet
            break;
        default:
            break;
    }

    if (currentAction != ACTION_INVALID)
    {
        mSprites[BASE_SPRITE]->play(currentAction);
        mAction = action;
    }
}
