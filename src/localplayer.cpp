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

#include "localplayer.h"

#include "equipment.h"
#include "floor_item.h"
#include "game.h"
#include "inventory.h"
#include "item.h"
#include "main.h"
#include "sound.h"

#include "net/messageout.h"
#include "net/protocol.h"

LocalPlayer *player_node = NULL;

LocalPlayer::LocalPlayer(Uint32 id, Uint16 job, Map *map):
    Player(id, job, map),
    mLevel(1),
    mInventory(new Inventory()),
    mTarget(NULL), mPickUpTarget(NULL),
    mTrading(false), mLastAction(-1)
{
}

LocalPlayer::~LocalPlayer()
{
}

void LocalPlayer::logic()
{
    switch (mAction) {
        case WALK:
            mFrame = (get_elapsed_time(mWalkTime) * 6) / mWalkSpeed;
            if (mFrame >= 6) {
                nextStep();
            }
            break;

        case ATTACK:
            int frames = 4;
            if (getWeapon() == 2)
            {
                frames = 5;
            }
            mFrame = (get_elapsed_time(mWalkTime) * frames) / mAttackSpeed;
            if (mFrame >= frames) {
                nextStep();
                attack();
            }
            break;
    }

    // Actions are allowed once per second
    if (get_elapsed_time(mLastAction) >= 1000) {
        mLastAction = -1;
    }

    Being::logic();
}

void LocalPlayer::nextStep()
{
    if (mPath.empty() && mPickUpTarget) {
        pickUp(mPickUpTarget);
    }
    Player::nextStep();
}

Being::Type LocalPlayer::getType() const
{
    return LOCALPLAYER;
}

void LocalPlayer::clearInventory()
{
    mInventory->clear();
}

void LocalPlayer::addInvItem(int id, int quantity, bool equipment)
{
    mInventory->addItem(id, quantity, equipment);
}

void LocalPlayer::addInvItem(int index, int id, int quantity, bool equipment)
{
    mInventory->addItem(index, id, quantity, equipment);
}

Item* LocalPlayer::getInvItem(int index)
{
    return mInventory->getItem(index);
}

void LocalPlayer::equipItem(Item *item)
{
    MessageOut outMsg;
    outMsg.writeShort(CMSG_PLAYER_EQUIP);
    outMsg.writeShort(item->getInvIndex());
    outMsg.writeShort(0);
}

void LocalPlayer::unequipItem(Item *item)
{
    if (!item)
        return;

    MessageOut outMsg;
    outMsg.writeShort(CMSG_PLAYER_UNEQUIP);
    outMsg.writeShort(item->getInvIndex());

    // Tidy equipment directly to avoid weapon still shown bug, by instance
    mEquipment->removeEquipment(item);
}

void LocalPlayer::useItem(Item *item)
{
    MessageOut outMsg;
    outMsg.writeShort(CMSG_PLAYER_INVENTORY_USE);
    outMsg.writeShort(item->getInvIndex());
    outMsg.writeLong(item->getId());
    // Note: id is dest of item, usually player_node->account_ID ??
}

void LocalPlayer::dropItem(Item *item, int quantity)
{
    // TODO: Fix wrong coordinates of drops, serverside?
    MessageOut outMsg;
    outMsg.writeShort(CMSG_PLAYER_INVENTORY_DROP);
    outMsg.writeShort(item->getInvIndex());
    outMsg.writeShort(quantity);
}

void LocalPlayer::pickUp(FloorItem *item)
{
    int dx = item->getX() - mX;
    int dy = item->getY() - mY;

    if (dx * dx + dy * dy < 4) {
        MessageOut outMsg;
        outMsg.writeShort(CMSG_ITEM_PICKUP);
        outMsg.writeLong(item->getId());
        mPickUpTarget = NULL;
    } else {
        setDestination(item->getX(), item->getY());
        mPickUpTarget = item;
        stopAttack();
    }
}

void LocalPlayer::walk(unsigned char dir)
{
    if (!mMap || !dir)
        return;

    if (mAction == WALK)
    {
        // Just finish the current action, otherwise we get out of sync
        Being::setDestination(mX, mY);
        return;
    }

    Sint16 dx = 0, dy = 0;
    if (dir & UP)
        dy--;
    if (dir & DOWN)
        dy++;
    if (dir & LEFT)
        dx--;
    if (dir & RIGHT)
        dx++;

    // Prevent skipping corners over colliding tiles
    if (dx && mMap->tileCollides(mX + dx, mY))
        dx = 0;
    if (dy && mMap->tileCollides(mX, mY + dy))
        dy = 0;

    // Choose a straight direction when diagonal target is blocked
    if (dx && dy && !mMap->getWalk(mX + dx, mY + dy))
        dx = 0;

    // Walk to where the player can actually go
    if ((dx || dy) && mMap->getWalk(mX + dx, mY + dy))
    {
        setDestination(mX + dx, mY + dy);
    }
    else if (dir)
    {
        // Update the player direction to where he wants to walk
        // Warning: Not communicated to the server yet

        // If the being can't move, just change direction
        setDirection(dir);
    }
}

void LocalPlayer::setDestination(Uint16 x, Uint16 y)
{
    char temp[3];
    MessageOut outMsg;
    set_coordinates(temp, x, y, mDirection);
    outMsg.writeShort(0x0085);
    outMsg.writeString(temp, 3);

    mPickUpTarget = NULL;

    Being::setDestination(x, y);
}

void LocalPlayer::raiseAttribute(Attribute attr)
{
    MessageOut outMsg;
    outMsg.writeShort(CMSG_STAT_UPDATE_REQUEST);

    switch (attr)
    {
        case STR:
            outMsg.writeShort(0x000d);
            break;

        case AGI:
            outMsg.writeShort(0x000e);
            break;

        case VIT:
            outMsg.writeShort(0x000f);
            break;

        case INT:
            outMsg.writeShort(0x0010);
            break;

        case DEX:
            outMsg.writeShort(0x0011);
            break;

        case LUK:
            outMsg.writeShort(0x0012);
            break;
    }
    outMsg.writeByte(1);
}

void LocalPlayer::raiseSkill(Uint16 skillId)
{
    if (mSkillPoint <= 0)
        return;

    MessageOut outMsg;
    outMsg.writeShort(CMSG_SKILL_LEVELUP_REQUEST);
    outMsg.writeShort(skillId);
}

void LocalPlayer::toggleSit()
{
    if (mLastAction != -1)
        return;
    mLastAction = tick_time;

    char type;
    switch (mAction)
    {
        case STAND: type = 2; break;
        case SIT: type = 3; break;
        default: return;
    }

    MessageOut outMsg;
    outMsg.writeShort(0x0089);
    outMsg.writeLong(0);
    outMsg.writeByte(type);
}

void LocalPlayer::emote(Uint8 emotion)
{
    if (mLastAction != -1)
        return;
    mLastAction = tick_time;

    MessageOut outMsg;
    outMsg.writeShort(0x00bf);
    outMsg.writeByte(emotion);
}

void LocalPlayer::tradeReply(bool accept)
{
    if (!accept)
        mTrading = false;

    MessageOut outMsg;
    outMsg.writeShort(CMSG_TRADE_RESPONSE);
    outMsg.writeByte(accept ? 3 : 4);
}

void LocalPlayer::trade(Being *being) const
{
    MessageOut outMsg;
    outMsg.writeShort(CMSG_TRADE_REQUEST);
    outMsg.writeLong(being->getId());
}

bool LocalPlayer::tradeRequestOk() const
{
    return !mTrading;
}

void LocalPlayer::attack(Being *target, bool keep)
{
    // Can only attack when standing still
    if (mAction != STAND)
        return;

    if (keep && target)
        mTarget = target;
    else if (mTarget)
        target = mTarget;

    if (!target)
        return;

    int dist_x = target->mX - mX;
    int dist_y = target->mY - mY;

    if (abs(dist_y) >= abs(dist_x))
    {
        if (dist_y > 0)
            setDirection(DOWN);
        else
            setDirection(UP);
    }
    else
    {
        if (dist_x > 0)
            setDirection(RIGHT);
        else
            setDirection(LEFT);
    }

    // Implement charging attacks here
    mLastAttackTime = 0;

    setAction(ATTACK);
    mWalkTime = tick_time;
    if (getWeapon() == 2)
        sound.playSfx("sfx/bow_shoot_1.ogg");
    else
        sound.playSfx("sfx/fist-swish.ogg");

    MessageOut outMsg;
    outMsg.writeShort(0x0089);
    outMsg.writeLong(target->getId());
    outMsg.writeByte(0);
}

void LocalPlayer::stopAttack()
{
    mTarget = NULL;
}

Being* LocalPlayer::getTarget() const
{
    return mTarget;
}

void LocalPlayer::revive()
{
    MessageOut outMsg;
    outMsg.writeShort(0x00b2);
    outMsg.writeByte(0);
}
