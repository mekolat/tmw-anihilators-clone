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

#include "text.h"

#include <cstring>

#include <guichan/font.hpp>

#include "configuration.h"
#include "textmanager.h"
#include "resources/resourcemanager.h"
#include "resources/image.h"

int Text::mInstances = 0;
ImageRect Text::mBubble;
Image *Text::mBubbleArrow;

Text::Text(const std::string &text, int x, int y,
           gcn::Graphics::Alignment alignment, gcn::Font *font,
           gcn::Color colour, bool isSpeech) :
    mText(text),
    mColour(colour),
    mIsSpeech(isSpeech)
{
    if (textManager == 0)
    {
        textManager = new TextManager;
        ResourceManager *resman = ResourceManager::getInstance();
        Image *sbImage = resman->getImage("graphics/gui/bubble.png|W:#"
            + config.getValue("speechBubbleColour", "000000"));
        mBubble.grid[0] = sbImage->getSubImage(0, 0, 5, 5);
        mBubble.grid[1] = sbImage->getSubImage(5, 0, 5, 5);
        mBubble.grid[2] = sbImage->getSubImage(10, 0, 5, 5);
        mBubble.grid[3] = sbImage->getSubImage(0, 5, 5, 5);
        mBubble.grid[4] = sbImage->getSubImage(5, 5, 5, 5);
        mBubble.grid[5] = sbImage->getSubImage(10, 5, 5, 5);
        mBubble.grid[6] = sbImage->getSubImage(0, 10, 5, 5);
        mBubble.grid[7] = sbImage->getSubImage(5, 10, 5, 5);
        mBubble.grid[8] = sbImage->getSubImage(10, 10, 5, 5);
        mBubbleArrow = sbImage->getSubImage(0, 15, 15, 10);
        double bubbleAlpha = config.getValue("speechBubbleAlpha", 0.4);
        for (int i = 0; i < 9; i++)
        {
             mBubble.grid[i]->setAlpha(bubbleAlpha);
        }
        mBubbleArrow->setAlpha(bubbleAlpha);
        sbImage->decRef();
    }
    ++mInstances;
    mHeight = font->getHeight();
    mWidth = font->getWidth(text);

    switch (alignment)
    {
        case gcn::Graphics::LEFT:
            mXOffset = 0;
            break;
        case gcn::Graphics::CENTER:
            mXOffset = mWidth / 2;
            break;
        case gcn::Graphics::RIGHT:
            mXOffset = mWidth;
            break;
    }
    mX = x - mXOffset;
    mY = y;
    textManager->addText(this);
    mFont = font;
}

Text::~Text()
{
    textManager->removeText(this);
    if (--mInstances == 0)
    {
        delete textManager;
        textManager = 0;
        delete mBubble.grid[0];
        delete mBubble.grid[1];
        delete mBubble.grid[2];
        delete mBubble.grid[3];
        delete mBubble.grid[4];
        delete mBubble.grid[5];
        delete mBubble.grid[6];
        delete mBubble.grid[7];
        delete mBubble.grid[8];
        delete mBubbleArrow;
    }
}

void Text::adviseXY(int x, int y)
{
    textManager->moveText(this, x - mXOffset, y);
}

void Text::draw(Graphics *graphics, int xOff, int yOff)
{
    graphics->setFont(mFont);
    graphics->setColor(mColour);

    if (mIsSpeech) {
        static_cast<Graphics*>(graphics)->drawImageRect(
                mX - xOff - 5, mY - yOff - 5, mWidth + 10, mHeight + 10,
                mBubble);

        if (mWidth >= 15) {
            static_cast<Graphics*>(graphics)->drawImage(
                    mBubbleArrow, mX - xOff - 7 + mWidth / 2,
                    mY - yOff + mHeight + 4);
        }
    }

    graphics->drawText(mText, mX - xOff, mY - yOff,
            gcn::Graphics::LEFT);
}

FlashText::FlashText(const std::string &text, int x, int y,
                     gcn::Graphics::Alignment alignment, gcn::Font *font,
                     gcn::Color colour) :
    Text(text, x, y, alignment, font, colour), mTime(0)
{
}

void FlashText::draw(Graphics *graphics, int xOff, int yOff)
{
    if (mTime)
    {
        if ((--mTime & 4) == 0)
        {
            return;
        }
    }
    Text::draw(graphics, xOff, yOff);
}
