/*
 *  The Mana World
 *  Copyright (C) 2004  The Mana World Development Team
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

#ifndef GUI_NPCLISTDIALOG_H
#define GUI_NPCLISTDIALOG_H

#include "window.h"

#include <guichan/actionlistener.hpp>
#include <guichan/listmodel.hpp>

#include <vector>

/**
 * The npc list dialog.
 *
 * \ingroup Interface
 */
class NpcListDialog : public Window, public gcn::ActionListener,
                      public gcn::ListModel
{
    public:
        /**
         * Constructor.
         *
         * @see Window::Window
         */
        NpcListDialog();

        /**
         * Called when receiving actions from the widgets.
         */
        void action(const gcn::ActionEvent &event);

        /**
         * Returns the number of items in the choices list.
         */
        int getNumberOfElements();

        /**
         * Returns the name of item number i of the choices list.
         */
        std::string getElementAt(int i);

        /**
         * Adds an item to the option list.
         */
        void addItem(const std::string &);

        /**
         * Fills the options list for an NPC dialog.
         *
         * @param itemString A string with the options separated with colons.
         */
        void parseItems(const std::string &itemString);

        /**
         * Resets the list by removing all items.
         */
        void reset();

    private:
        gcn::ListBox *mItemList;

        std::vector<std::string> mItems;
};

#endif // GUI_NPCLISTDIALOG_H
