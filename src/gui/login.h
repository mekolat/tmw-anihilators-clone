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

#ifndef LOGIN_H
#define LOGIN_H

#include "gui/widgets/window.h"

#include <guichan/actionlistener.hpp>
#include <guichan/keylistener.hpp>
#include <guichan/listmodel.hpp>

#include <string>
#include <vector>

class LoginData;

/**
 * The login dialog.
 *
 * \ingroup Interface
 */
class LoginDialog : public Window, public gcn::ActionListener,
                    public gcn::KeyListener
{
    public:
        /**
         * Constructor
         *
         * @see Window::Window
         */
        LoginDialog(LoginData *loginData);

        ~LoginDialog();

        /**
         * Called when receiving actions from the widgets.
         */
        void action(const gcn::ActionEvent &event);

        /**
         * Called when a key is pressed in one of the text fields.
         */
        void keyPressed(gcn::KeyEvent &keyEvent);

    private:
        /**
         * Returns whether submit can be enabled. This is true in the login
         * state, when all necessary fields have some text.
         */
        bool canSubmit();

        gcn::TextField *mUserField;
        gcn::TextField *mPassField;
        gcn::CheckBox *mKeepCheck;
        gcn::Button *mServerButton;
        gcn::Button *mLoginButton;
        gcn::Button *mRegisterButton;

        LoginData *mLoginData;
};

#endif
