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
 *  $Id:  $
 */

#include "tabbedarea.h"

#include <guichan/widgets/container.hpp>

TabbedArea::TabbedArea() : gcn::TabbedArea()
{
    mWidgetContainer->setOpaque(false);
}

int TabbedArea::getNumberOfTabs()
{
    return mTabs.size();
}

gcn::Tab* TabbedArea::getTab(const std::string &name)
{
    std::vector< std::pair<gcn::Tab*, gcn::Widget*> >::iterator itr = mTabs.begin(),
                                                        itr_end = mTabs.end();
    while (itr != itr_end)
    {
        if ((*itr).first->getCaption() == name)
        {
            return (*itr).first;
        }
        ++itr;
    }
    return NULL;
}

void TabbedArea::draw(gcn::Graphics *graphics)
{
    if (mTabs.empty())
    {
        return;
    }

    drawChildren(graphics);
}

gcn::Widget* TabbedArea::getWidget(const std::string &name)
{
    std::vector< std::pair<gcn::Tab*, gcn::Widget*> >::iterator itr = mTabs.begin(),
                                                        itr_end = mTabs.end();
    while (itr != itr_end)
    {
        if ((*itr).first->getCaption() == name)
        {
            return (*itr).second;
        }
        ++itr;
    }

    return NULL;
}

void TabbedArea::removeTab(gcn::Tab *tab)
{
    int tabIndexToBeSelected = 0;

    if (tab == mSelectedTab)
    {
        int index = getSelectedTabIndex();

        if (index == (int)mTabs.size() - 1
                 && mTabs.size() == 1)
        {
            tabIndexToBeSelected = -1;
        }
        else
        {
            tabIndexToBeSelected = index - 1;
        }
    }

    std::vector<std::pair<gcn::Tab*, gcn::Widget*> >::iterator iter;
    for (iter = mTabs.begin(); iter != mTabs.end(); iter++)
    {
        if (iter->first == tab)
        {
            mTabContainer->remove(tab);
            mTabs.erase(iter);
            break;
        }
    }

    std::vector<gcn::Tab*>::iterator iter2;
    for (iter2 = mTabsToDelete.begin(); iter2 != mTabsToDelete.end(); iter2++)
    {
        if (*iter2 == tab)
        {
            mTabsToDelete.erase(iter2);
            delete tab;
            break;
        }
    }

    if (tabIndexToBeSelected == -1)
    {
        mSelectedTab = NULL;
        mWidgetContainer->clear();
    }
    else
    {
        setSelectedTab(tabIndexToBeSelected);
    }

    adjustSize();
    adjustTabPositions();
}
