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

#ifndef _TMW_DEBUGWINDOW_H
#define _TMW_DEBUGWINDOW_H

#include <string>

#include <guichan/actionlistener.hpp>

#include "window.h"
#include "../map.h"
#include "../guichanfwd.h"

/**
 * The chat window.
 *
 * \ingroup Interface
 */
class DebugWindow : public Window, public gcn::ActionListener
{
    public:
        /**
         * Constructor.
         */
        DebugWindow();

        /**
         * Destructor.
         */
        ~DebugWindow();

        /**
         * Logic (updates components' size and infos)
         */
        void logic();

        /**
         * Performs action.
         */
        void action(const std::string &actionId);

    private:
        gcn::Label *musicFileLabel, *mapFileLabel;
        gcn::Label *tileMouseLabel, *FPSLabel;
        gcn::Button *closeButton;
        Map *mCurrentMap;

};

extern DebugWindow *debugWindow;

#endif
