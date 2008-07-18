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
 *  $Id: setup_joystick.cpp 4045 2008-04-07 15:23:07Z b_lindeijer $
 */

#include "setup_joystick.h"

#include <guichan/widgets/label.hpp>

#include "button.h"
#include "checkbox.h"
#include "../configuration.h"
#include "../joystick.h"

extern Joystick *joystick;

Setup_Joystick::Setup_Joystick():
    mCalibrateLabel(new gcn::Label("Press the button to start calibration")),
    mCalibrateButton(new Button("Calibrate", "calibrate", this)),
    mJoystickEnabled(new CheckBox("Enable joystick"))
{
    setOpaque(false);
    mJoystickEnabled->setPosition(10, 10);
    mCalibrateLabel->setPosition(10, 25);
    mCalibrateButton->setPosition(10, 30 + mCalibrateLabel->getHeight());

    mOriginalJoystickEnabled = (int)config.getValue("joystickEnabled", 0) != 0;
    mJoystickEnabled->setSelected(mOriginalJoystickEnabled);

    mJoystickEnabled->addActionListener(this);

    add(mCalibrateLabel);
    add(mCalibrateButton);
    add(mJoystickEnabled);
}

void Setup_Joystick::action(const gcn::ActionEvent &event)
{
    if (!joystick) {
        return;
    }

    if (event.getSource() == mJoystickEnabled)
    {
        joystick->setEnabled(mJoystickEnabled->isSelected());
    }
    else
    {
        if (joystick->isCalibrating()) {
            mCalibrateButton->setCaption("Calibrate");
            mCalibrateLabel->setCaption(
                    "Press the button to start calibration");
            joystick->finishCalibration();
        } else {
            mCalibrateButton->setCaption("Stop");
            mCalibrateLabel->setCaption("Rotate the stick");
            joystick->startCalibration();
        }
    }
}

void Setup_Joystick::cancel()
{
    if (joystick)
    {
        joystick->setEnabled(mOriginalJoystickEnabled);
    }
    mJoystickEnabled->setSelected(mOriginalJoystickEnabled);
}

void Setup_Joystick::apply()
{
    config.setValue("joystickEnabled",
                    joystick ? joystick->isEnabled() : false);
}

