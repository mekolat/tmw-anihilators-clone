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

#ifndef _TMW_STATUS_H
#define _TMW_STATUS_H

#include <iosfwd>

#include <guichan/actionlistener.hpp>

#include "window.h"

#include "../guichanfwd.h"

class LocalPlayer;
class ProgressBar;


/**
 * The player status dialog.
 *
 * \ingroup Interface
 */
class StatusWindow : public Window, public gcn::ActionListener {
    public:
        /**
         * Constructor.
         */
        StatusWindow(LocalPlayer *player);

         /**
         * Called when receiving actions from widget.
         */
        void action(const std::string& eventId, gcn::Widget* widget);

        /**
         * Draw this window
         */
        void draw(gcn::Graphics *graphics);

        /**
         * Updates this dialog with values from PLAYER_INFO *char_info
         */
        void update();

    private:
        LocalPlayer *mPlayer;

        /**
         * Status Part
         */
        gcn::Label *mLvlLabel, *mMoneyLabel, *mHpLabel, *mHpValueLabel;
        gcn::Label *mMpLabel, *mMpValueLabel;
        gcn::Label *mXpLabel, *mXpValueLabel, *mJobXpLabel, *mJobValueLabel;
        ProgressBar *mHpBar, *mMpBar;
        ProgressBar *mXpBar, *mJobXpBar;

        /**
         * Derived Statistics captions
         */
        gcn::Label *mStatsAttackLabel, *mStatsDefenseLabel;
        gcn::Label *mStatsMagicAttackLabel, *mStatsMagicDefenseLabel;
        gcn::Label *mStatsAccuracyLabel, *mStatsEvadeLabel;
        gcn::Label *mStatsReflexLabel;

        gcn::Label *mStatsAttackPoints, *mStatsDefensePoints;
        gcn::Label *mStatsMagicAttackPoints, *mStatsMagicDefensePoints;
        gcn::Label *mStatsAccuracyPoints, *mStatsEvadePoints;
        gcn::Label *mStatsReflexPoints;

        /**
         * Stats captions.
         */
        gcn::Label *mStatsLabel[6];
        gcn::Label *mPointsLabel[6];
        gcn::Label *mStatsDisplayLabel[6];
        gcn::Label *mRemainingStatsPointsLabel;

        /**
         * Stats buttons.
         */
        gcn::Button *mStatsButton[6];
};

extern StatusWindow *statusWindow;

#endif
