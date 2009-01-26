/*
 *  The Mana World
 *  Copyright (C) 2008  The Legend of Mazzeroth Development Team
 *  Copyright (C) 2008  The Mana World Development Team
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
 */

#include <guichan/font.hpp>

#include <guichan/widgets/label.hpp>

#include "gui.h"
#include "itempopup.h"
#include "windowcontainer.h"

#include "widgets/layout.h"

#include "../resources/image.h"
#include "../resources/iteminfo.h"
#include "../resources/resourcemanager.h"

#include "../utils/gettext.h"
#include "../utils/strprintf.h"
#include "../utils/tostring.h"

ItemPopup::ItemPopup():
    Window()
{
    setResizable(false);
    setShowTitle(false);
    setTitleBarHeight(0);

    // Item Name
    mItemName = new gcn::Label("Label");
    mItemName->setFont(gui->getFont());
    mItemName->setPosition(2, 2);
    mItemName->setWidth(getWidth() - 4);

    // Item Description
    mItemDesc = new TextBox();
    mItemDesc->setEditable(false);
    mItemDesc->setMinWidth(186);
    mItemDesc->setTextWrapped("");
    mItemDescScroll = new ScrollArea(mItemDesc);

    mItemDescScroll->setHorizontalScrollPolicy(gcn::ScrollArea::SHOW_NEVER);
    mItemDescScroll->setVerticalScrollPolicy(gcn::ScrollArea::SHOW_NEVER);
    mItemDescScroll->setDimension(gcn::Rectangle(0, 0, 196, getFont()->getHeight()));
    mItemDescScroll->setOpaque(false);
    mItemDescScroll->setPosition(2, getFont()->getHeight());

    // Item Effect
    mItemEffect = new TextBox();
    mItemEffect->setEditable(false);
    mItemEffect->setMinWidth(186);
    mItemEffect->setTextWrapped("");
    mItemEffectScroll = new ScrollArea(mItemEffect);

    mItemEffectScroll->setHorizontalScrollPolicy(gcn::ScrollArea::SHOW_NEVER);
    mItemEffectScroll->setVerticalScrollPolicy(gcn::ScrollArea::SHOW_NEVER);
    mItemEffectScroll->setDimension(gcn::Rectangle(0, 0, 196, getFont()->getHeight()));
    mItemEffectScroll->setOpaque(false);
    mItemEffectScroll->setPosition(2, (2 * getFont()->getHeight()) + 5);

    // Item Weight
    mItemWeight = new TextBox();
    mItemWeight->setEditable(false);
    mItemWeight->setMinWidth(186);
    mItemWeight->setTextWrapped("");
    mItemWeightScroll = new ScrollArea(mItemWeight);

    mItemWeightScroll->setHorizontalScrollPolicy(gcn::ScrollArea::SHOW_NEVER);
    mItemWeightScroll->setVerticalScrollPolicy(gcn::ScrollArea::SHOW_NEVER);
    mItemWeightScroll->setDimension(gcn::Rectangle(0, 0, 196, getFont()->getHeight()));
    mItemWeightScroll->setOpaque(false);
    mItemWeightScroll->setPosition(2, (3 * getFont()->getHeight()) + 10);

    add(mItemName);
    add(mItemDescScroll);
    add(mItemEffectScroll);
    add(mItemWeightScroll);

    setLocationRelativeTo(getParent());

    // LEEOR / TODO: This causes an exception error.
    //moveToBottom(getParent());
}

void ItemPopup::setItem(const ItemInfo &item)
{
    const gcn::Rectangle &area = getChildrenArea();
    const int width = area.width;

    mItemDesc->setMinWidth(width - 10);
    mItemEffect->setMinWidth(width - 10);
    mItemWeight->setMinWidth(width - 10);

    mItemName->setCaption(item.getName());
    mItemDesc->setTextWrapped(item.getDescription());
    mItemEffect->setTextWrapped(item.getEffect());
    mItemWeight->setTextWrapped(_("Weight: ") + toString(item.getWeight()) + 
                                _(" grams"));

    int numRowsDesc = mItemDesc->getNumberOfRows();
    int numRowsEffect = mItemEffect->getNumberOfRows();
    int numRowsWeight = mItemWeight->getNumberOfRows();

    mItemDescScroll->setDimension(gcn::Rectangle(2, 0, 196,
                                  numRowsDesc * getFont()->getHeight()));

    mItemEffectScroll->setDimension(gcn::Rectangle(2, 0, 196,
                                    numRowsEffect * getFont()->getHeight()));

    mItemWeightScroll->setDimension(gcn::Rectangle(2, 0, 196,
                                    numRowsWeight * getFont()->getHeight()));

    if(item.getEffect() == "")
    {
        setContentSize(200, (numRowsDesc * getFont()->getHeight() + 
                      (3 * getFont()->getHeight())));

        mItemWeightScroll->setPosition(2, 
                          (numRowsDesc * getFont()->getHeight()) +
                          (2 * getFont()->getHeight()));
    }
    else
    {
        setContentSize(200, (numRowsDesc * getFont()->getHeight()) + 
                      (numRowsEffect * getFont()->getHeight()) +
                      (3 * getFont()->getHeight()));

        mItemWeightScroll->setPosition(2, 
                          (numRowsDesc * getFont()->getHeight()) +
                          (numRowsEffect * getFont()->getHeight()) +
                          (2 * getFont()->getHeight()));
    }

    mItemDescScroll->setPosition(2, 20);
    mItemEffectScroll->setPosition(2, (numRowsDesc * getFont()->getHeight()) +
                      (2 * getFont()->getHeight()));
}

unsigned int ItemPopup::getNumRows()
{
    return mItemDesc->getNumberOfRows() + mItemEffect->getNumberOfRows() +
           mItemWeight->getNumberOfRows();
}

void ItemPopup::view(int x, int y)
{
    if (windowContainer->getWidth() < (x + getWidth() + 5))
	x = windowContainer->getWidth() - getWidth();
    if ((y - getHeight() - 5) < 0)
	y = 0;
    else
        y = y - getHeight() - 5;
    setPosition(x, y);
    setVisible(true);
    requestMoveToTop();
}
