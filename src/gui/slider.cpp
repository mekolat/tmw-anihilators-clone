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

#include "slider.h"
#include "../resources/resourcemanager.h"
#include "../graphics.h"

Slider::Slider(double scaleEnd):
    gcn::Slider(scaleEnd)
{
    init();
}

Slider::Slider(double scaleStart, double scaleEnd):
    gcn::Slider(scaleStart, scaleEnd)
{
    init();
}

void Slider::init()
{
    int x, y, w, h,o1,o2;
    setBorderSize(0);

    // Load resources
    ResourceManager *resman = ResourceManager::getInstance();
    Image *slider = resman->getImage("core/graphics/gui/slider.png");

    x = 0; y = 0;
    w = 15; h = 6;
    o1 = 4; o2 = 11;
    hStart = slider->getSubImage(x, y, o1 - x, h);
    hMid = slider->getSubImage(o1, y, o2 - o1, h);
    hEnd = slider->getSubImage(o2, y, w - o2 + x, h);

    x = 0; y = 6;
    w = 6; h = 21;
    o1 = 10; o2 = 18;
    vStart = slider->getSubImage(x, y, w, o1 - y);
    vMid = slider->getSubImage(x, o1, w, o2 - o1);
    vEnd = slider->getSubImage(x, o2, w, h - o2 + y);

    x = 6; y = 8;
    w = 9; h = 10;
    vGrip = slider->getSubImage(x, y, w, h);

    x = 6; y = 8;
    w = 9; h = 10;
    hGrip = slider->getSubImage(x, y, w, h);

    mMarkerWidth = hGrip->getWidth();
}

void Slider::draw(gcn::Graphics *graphics)
{
    int w = getWidth();
    int h = getHeight();
    int x, y;
    getAbsolutePosition(x, y);

    y += (h - hStart->getHeight()) / 2;

    hStart->draw(screen, x, y);

    w -= hStart->getWidth() + hEnd->getWidth();
    x += hStart->getWidth();

    hMid->drawPattern(screen, x, y, w, hMid->getHeight());

    x += w;
    hEnd->draw(screen, x, y);

    drawMarker(graphics);
}

void Slider::drawMarker(gcn::Graphics *graphics)
{
    int h = getHeight();
    int x, y;
    getAbsolutePosition(x, y);

    x += mMarkerPosition;
    y += (h - hGrip->getHeight()) / 2;

    hGrip->draw(screen, x, y);
}
