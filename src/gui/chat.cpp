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

#include "chat.h"

#include <sstream>

#include <guichan/focushandler.hpp>
#include <guichan/key.hpp>

#include "browserbox.h"
#include "chatinput.h"
#include "scrollarea.h"
#include "windowcontainer.h"

#include "../game.h"
#include "../localplayer.h"

#include "../net/chatserver/chatserver.h"

#include "../net/gameserver/player.h"

ChatWindow::ChatWindow():
    Window(""),
    mTmpVisible(false)
{
    setWindowName("Chat");
    mItems = 0;
    mItemsKeep = 20;

    setResizable(true);
    setDefaultSize(0, (windowContainer->getHeight() - 123), 600, 100);
    loadWindowState();

    mChatInput = new ChatInput();
    mChatInput->setActionEventId("chatinput");
    mChatInput->addActionListener(this);

    mTextOutput = new BrowserBox(BrowserBox::AUTO_WRAP);
    mTextOutput->setOpaque(false);
    mTextOutput->disableLinksAndUserColors();
    mScrollArea = new ScrollArea(mTextOutput);
    mScrollArea->setPosition(
            mScrollArea->getBorderSize(), mScrollArea->getBorderSize());
    mScrollArea->setScrollPolicy(
            gcn::ScrollArea::SHOW_NEVER, gcn::ScrollArea::SHOW_ALWAYS);
    mScrollArea->setOpaque(false);

    add(mScrollArea);
    add(mChatInput);

    // Add key listener to chat input to be able to respond to up/down
    mChatInput->addKeyListener(this);
    mCurHist = mHistory.end();
}

void
ChatWindow::logic()
{
    // todo: only do this when the size changes (updateWidgets?)

    const gcn::Rectangle area = getChildrenArea();

    mChatInput->setPosition(mChatInput->getBorderSize(),
                            area.height - mChatInput->getHeight() -
                                mChatInput->getBorderSize());
    mChatInput->setWidth(area.width - 2 * mChatInput->getBorderSize());
    mScrollArea->setWidth(area.width - 2 * mScrollArea->getBorderSize());
    mScrollArea->setHeight(area.height - 2 * mScrollArea->getBorderSize() -
            mChatInput->getHeight() - 5);
    mScrollArea->logic();
}

void
ChatWindow::chatLog(std::string line, int own)
{
    // Delete overhead from the end of the list
    while ((int)mChatlog.size() > mItemsKeep) {
        mChatlog.pop_back();
    }

    CHATLOG tmp;
    tmp.own  = own;
    tmp.nick = "";

    // Fix the owner of welcome message.
    if (line.substr(0, 7) == "Welcome")
    {
        own = BY_SERVER;
    }

    int pos = line.find(" : ");
    if (pos > 0) {
        tmp.nick = line.substr(0, pos);
        line.erase(0, pos + 3);
    }

    std::string lineColor = "##0"; // Equiv. to BrowserBox::BLACK
    switch (own) {
        case BY_GM:
            tmp.nick += std::string("Global announcement: ");
            lineColor = "##1"; // Equiv. to BrowserBox::RED
            break;
        case BY_PLAYER:
            tmp.nick += CAT_NORMAL;
            lineColor = "##2"; // Equiv. to BrowserBox::GREEN
            break;
        case BY_OTHER:
            tmp.nick += CAT_NORMAL;
            lineColor = "##4"; // Equiv. to BrowserBox::ORANGE
            break;
        case BY_SERVER:
            tmp.nick += std::string("Server: ");
            lineColor = "##7"; // Equiv. to BrowserBox::PINK
            break;
        case ACT_WHISPER:
            tmp.nick += CAT_WHISPER;
            lineColor = "##3"; // Equiv. to BrowserBox::BLUE
            break;
        case ACT_IS:
            tmp.nick += CAT_IS;
            lineColor = "##5"; // Equiv. to BrowserBox::YELLOW
            break;
    }

    // Get the current system time
    time_t t;
    time(&t);

    // Format the time string properly
    std::stringstream timeStr;
    timeStr << "["
            << ((((t / 60) / 60) % 24 < 10) ? "0" : "")
            << (int)(((t / 60) / 60) % 24)
            << ":"
            << (((t / 60) % 60 < 10) ? "0" : "")
            << (int)((t / 60) % 60)
            << "] ";

    line = lineColor + timeStr.str() + tmp.nick + line;

    // We look if the Vertical Scroll Bar is set at the max before
    // adding a row, otherwise the max will always be a row higher
    // at comparison.
    if (mScrollArea->getVerticalScrollAmount() == mScrollArea->getVerticalMaxScroll() )
    {
        mTextOutput->addRow(line);
        mScrollArea->setVerticalScrollAmount(mScrollArea->getVerticalMaxScroll());
    }
    else
    {
        mTextOutput->addRow(line);
    }
}

void
ChatWindow::chatLog(CHATSKILL act)
{
    chatLog(const_msg(act), BY_SERVER);
}

void
ChatWindow::action(const gcn::ActionEvent &event)
{
    if (event.getId() == "chatinput")
    {
        std::string message = mChatInput->getText();

        if (!message.empty()) {
            // If message different from previous, put it in the history
            if (mHistory.empty() || message != mHistory.back()) {
                mHistory.push_back(message);
            }

            // Reset history iterator
            mCurHist = mHistory.end();

            // Send the message to the server
            chatSend(player_node->getName(), message);

            // Clear the text from the chat input
            mChatInput->setText("");
        }

        // Remove focus and hide input
        mFocusHandler->focusNone();

        // If the chatWindow is shown up because you want to send a message
        // It should hide now
        if (mTmpVisible) {
            setVisible(false);
        }
    }
}

void
ChatWindow::requestChatFocus()
{
    // Make sure chatWindow is visible
    if (!isVisible())
    {
        setVisible(true);

        /*
         * This is used to hide chatWindow after sending the message. There is
         * a trick here, because setVisible will set mTmpVisible to false, you
         * have to put this sentence *after* setVisible, not before it
         */
        mTmpVisible = true;
    }

    // Give focus to the chat input
    mChatInput->setVisible(true);
    mChatInput->requestFocus();
}

bool
ChatWindow::isFocused()
{
    return mChatInput->isFocused();
}

void
ChatWindow::chatSend(const std::string &nick, std::string msg)
{
    /* Some messages are managed client side, while others
     * require server handling by proper packet. Probably
     * those if elses should be replaced by protocol calls */

    // Prepare ordinary message
    if (msg.substr(0, 1) != "/") {
        Net::GameServer::Player::say(msg);
    }
    else if (msg.substr(0, IS_ANNOUNCE_LENGTH) == IS_ANNOUNCE)
    {
        msg.erase(0, IS_ANNOUNCE_LENGTH);
        Net::ChatServer::announce(msg);
    }
    else if (msg.substr(0, IS_HELP_LENGTH) == IS_HELP)
    {
        chatLog("-- Help --", BY_SERVER);
        chatLog("/help : Display this help.", BY_SERVER);
        chatLog("/announce : Global announcement (GM only)", BY_SERVER);
        chatLog("/where : Display map name", BY_SERVER);
        chatLog("/who : Display number of online users", BY_SERVER);
    }
    else if (msg.substr(0, IS_WHERE_LENGTH) == IS_WHERE)
    {
        chatLog(map_path, BY_SERVER);
    }
    else if (msg.substr(0, IS_WHO_LENGTH) == IS_WHO)
    {
        // XXX Convert for new server
        /*
        MessageOut outMsg(0x00c1);
        */
    }
    else
    {
        chatLog("Unknown command", BY_SERVER);
    }
}

std::string
ChatWindow::const_msg(CHATSKILL act)
{
    std::string msg;
    if (act.success == SKILL_FAILED && act.skill == SKILL_BASIC) {
        switch (act.bskill) {
            case BSKILL_TRADE :
                msg = "Trade failed!";
                break;
            case BSKILL_EMOTE :
                msg = "Emote failed!";
                break;
            case BSKILL_SIT :
                msg = "Sit failed!";
                break;
            case BSKILL_CREATECHAT :
                msg = "Chat creating failed!";
                break;
            case BSKILL_JOINPARTY :
                msg = "Could not join party!";
                break;
            case BSKILL_SHOUT :
                msg = "Cannot shout!";
                break;
        }

        switch (act.reason) {
            case RFAIL_SKILLDEP :
                msg += " You have not yet reached a high enough lvl!";
                break;
            case RFAIL_INSUFHP :
                msg += " Insufficient HP!";
                break;
            case RFAIL_INSUFSP :
                msg += " Insufficient SP!";
                break;
            case RFAIL_NOMEMO :
                msg += " You have no memos!";
                break;
            case RFAIL_SKILLDELAY :
                msg += " You cannot do that right now!";
                break;
            case RFAIL_ZENY :
                msg += " Seems you need more Zeny... ;-)";
                break;
            case RFAIL_WEAPON :
                msg += " You cannot use this skill with that kind of weapon!";
                break;
            case RFAIL_REDGEM :
                msg += " You need another red gem!";
                break;
            case RFAIL_BLUEGEM :
                msg += " You need another blue gem!";
                break;
            case RFAIL_OVERWEIGHT :
                msg += " You're carrying to much to do this!";
                break;
            default :
                msg += " Huh? What's that?";
                break;
        }
    } else {
        switch(act.skill) {
            case SKILL_WARP :
                msg = "Warp failed...";
                break;
            case SKILL_STEAL :
                msg = "Could not steal anything...";
                break;
            case SKILL_ENVENOM :
                msg = "Poison had no effect...";
                break;
        }
    }

    return msg;
}

void
ChatWindow::keyPressed(gcn::KeyEvent &event)
{
    if (event.getKey().getValue() == gcn::Key::DOWN &&
            mCurHist != mHistory.end())
    {
        // Move forward through the history
        HistoryIterator prevHist = mCurHist++;
        if (mCurHist != mHistory.end()) {
            mChatInput->setText(*mCurHist);
            mChatInput->setCaretPosition(mChatInput->getText().length());
        }
        else {
            mCurHist = prevHist;
        }
    }
    else if (event.getKey().getValue() == gcn::Key::UP &&
            mCurHist != mHistory.begin() && mHistory.size() > 0)
    {
        // Move backward through the history
        mCurHist--;
        mChatInput->setText(*mCurHist);
        mChatInput->setCaretPosition(mChatInput->getText().length());
    }
}

void
ChatWindow::setInputText(std::string input_str)
{
     mChatInput->setText(input_str + " ");
     requestChatFocus();
}

void
ChatWindow::setVisible(bool isVisible)
{
     Window::setVisible(isVisible);

     /*
      * For whatever reason, if setVisible is called, the mTmpVisible effect
      * should be disabled.
      */
     mTmpVisible = false;
}
