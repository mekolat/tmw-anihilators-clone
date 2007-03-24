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

#include "register.h"

#include <string>
#include <sstream>

#include <guichan/widgets/label.hpp>

#include "../main.h"
#include "../configuration.h"
#include "../log.h"
#include "../logindata.h"

#include "button.h"
#include "checkbox.h"
#include "login.h"
#include "passwordfield.h"
#include "radiobutton.h"
#include "textfield.h"
#include "ok_dialog.h"

void
WrongDataNoticeListener::setTarget(gcn::TextField *textField)
{
    mTarget = textField;
}

void
WrongDataNoticeListener::action(const gcn::ActionEvent &event)
{
    if (event.getId() == "ok")
    {
        mTarget->requestFocus();
    }
}

RegisterDialog::RegisterDialog(LoginData *loginData):
    Window("Register"),
    mWrongDataNoticeListener(new WrongDataNoticeListener()),
    mLoginData(loginData)
{
    gcn::Label *userLabel = new gcn::Label("Name:");
    gcn::Label *passwordLabel = new gcn::Label("Password:");
    gcn::Label *confirmLabel = new gcn::Label("Confirm:");
    gcn::Label *serverLabel = new gcn::Label("Server:");
    mUserField = new TextField(loginData->username);
    mPasswordField = new PasswordField(loginData->password);
    mConfirmField = new PasswordField();
    mServerField = new TextField(loginData->hostname);
    mMaleButton = new RadioButton("Male", "sex", true);
    mFemaleButton = new RadioButton("Female", "sex", false);
    mRegisterButton = new Button("Register", "register", this);
    mCancelButton = new Button("Cancel", "cancel", this);

    const int width = 220;
    const int height = 150;
    setContentSize(width, height);

    mUserField->setPosition(65, 5);
    mUserField->setWidth(width - 70);
    mPasswordField->setPosition(
            65, mUserField->getY() + mUserField->getHeight() + 7);
    mPasswordField->setWidth(mUserField->getWidth());
    mConfirmField->setPosition(
            65, mPasswordField->getY() + mPasswordField->getHeight() + 7);
    mConfirmField->setWidth(mUserField->getWidth());
    mServerField->setPosition(
            65, 23 + mConfirmField->getY() + mConfirmField->getHeight() + 7);
    mServerField->setWidth(mUserField->getWidth());

    userLabel->setPosition(5, mUserField->getY() + 1);
    passwordLabel->setPosition(5, mPasswordField->getY() + 1);
    confirmLabel->setPosition(5, mConfirmField->getY() + 1);
    serverLabel->setPosition(5, mServerField->getY() + 1);

    mMaleButton->setPosition(
            70, mConfirmField->getY() + mConfirmField->getHeight() + 7);
    mFemaleButton->setPosition(
            70 + 10 + mMaleButton->getWidth(),
            mMaleButton->getY());

    mCancelButton->setPosition(
            width - mCancelButton->getWidth() - 5,
            height - mCancelButton->getHeight() - 5);
    mRegisterButton->setPosition(
            mCancelButton->getX() - mRegisterButton->getWidth() - 5,
            height - mRegisterButton->getHeight() - 5);

    mUserField->addKeyListener(this);
    mPasswordField->addKeyListener(this);
    mConfirmField->addKeyListener(this);
    mServerField->addKeyListener(this);

    /* TODO:
     * This is a quick and dirty way to respond to the ENTER key, regardless of
     * which text field is selected. There may be a better way now with the new
     * input system of Guichan 0.6.0. See also the login dialog.
     */
    mUserField->setActionEventId("register");
    mPasswordField->setActionEventId("register");
    mConfirmField->setActionEventId("register");
    mServerField->setActionEventId("register");
    mUserField->addActionListener(this);
    mPasswordField->addActionListener(this);
    mConfirmField->addActionListener(this);
    mServerField->addActionListener(this);

    add(userLabel);
    add(passwordLabel);
    add(serverLabel);
    add(confirmLabel);
    add(mUserField);
    add(mPasswordField);
    add(mConfirmField);
    add(mServerField);
    add(mMaleButton);
    add(mFemaleButton);
    add(mRegisterButton);
    add(mCancelButton);

    setLocationRelativeTo(getParent());
    setVisible(true);
    mUserField->requestFocus();
    mUserField->setCaretPosition(mUserField->getText().length());

    mRegisterButton->setEnabled(canSubmit());
}

RegisterDialog::~RegisterDialog()
{
    delete mWrongDataNoticeListener;
}

void
RegisterDialog::action(const gcn::ActionEvent &event)
{
    if (event.getId() == "cancel")
    {
        state = LOGIN_STATE;
    }
    else if (event.getId() == "register" && canSubmit())
    {
        const std::string user = mUserField->getText();
        logger->log("RegisterDialog::register Username is %s", user.c_str());

        std::stringstream errorMsg;
        int error = 0;

        if (user.length() < LEN_MIN_USERNAME)
        {
            // Name too short
            errorMsg << "The username needs to be at least "
                     << LEN_MIN_USERNAME
                     << " characters long.";
            error = 1;
        }
        else if (user.length() > LEN_MAX_USERNAME - 1 )
        {
            // Name too long
            errorMsg << "The username needs to be less than "
                     << LEN_MAX_USERNAME
                     << " characters long.";
            error = 1;
        }
        else if (mPasswordField->getText().length() < LEN_MIN_PASSWORD)
        {
            // Pass too short
            errorMsg << "The password needs to be at least "
                     << LEN_MIN_PASSWORD
                     << " characters long.";
            error = 2;
        }
        else if (mPasswordField->getText().length() > LEN_MAX_PASSWORD - 1 )
        {
            // Pass too long
            errorMsg << "The password needs to be less than "
                     << LEN_MAX_PASSWORD
                     << " characters long.";
            error = 2;
        }
        else if (mPasswordField->getText() != mConfirmField->getText())
        {
            // Password does not match with the confirmation one
            errorMsg << "Passwords do not match.";
            error = 2;
        }

        if (error > 0)
        {
            if (error == 1)
            {
                mWrongDataNoticeListener->setTarget(this->mUserField);
            }
            else if (error == 2)
            {
                // Reset password confirmation
                mPasswordField->setText("");
                mConfirmField->setText("");

                mWrongDataNoticeListener->setTarget(this->mPasswordField);
            }

            OkDialog *mWrongRegisterNotice = new OkDialog("Error",
                                                          errorMsg.str());
            mWrongRegisterNotice->addActionListener(mWrongDataNoticeListener);
        }
        else
        {
            // No errors detected, register the new user.
            mRegisterButton->setEnabled(false);

            mLoginData->hostname = mServerField->getText();
            mLoginData->port = (short) config.getValue("port", 0);
            mLoginData->username = mUserField->getText();
            mLoginData->password = mPasswordField->getText();
            mLoginData->username += mFemaleButton->isMarked() ? "_F" : "_M";
            mLoginData->registerLogin = true;

            state = ACCOUNT_STATE;
        }
    }
}

void
RegisterDialog::keyPressed(gcn::KeyEvent &keyEvent)
{
    mRegisterButton->setEnabled(canSubmit());
}

bool
RegisterDialog::canSubmit()
{
    return !mUserField->getText().empty() &&
           !mPasswordField->getText().empty() &&
           !mConfirmField->getText().empty() &&
           !mServerField->getText().empty() &&
           state == REGISTER_STATE;
}
