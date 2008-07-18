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
 *  $Id: being.cpp 4301 2008-05-28 16:06:48Z peaveydk $
 */
#include "being.h"

#include <algorithm>
#include <cassert>

#include "animatedsprite.h"
#include "equipment.h"
#include "game.h"
#include "graphics.h"
#include "log.h"
#include "map.h"
#include "particle.h"
#include "text.h"

#include "resources/resourcemanager.h"
#include "resources/imageset.h"
#include "resources/iteminfo.h"

#include "gui/gui.h"

#include "utils/dtor.h"
#include "utils/tostring.h"

int Being::instances = 0;
ImageSet *Being::emotionSet = NULL;

static const int X_SPEECH_OFFSET = 18;
static const int Y_SPEECH_OFFSET = 60;

Being::Being(int id, int job, Map *map):
    mJob(job),
    mX(0), mY(0),
    mAction(0),
    mWalkTime(0),
    mEmotion(0), mEmotionTime(0),
    mAttackSpeed(350),
    mEquipment(new Equipment()),
    mId(id),
    mWalkSpeed(150),
    mDirection(DOWN),
    mMap(NULL),
    mEquippedWeapon(NULL),
    mHairStyle(1), mHairColor(0),
    mGender(2),
    mSpeechTime(0),
    mPx(0), mPy(0),
    mSprites(VECTOREND_SPRITE, NULL),
    mSpriteIDs(VECTOREND_SPRITE, 0),
    mSpriteColors(VECTOREND_SPRITE, "")
{
    setMap(map);

    if (instances == 0)
    {
        // Load the emotion set
        ResourceManager *rm = ResourceManager::getInstance();
        emotionSet = rm->getImageSet("graphics/gui/emotions.png", 30, 32);
        if (!emotionSet) logger->error("Unable to load emotions!");
    }

    instances++;
    mSpeech = 0;
}

Being::~Being()
{
    std::for_each(mSprites.begin(), mSprites.end(), make_dtor(mSprites));
    clearPath();

    for (   std::list<Particle *>::iterator i = mChildParticleEffects.begin();
            i != mChildParticleEffects.end();
            i++)
    {
        (*i)->kill();
    }

    setMap(NULL);

    instances--;

    if (instances == 0)
    {
        emotionSet->decRef();
        emotionSet = NULL;
    }
    if (mSpeech)
    {
        delete mSpeech;
    }
}

void
Being::setDestination(Uint16 destX, Uint16 destY)
{
    if (mMap)
    {
        setPath(mMap->findPath(mX, mY, destX, destY));
    }
}

void
Being::clearPath()
{
    mPath.clear();
}

void
Being::setPath(const Path &path)
{
    mPath = path;

    if (mAction != WALK && mAction != DEAD)
    {
        nextStep();
        mWalkTime = tick_time;
    }
}

void
Being::setHairStyle(int style, int color)
{
    mHairStyle = style < 0 ? mHairStyle : style % NR_HAIR_STYLES;
    mHairColor = color < 0 ? mHairColor : color % NR_HAIR_COLORS;
}

void
Being::setSprite(int slot, int id, std::string color)
{
    assert(slot >= BASE_SPRITE && slot < VECTOREND_SPRITE);
    mSpriteIDs[slot] = id;
    mSpriteColors[slot] = color;
}

void
Being::setSpeech(const std::string &text, Uint32 time)
{
    if (mSpeech) // don't introduce a memory leak
    {
        delete mSpeech;
    }
    mSpeech = new Text(text, mPx + X_SPEECH_OFFSET, mPy - Y_SPEECH_OFFSET,
                       gcn::Graphics::CENTER, speechFont,
                       gcn::Color(255, 255, 255));
    mSpeechTime = 500;
}

void
Being::takeDamage(int amount)
{
    gcn::Font *font;
    std::string damage = amount ? toString(amount) : "miss";

    // Selecting the right color
    if (damage == "miss")
    {
        font = hitYellowFont;
    }
    else
    {
        // Hit particle effect
        controlParticle(particleEngine->addEffect(
                    "graphics/particles/hit.particle.xml", 0, 0));

        if (getType() == MONSTER)
        {
            font = hitBlueFont;
        }
        else
        {
            font = hitRedFont;
        }
    }

    // Show damage number
    particleEngine->addTextSplashEffect(damage, 255, 255, 255, font,
                                        mPx + 16, mPy + 16);
}

void
Being::handleAttack(Being *victim, int damage)
{
    setAction(Being::ATTACK);
    mFrame = 0;
    mWalkTime = tick_time;
}

void
Being::setMap(Map *map)
{
    // Remove sprite from potential previous map
    if (mMap)
    {
        mMap->removeSprite(mSpriteIterator);
    }

    mMap = map;

    // Add sprite to potential new map
    if (mMap)
    {
        mSpriteIterator = mMap->addSprite(this);
    }

    // Clear particle effect list because child particles became invalid
    mChildParticleEffects.clear();
}

void
Being::controlParticle(Particle *particle)
{
    if (particle)
    {
        // The effect may not die without the beings permission or we segfault
        particle->disableAutoDelete();
        mChildParticleEffects.push_back(particle);
    }
}

void
Being::setAction(Uint8 action)
{
    SpriteAction currentAction = ACTION_INVALID;
    switch (action)
    {
        case WALK:
            currentAction = ACTION_WALK;
            break;
        case SIT:
            currentAction = ACTION_SIT;
            break;
        case ATTACK:
            if (mEquippedWeapon)
            {
                currentAction = mEquippedWeapon->getAttackType();
            }
            else {
                currentAction = ACTION_ATTACK;
            }
            for (int i = 0; i < VECTOREND_SPRITE; i++)
            {
                if (mSprites[i])
                {
                    mSprites[i]->reset();
                }
            }
            break;
        case HURT:
            //currentAction = ACTION_HURT;  // Buggy: makes the player stop
                                            // attacking and unable to attack
                                            // again until he moves
            break;
        case DEAD:
            currentAction = ACTION_DEAD;
            break;
        case STAND:
            currentAction = ACTION_STAND;
            break;
    }

    if (currentAction != ACTION_INVALID)
    {
        for (int i = 0; i < VECTOREND_SPRITE; i++)
        {
            if (mSprites[i])
            {
                mSprites[i]->play(currentAction);
            }
        }
        mAction = action;
    }
}


void
Being::setDirection(Uint8 direction)
{
    mDirection = direction;
    SpriteDirection dir = getSpriteDirection();

    for (int i = 0; i < VECTOREND_SPRITE; i++)
    {
        if (mSprites[i] != NULL)
            mSprites[i]->setDirection(dir);
    }
}

SpriteDirection
Being::getSpriteDirection() const
{
    SpriteDirection dir;

    if (mDirection & UP)
    {
        dir = DIRECTION_UP;
    }
    else if (mDirection & DOWN)
    {
        dir = DIRECTION_DOWN;
    }
    else if (mDirection & RIGHT)
    {
        dir = DIRECTION_RIGHT;
    }
    else {
        dir = DIRECTION_LEFT;
    }

    return dir;
}

void
Being::nextStep()
{
    if (mPath.empty())
    {
        setAction(STAND);
        return;
    }

    PATH_NODE node = mPath.front();
    mPath.pop_front();

    int dir = 0;
    if (node.x > mX)
        dir |= RIGHT;
    else if (node.x < mX)
        dir |= LEFT;
    if (node.y > mY)
        dir |= DOWN;
    else if (node.y < mY)
        dir |= UP;

    setDirection(dir);

    if (mMap->tileCollides(node.x, node.y))
    {
        setAction(STAND);
        return;
    }

    mX = node.x;
    mY = node.y;
    setAction(WALK);
    mWalkTime += mWalkSpeed / 10;
}

void
Being::logic()
{
    // Reduce the time that speech is still displayed
    if (mSpeechTime > 0 && mSpeech)
    {
        if (--mSpeechTime == 0)
        {
            delete mSpeech;
            mSpeech = 0;
        }
    }

    int oldPx = mPx;
    int oldPy = mPy;
    // Update pixel coordinates
    mPx = mX * 32 + getXOffset();
    mPy = mY * 32 + getYOffset();
    if (mPx != oldPx || mPy != oldPy)
    {
        if (mSpeech)
        {
            mSpeech->adviseXY(mPx + X_SPEECH_OFFSET, mPy - Y_SPEECH_OFFSET);
        }
        updateCoords();
    }
    if (mEmotion != 0)
    {
        mEmotionTime--;
        if (mEmotionTime == 0) {
            mEmotion = 0;
        }
    }

    // Update sprite animations
    for (int i = 0; i < VECTOREND_SPRITE; i++)
    {
        if (mSprites[i] != NULL)
        {
            mSprites[i]->update(tick_time * 10);
        }
    }

    //Update particle effects
    for (   std::list<Particle *>::iterator i = mChildParticleEffects.begin();
            i != mChildParticleEffects.end();

        )
    {
        (*i)->setPosition((float)mPx + 16.0f, (float)mPy + 32.0f);
        if (!(*i)->isAlive())
        {
            (*i)->kill();
            i = mChildParticleEffects.erase(i);
        }
        else {
            i++;
        }
    }
}

void
Being::draw(Graphics *graphics, int offsetX, int offsetY) const
{
    int px = mPx + offsetX;
    int py = mPy + offsetY;

    for (int i = 0; i < VECTOREND_SPRITE; i++)
    {
        if (mSprites[i] != NULL)
        {
            mSprites[i]->draw(graphics, px, py);
        }
    }
}

void
Being::drawEmotion(Graphics *graphics, int offsetX, int offsetY)
{
    if (!mEmotion)
        return;

    const int px = mPx + offsetX + 3;
    const int py = mPy + offsetY - 60;
    const int emotionIndex = mEmotion - 1;

    if (emotionIndex >= 0 && emotionIndex < (int) emotionSet->size())
        graphics->drawImage(emotionSet->get(emotionIndex), px, py);
}

Being::Type
Being::getType() const
{
    return UNKNOWN;
}

int
Being::getOffset(char pos, char neg) const
{
    // Check whether we're walking in the requested direction
    if (mAction != WALK || !(mDirection & (pos | neg))) {
        return 0;
    }

    int offset = (get_elapsed_time(mWalkTime) * 32) / mWalkSpeed;

    // We calculate the offset _from_ the _target_ location
    offset -= 32;
    if (offset > 0) {
        offset = 0;
    }

    // Going into negative direction? Invert the offset.
    if (mDirection & pos) {
        offset = -offset;
    }

    return offset;
}


int
Being::getWidth() const
{
    if (mSprites[BASE_SPRITE])
    {
        return mSprites[BASE_SPRITE]->getWidth();
    }
    else {
        return 0;
    }
}


int
Being::getHeight() const
{
    if (mSprites[BASE_SPRITE])
    {
        return mSprites[BASE_SPRITE]->getHeight();
    }
    else {
        return 0;
    }
}
