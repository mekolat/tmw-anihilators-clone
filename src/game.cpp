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

#include "beingmanager.h"
#include "channelmanager.h"
#include "commandhandler.h"
#include "configuration.h"
#include "effectmanager.h"
#include "emoteshortcut.h"
#include "engine.h"
#include "flooritemmanager.h"
#include "game.h"
#include "graphics.h"
#include "itemshortcut.h"
#include "joystick.h"
#include "keyboardconfig.h"
#include "localplayer.h"
#include "log.h"
#include "main.h"
#include "map.h"
#include "npc.h"
#include "particle.h"
#include "playerrelations.h"
#include "sound.h"

#include "gui/widgets/chattab.h"
#include "gui/buy.h"
#include "gui/buysell.h"
#include "gui/chat.h"
#include "gui/confirmdialog.h"
#include "gui/debugwindow.h"
#include "gui/emoteshortcutcontainer.h"
#include "gui/equipmentwindow.h"
#include "gui/gui.h"
#include "gui/help.h"
#include "gui/inventorywindow.h"
#include "gui/shortcutwindow.h"
#include "gui/itemshortcutcontainer.h"
#include "gui/minimap.h"
#include "gui/ministatus.h"
#include "gui/npcdialog.h"
#include "gui/okdialog.h"
#include "gui/outfitwindow.h"
#include "gui/sdlinput.h"
#include "gui/sell.h"
#include "gui/setup.h"
#include "gui/skilldialog.h"
#include "gui/statuswindow.h"
#include "gui/trade.h"
#include "gui/viewport.h"
#include "gui/windowmenu.h"
#include "gui/partywindow.h"
#ifdef TMWSERV_SUPPORT
#include "gui/buddywindow.h"
#include "gui/guildwindow.h"
#endif
#include "gui/npcpostdialog.h"
#include "gui/quitdialog.h"
#include "gui/specialswindow.h"
#include "gui/storagewindow.h"

#include "net/gamehandler.h"
#include "net/generalhandler.h"
#include "net/net.h"

#include "net/tmwserv/inventoryhandler.h"
#include "net/ea/inventoryhandler.h"

#include "resources/imagewriter.h"

#include "utils/gettext.h"

#include <guichan/exception.hpp>
#include <guichan/focushandler.hpp>

#include <fstream>
#include <physfs.h>
#include <sstream>
#include <string>

std::string map_path;

bool done = false;
volatile int tick_time;
volatile int fps = 0, frame = 0;

Engine *engine = NULL;
Joystick *joystick = NULL;

extern Window *weightNotice;
extern Window *deathNotice;
QuitDialog *quitDialog = NULL;
OkDialog *disconnectedDialog = NULL;

ChatWindow *chatWindow;
StatusWindow *statusWindow;
MiniStatusWindow *miniStatusWindow;
BuyDialog *buyDialog;
SellDialog *sellDialog;
#ifdef EATHENA_SUPPORT
BuySellDialog *buySellDialog;
#endif
InventoryWindow *inventoryWindow;
SkillDialog *skillDialog;
PartyWindow *partyWindow;
#ifdef TMWSERV_SUPPORT
BuddyWindow *buddyWindow;
GuildWindow *guildWindow;
#endif
NpcDialog *npcDialog;
NpcPostDialog *npcPostDialog;
StorageWindow *storageWindow;
Minimap *minimap;
EquipmentWindow *equipmentWindow;
TradeWindow *tradeWindow;
HelpWindow *helpWindow;
DebugWindow *debugWindow;
ShortcutWindow *itemShortcutWindow;
ShortcutWindow *emoteShortcutWindow;
OutfitWindow *outfitWindow;
SpecialsWindow *specialsWindow;

BeingManager *beingManager = NULL;
FloorItemManager *floorItemManager = NULL;
ChannelManager *channelManager = NULL;
CommandHandler *commandHandler = NULL;
Particle *particleEngine = NULL;
EffectManager *effectManager = NULL;

ChatTab *localChatTab = NULL;

/**
 * Tells the max tick value,
 * setting it back to zero (and start again).
 */
const int MAX_TICK_VALUE = 10000;
/**
 * Set the milliseconds value of a tick time.
 */
const int MILLISECONDS_IN_A_TICK = 10;

/**
 * Listener used for exiting handling.
 */
namespace {
    class ExitListener : public gcn::ActionListener
    {
    public:
        void action(const gcn::ActionEvent &event)
        {
            if (event.getId() == "yes" || event.getId() == "ok")
                done = true;

            disconnectedDialog = NULL;
        }
    } exitListener;
}

/**
 * Advances game logic counter.
 * Called every 10 milliseconds by SDL_AddTimer()
 * @see MILLISECONDS_IN_A_TICK value
 */
Uint32 nextTick(Uint32 interval, void *param)
{
    tick_time++;
    if (tick_time == MAX_TICK_VALUE) tick_time = 0;
    return interval;
}

/**
 * Updates fps.
 * Called every seconds by SDL_AddTimer()
 */
Uint32 nextSecond(Uint32 interval, void *param)
{
    fps = frame;
    frame = 0;

    return interval;
}

/**
 * @return the elapsed time in milliseconds
 * between two tick values.
 */
int get_elapsed_time(int start_time)
{
    if (start_time <= tick_time)
        return (tick_time - start_time) * MILLISECONDS_IN_A_TICK;
    else
        return (tick_time + (MAX_TICK_VALUE - start_time))
                * MILLISECONDS_IN_A_TICK;
}

/**
 * Create all the various globally accessible gui windows
 */
static void createGuiWindows()
{
    setupWindow->clearWindowsForReset();

    // Create dialogs
    chatWindow = new ChatWindow;
    buyDialog = new BuyDialog;
    sellDialog = new SellDialog;
    tradeWindow = new TradeWindow;
    partyWindow = new PartyWindow;
#ifdef TMWSERV_SUPPORT
    buddyWindow = new BuddyWindow;
    guildWindow = new GuildWindow;
#else
    buySellDialog = new BuySellDialog;
#endif
    equipmentWindow = new EquipmentWindow(player_node->mEquipment.get());
    npcDialog = new NpcDialog;
    npcPostDialog = new NpcPostDialog;
    storageWindow = new StorageWindow;
    statusWindow = new StatusWindow;
    miniStatusWindow = new MiniStatusWindow;
    inventoryWindow = new InventoryWindow;
    skillDialog = new SkillDialog;
    minimap = new Minimap;
    helpWindow = new HelpWindow;
    debugWindow = new DebugWindow;
    itemShortcutWindow = new ShortcutWindow("ItemShortcut",
                                            new ItemShortcutContainer);
    emoteShortcutWindow = new ShortcutWindow("EmoteShortcut",
                                             new EmoteShortcutContainer);
    outfitWindow = new OutfitWindow();
    specialsWindow = new SpecialsWindow();

    localChatTab = new ChatTab(_("General"));

    if (config.getValue("logToChat", 0))
    {
        logger->setChatWindow(chatWindow);
    }

    Net::getGeneralHandler()->guiWindowsLoaded();
}

/**
 * Destroy all the globally accessible gui windows
 */
static void destroyGuiWindows()
{
    Net::getGeneralHandler()->guiWindowsUnloaded();
    logger->setChatWindow(NULL);
    delete localChatTab; // Need to do this first, so it can remove itself
    delete chatWindow;
    delete statusWindow;
    delete miniStatusWindow;
    delete buyDialog;
    delete sellDialog;
#ifdef EATHENA_SUPPORT
    delete buySellDialog;
#endif
    delete inventoryWindow;
    delete partyWindow;
    delete npcDialog;
    delete npcPostDialog;
#ifdef TMWSERV_SUPPORT
    delete buddyWindow;
    delete guildWindow;
#endif
    delete skillDialog;
    delete minimap;
    delete equipmentWindow;
    delete tradeWindow;
    delete helpWindow;
    delete debugWindow;
    delete itemShortcutWindow;
    delete emoteShortcutWindow;
    delete storageWindow;
    delete outfitWindow;
    delete specialsWindow;
}

Game::Game():
    mLastTarget(Being::UNKNOWN),
    mLogicCounterId(0), mSecondsCounterId(0)
{
    done = false;

    createGuiWindows();

    mWindowMenu = new WindowMenu;
    windowContainer->add(mWindowMenu);

    engine = new Engine;

    beingManager = new BeingManager;
    commandHandler = new CommandHandler;
    floorItemManager = new FloorItemManager;
    channelManager = new ChannelManager;
    effectManager = new EffectManager;

    particleEngine = new Particle(NULL);
    particleEngine->setupEngine();

    // Initialize logic and seconds counters
    tick_time = 0;
    mLogicCounterId = SDL_AddTimer(MILLISECONDS_IN_A_TICK, nextTick, NULL);
    mSecondsCounterId = SDL_AddTimer(1000, nextSecond, NULL);

    // Initialize frame limiting
    config.addListener("fpslimit", this);
    optionChanged("fpslimit");

    // Initialize beings
    beingManager->setPlayer(player_node);

    Joystick::init();
    // TODO: The user should be able to choose which one to use
    // Open the first device
    if (Joystick::getNumberOfJoysticks() > 0)
        joystick = new Joystick(0);

    // fade out logon-music here too to give the desired effect of "flowing"
    // into the game.
    sound.fadeOutMusic(1000);
    map_path = map_path.substr(0, map_path.rfind("."));

    if (!map_path.empty())
        engine->changeMap(map_path);

    setupWindow->setInGame(true);

    /*
     * To prevent the server from sending data before the client
     * has initialized, I've modified it to wait for a "ping"
     * from the client to complete its initialization
     *
     * Note: This only affects the latest eAthena version.  This
     * packet is handled by the older version, but its response
     * is ignored by the client
     */
    Net::getGameHandler()->ping(tick_time);
}

Game::~Game()
{
    delete mWindowMenu;

    destroyGuiWindows();

    delete beingManager;
    delete player_node;
    delete floorItemManager;
    delete channelManager;
    delete commandHandler;
    delete joystick;
    delete particleEngine;
    delete engine;

    viewport->setMap(NULL);
    player_node = NULL;
    beingManager = NULL;
    floorItemManager = NULL;
    joystick = NULL;

    SDL_RemoveTimer(mLogicCounterId);
    SDL_RemoveTimer(mSecondsCounterId);
}

static bool saveScreenshot()
{
    static unsigned int screenshotCount = 0;

    SDL_Surface *screenshot = graphics->getScreenshot();

    // Search for an unused screenshot name
    std::stringstream filenameSuffix;
    std::stringstream filename;
    std::fstream testExists;
    bool found = false;

    do {
        screenshotCount++;
        filename.str("");
        filenameSuffix.str("");
        filename << PHYSFS_getUserDir();
#if (defined __USE_UNIX98 || defined __FreeBSD__)
        filenameSuffix << ".tmw/";
#elif defined __APPLE__
        filenameSuffix << "Desktop/";
#endif
        filenameSuffix << "TMW_Screenshot_" << screenshotCount << ".png";
        filename << filenameSuffix.str();
        testExists.open(filename.str().c_str(), std::ios::in);
        found = !testExists.is_open();
        testExists.close();
    } while (!found);

    const bool success = ImageWriter::writePNG(screenshot, filename.str());

    if (success)
    {
        std::stringstream chatlogentry;
        // TODO: Make it one complete gettext string below
        chatlogentry << _("Screenshot saved to ~/") << filenameSuffix.str();
        localChatTab->chatLog(chatlogentry.str(), BY_SERVER);
    }
    else
    {
        localChatTab->chatLog(_("Saving screenshot failed!"), BY_SERVER);
        logger->log("Error: could not save screenshot.");
    }

    SDL_FreeSurface(screenshot);

    return success;
}

void Game::optionChanged(const std::string &name)
{
    int fpsLimit = (int) config.getValue("fpslimit", 60);

    mMinFrameTime = fpsLimit ? 1000 / fpsLimit : 0;

    // Reset draw time to current time
    mDrawTime = tick_time * MILLISECONDS_IN_A_TICK;
}

void Game::logic()
{
    // mDrawTime has a higher granularity than gameTime in order to be able to
    // work with minimum frame durations in milliseconds.
    int gameTime = tick_time;
    mDrawTime = tick_time * MILLISECONDS_IN_A_TICK;

    while (!done)
    {
        if (Map *map = engine->getCurrentMap())
            map->update(get_elapsed_time(gameTime));

        // Handle all necessary game logic
        while (get_elapsed_time(gameTime) > 0)
        {
            handleInput();
            engine->logic();
            gameTime++;
        }

        // This is done because at some point tick_time will wrap.
        gameTime = tick_time;

        // Update the screen when application is active, delay otherwise.
        if (SDL_GetAppState() & SDL_APPACTIVE)
        {
            // Draw a frame if either frames are not limited or enough time has
            // passed since the last frame.
            if (!mMinFrameTime ||
                get_elapsed_time(mDrawTime / 10) > mMinFrameTime)
            {
                frame++;
                gui->draw();
                graphics->updateScreen();
                mDrawTime += mMinFrameTime;

                // Make sure to wrap mDrawTime, since tick_time will wrap.
                if (mDrawTime > MAX_TICK_VALUE * MILLISECONDS_IN_A_TICK)
                    mDrawTime -= MAX_TICK_VALUE * MILLISECONDS_IN_A_TICK;
            }
            else
            {
                SDL_Delay(MILLISECONDS_IN_A_TICK);
            }
        }
        else
        {
            SDL_Delay(MILLISECONDS_IN_A_TICK);
            mDrawTime = tick_time * MILLISECONDS_IN_A_TICK;
        }

        // Handle network stuff
        Net::getGeneralHandler()->flushNetwork();
        if (!Net::getGameHandler()->isConnected())
        {
            if (state != STATE_ERROR)
            {
                errorMessage = _("The connection to the server was lost, "
                                 "the program will now quit");
            }

            if (!disconnectedDialog)
            {
                disconnectedDialog = new OkDialog(_("Network Error"), errorMessage);
                disconnectedDialog->addActionListener(&exitListener);
                disconnectedDialog->requestMoveToTop();
            }
        }
    }
}

void Game::handleInput()
{
    if (joystick)
        joystick->update();

    // Events
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        bool used = false;

        // Keyboard events (for discontinuous keys)
        if (event.type == SDL_KEYDOWN)
        {
            gcn::Window *requestedWindow = NULL;

            if (setupWindow->isVisible() &&
                keyboard.getNewKeyIndex() > keyboard.KEY_NO_VALUE)
            {
                keyboard.setNewKey((int) event.key.keysym.sym);
                keyboard.callbackNewKey();
                keyboard.setNewKeyIndex(keyboard.KEY_NO_VALUE);
                return;
            }

            // send straight to gui for certain windows
            if (npcPostDialog->isVisible())
            {
                try
                {
                    guiInput->pushInput(event);
                }
                catch (gcn::Exception e)
                {
                    const char* err = e.getMessage().c_str();
                    logger->log("Warning: guichan input exception: %s", err);
                }
                return;
            }

            // Mode switch to emotes
            if (keyboard.isKeyActive(keyboard.KEY_EMOTE))
            {
                // Emotions
                int emotion = keyboard.getKeyEmoteOffset(event.key.keysym.sym);
                if (emotion)
                {
                    emoteShortcut->useEmote(emotion);
                    used = true;
                    return;
                }
            }

            if (!chatWindow->isInputFocused()
                && !gui->getFocusHandler()->getModalFocused())
            {
                if (keyboard.isKeyActive(keyboard.KEY_OK))
                {
                    // Do not focus chat input when quit dialog is active
                    if (quitDialog != NULL && quitDialog->isVisible())
                        continue;
                    // Close the Browser if opened
                    else if (helpWindow->isVisible() &&
                                keyboard.isKeyActive(keyboard.KEY_OK))
                        helpWindow->setVisible(false);
                    // Close the config window, cancelling changes if opened
                    else if (setupWindow->isVisible() &&
                                keyboard.isKeyActive(keyboard.KEY_OK))
                        setupWindow->action(gcn::ActionEvent(NULL, "cancel"));
                    else if (npcDialog->isVisible() &&
                                keyboard.isKeyActive(keyboard.KEY_OK))
                        npcDialog->action(gcn::ActionEvent(NULL, "ok"));
                    /*
                    else if (guildWindow->isVisible())
                    {
                        // TODO: Check if a dialog is open and close it if so
                    }
                    */
                }
                if (keyboard.isKeyActive(keyboard.KEY_TOGGLE_CHAT))
                {
                    if (chatWindow->requestChatFocus())
                        used = true;
                }
                if (npcDialog->isVisible())
                {
                    if (keyboard.isKeyActive(keyboard.KEY_MOVE_UP))
                        npcDialog->move(1);
                    else if (keyboard.isKeyActive(keyboard.KEY_MOVE_DOWN))
                        npcDialog->move(-1);
                }
            }


            if (!chatWindow->isInputFocused() || (event.key.keysym.mod & KMOD_ALT))
            {
                if (keyboard.isKeyActive(keyboard.KEY_PREV_CHAT_TAB))
                {
                    chatWindow->prevTab();
                    return;
                }
                else if (keyboard.isKeyActive(keyboard.KEY_NEXT_CHAT_TAB))
                {
                    chatWindow->nextTab();
                    return;
                }
            }

            if ((event.key.keysym.mod & KMOD_RCTRL || event.key.keysym.mod & KMOD_LCTRL)
                && !chatWindow->isInputFocused())
            {
                int outfitNum = -1;
                switch (event.key.keysym.sym)
                {
                    case SDLK_1:
                        outfitNum = 0;
                        break;

                    case SDLK_2:
                        outfitNum = 1;
                        break;

                    case SDLK_3:
                        outfitNum = 2;
                        break;

                    case SDLK_4:
                        outfitNum = 3;
                        break;

                    case SDLK_5:
                        outfitNum = 4;
                        break;

                    case SDLK_6:
                        outfitNum = 5;
                        break;

                    case SDLK_7:
                        outfitNum = 6;
                        break;

                    case SDLK_8:
                        outfitNum = 7;
                        break;

                    case SDLK_9:
                        outfitNum = 8;
                        break;

                    case SDLK_0:
                        outfitNum = 9;
                        break;

                    case SDLK_MINUS:
                        outfitNum = 10;
                        break;

                    case SDLK_EQUALS:
                        outfitNum = 11;
                        break;

                    case SDLK_BACKSPACE:
                        outfitNum = 12;
                        break;

                    case SDLK_INSERT:
                        outfitNum = 13;
                        break;

                    case SDLK_HOME:
                        outfitNum = 14;
                        break;

                    default:
                        break;
                }
                if (outfitNum >= 0)
                {
                    used = true;
                    if (event.key.keysym.mod & KMOD_RCTRL)
                        outfitWindow->wearOutfit(outfitNum);
                    else if (event.key.keysym.mod & KMOD_LCTRL)
                        outfitWindow->copyOutfit(outfitNum);
                }
            }

            const int tKey = keyboard.getKeyIndex(event.key.keysym.sym);
            switch (tKey)
            {
                case KeyboardConfig::KEY_SCROLL_CHAT_UP:
                    if (chatWindow->isVisible())
                    {
                        chatWindow->scroll(-DEFAULT_CHAT_WINDOW_SCROLL);
                        used = true;
                    }
                    break;
                case KeyboardConfig::KEY_SCROLL_CHAT_DOWN:
                    if (chatWindow->isVisible())
                    {
                        chatWindow->scroll(DEFAULT_CHAT_WINDOW_SCROLL);
                        used = true;
                        return;
                    }
                    break;
                case KeyboardConfig::KEY_WINDOW_HELP:
                    // In-game Help
                    if (helpWindow->isVisible())
                        helpWindow->setVisible(false);
                    else
                    {
                        helpWindow->loadHelp("index");
                        helpWindow->requestMoveToTop();
                    }
                    used = true;
                    break;
               // Quitting confirmation dialog
               case KeyboardConfig::KEY_QUIT:
                    if (!quitDialog)
                    {
                        quitDialog = new QuitDialog(&done, &quitDialog);
                        quitDialog->requestMoveToTop();
                    }
                    else
                    {
                        quitDialog->action(gcn::ActionEvent(NULL, "cancel"));
                    }
                    break;
                default:
                    break;
            }
            if (keyboard.isEnabled() &&
                 !chatWindow->isInputFocused() && !npcDialog->isInputFocused())
            {
                const int tKey = keyboard.getKeyIndex(event.key.keysym.sym);

                // Do not activate shortcuts if tradewindow is visible
                if (!tradeWindow->isVisible())
                {
                    // Checks if any item shortcut is pressed.
                    for (int i = KeyboardConfig::KEY_SHORTCUT_1;
                             i <= KeyboardConfig::KEY_SHORTCUT_12;
                             i++)
                    {
                        if (tKey == i && !used)
                        {
                            itemShortcut->useItem(
                                          i - KeyboardConfig::KEY_SHORTCUT_1);
                            break;
                        }
                    }
                }

                switch (tKey)
                {
                    case KeyboardConfig::KEY_PICKUP:
                        {
#ifdef TMWSERV_SUPPORT
                            const Vector &pos = player_node->getPosition();
                            Uint16 x = (int) pos.x / 32;
                            Uint16 y = (int) pos.y / 32;
#else
                            Uint16 x = player_node->getTileX();
                            Uint16 y = player_node->getTileY();
#endif
                            FloorItem *item =
                                floorItemManager->findByCoordinates(x, y);

                            // If none below the player, try the tile in front
                            // of the player
                            if (!item)
                            {
                                // Temporary until tile-based picking is
                                // removed.
                                switch (player_node->getSpriteDirection())
                                {
                                    case DIRECTION_UP   : --y; break;
                                    case DIRECTION_DOWN : ++y; break;
                                    case DIRECTION_LEFT : --x; break;
                                    case DIRECTION_RIGHT: ++x; break;
                                    default: break;
                                }

                                item = floorItemManager->findByCoordinates(
                                        x, y);
                            }

                            if (item)
                                player_node->pickUp(item);

                            used = true;
                        }
                        break;
                    case KeyboardConfig::KEY_SIT:
                        // Player sit action
                        player_node->toggleSit();
                        used = true;
                        break;
                    case KeyboardConfig::KEY_HIDE_WINDOWS:
                        // Hide certain windows
                        if (!chatWindow->isInputFocused())
                        {
                            statusWindow->setVisible(false);
                            inventoryWindow->setVisible(false);
                            skillDialog->setVisible(false);
                            setupWindow->setVisible(false);
                            equipmentWindow->setVisible(false);
                            helpWindow->setVisible(false);
                            debugWindow->setVisible(false);
#ifdef TMWSERV_SUPPORT
                            guildWindow->setVisible(false);
                            buddyWindow->setVisible(false);
#endif
                        }
                        break;
                    case KeyboardConfig::KEY_WINDOW_STATUS:
                        requestedWindow = statusWindow;
                        break;
                    case KeyboardConfig::KEY_WINDOW_INVENTORY:
                        requestedWindow = inventoryWindow;
                        break;
                    case KeyboardConfig::KEY_WINDOW_EQUIPMENT:
                        requestedWindow = equipmentWindow;
                        break;
                    case KeyboardConfig::KEY_WINDOW_SKILL:
                        requestedWindow = skillDialog;
                        break;
                    case KeyboardConfig::KEY_WINDOW_MINIMAP:
                        minimap->toggle();
                        break;
                    case KeyboardConfig::KEY_WINDOW_CHAT:
                        requestedWindow = chatWindow;
                        break;
                    case KeyboardConfig::KEY_WINDOW_SHORTCUT:
                        requestedWindow = itemShortcutWindow;
                        break;
                    case KeyboardConfig::KEY_WINDOW_SETUP:
                        requestedWindow = setupWindow;
                        break;
                    case KeyboardConfig::KEY_WINDOW_DEBUG:
                        requestedWindow = debugWindow;
                        break;
                    case KeyboardConfig::KEY_WINDOW_PARTY:
                        requestedWindow = partyWindow;
                        break;
                    case KeyboardConfig::KEY_WINDOW_EMOTE_SHORTCUT:
                        requestedWindow = emoteShortcutWindow;
                        break;
                    case KeyboardConfig::KEY_WINDOW_OUTFIT:
                        requestedWindow = outfitWindow;
                        break;
                    case KeyboardConfig::KEY_SCREENSHOT:
                        // Screenshot (picture, hence the p)
                        saveScreenshot();
                        used = true;
                        break;
                    case KeyboardConfig::KEY_PATHFIND:
                        // Find path to mouse (debug purpose)
                        viewport->toggleDebugPath();
                        used = true;
                        break;
                    case KeyboardConfig::KEY_TRADE:
                        // Toggle accepting of incoming trade requests
                        unsigned int deflt = player_relations.getDefault();
                        if (deflt & PlayerRelation::TRADE)
                        {
                            localChatTab->chatLog(
                                        _("Ignoring incoming trade requests"),
                                        BY_SERVER);
                            deflt &= ~PlayerRelation::TRADE;
                        }
                        else
                        {
                            localChatTab->chatLog(
                                        _("Accepting incoming trade requests"),
                                        BY_SERVER);
                            deflt |= PlayerRelation::TRADE;
                        }

                        player_relations.setDefault(deflt);

                        used = true;
                        break;
                }
            }

            if (requestedWindow)
            {
                requestedWindow->setVisible(!requestedWindow->isVisible());
                if (requestedWindow->isVisible())
                    requestedWindow->requestMoveToTop();
                used = true;
            }
        }
        // Quit event
        else if (event.type == SDL_QUIT)
        {
            done = true;
        }

        // Push input to GUI when not used
        if (!used)
        {
            try
            {
                guiInput->pushInput(event);
            }
            catch (gcn::Exception e)
            {
                const char* err = e.getMessage().c_str();
                logger->log("Warning: guichan input exception: %s", err);
            }
        }

    } // End while

    // If the user is configuring the keys then don't respond.
    if (!keyboard.isEnabled())
        return;

    // Moving player around
    if (player_node->mAction != Being::DEAD && current_npc == 0 &&
        !chatWindow->isInputFocused())
    {
        // Get the state of the keyboard keys
        keyboard.refreshActiveKeys();

        // Ignore input if either "ignore" key is pressed
        // Stops the character moving about if the user's window manager
        // uses "ignore+arrow key" to switch virtual desktops.
        if (keyboard.isKeyActive(keyboard.KEY_IGNORE_INPUT_1) ||
            keyboard.isKeyActive(keyboard.KEY_IGNORE_INPUT_2))
        {
            return;
        }

        const Vector &pos = player_node->getPosition();
        const Uint16 x = (int) pos.x / 32;
        const Uint16 y = (int) pos.y / 32;
        unsigned char direction = 0;

        // Translate pressed keys to movement and direction
        if (keyboard.isKeyActive(keyboard.KEY_MOVE_UP) ||
            (joystick && joystick->isUp()))
        {
            direction |= Being::UP;
        }
        else if (keyboard.isKeyActive(keyboard.KEY_MOVE_DOWN) ||
                 (joystick && joystick->isDown()))
        {
            direction |= Being::DOWN;
        }

        if (keyboard.isKeyActive(keyboard.KEY_MOVE_LEFT) ||
            (joystick && joystick->isLeft()))
        {
            direction |= Being::LEFT;
        }
        else if (keyboard.isKeyActive(keyboard.KEY_MOVE_RIGHT) ||
                 (joystick && joystick->isRight()))
        {
            direction |= Being::RIGHT;
        }

        player_node->setWalkingDir(direction);

        // Attacking monsters
        if (keyboard.isKeyActive(keyboard.KEY_ATTACK) ||
           (joystick && joystick->buttonPressed(0)))
        {
            if (player_node->getTarget())
                player_node->attack(player_node->getTarget(), true);
        }

        if (keyboard.isKeyActive(keyboard.KEY_TARGET_ATTACK))
        {
            Being *target = NULL;

            bool newTarget = !keyboard.isKeyActive(keyboard.KEY_TARGET);
            // A set target has highest priority
            if (!player_node->getTarget())
            {
#ifdef TMWSERV_SUPPORT
                Uint16 targetX = x / 32, targetY = y / 32;
#else
                Uint16 targetX = x, targetY = y;
#endif
                // Only auto target Monsters
                target = beingManager->findNearestLivingBeing(targetX, targetY,
                         20, Being::MONSTER);
            }
            player_node->attack(target, newTarget);
        }

        // Target the nearest player/monster/npc
        if ((keyboard.isKeyActive(keyboard.KEY_TARGET_PLAYER) ||
            keyboard.isKeyActive(keyboard.KEY_TARGET_CLOSEST) ||
            keyboard.isKeyActive(keyboard.KEY_TARGET_NPC) ||
                    (joystick && joystick->buttonPressed(3))) &&
                !keyboard.isKeyActive(keyboard.KEY_TARGET))
        {
            Being::Type currentTarget = Being::UNKNOWN;
            if (keyboard.isKeyActive(keyboard.KEY_TARGET_CLOSEST) ||
                    (joystick && joystick->buttonPressed(3)))
                currentTarget = Being::MONSTER;
            else if (keyboard.isKeyActive(keyboard.KEY_TARGET_PLAYER))
                currentTarget = Being::PLAYER;
            else if (keyboard.isKeyActive(keyboard.KEY_TARGET_NPC))
                currentTarget = Being::NPC;

            Being *target = beingManager->findNearestLivingBeing(player_node,
                                                    20, currentTarget);

            if (target && (target != player_node->getTarget() ||
                    currentTarget != mLastTarget))
            {
                player_node->setTarget(target);
                mLastTarget = currentTarget;
            }
        } else mLastTarget = Being::UNKNOWN; // Reset last target

        // Talk to the nearest NPC if 't' pressed
        if ( keyboard.isKeyActive(keyboard.KEY_TALK) )
        {
            if (!npcDialog->isVisible())
            {
                Being *target = player_node->getTarget();

                if (target)
                {
                    if (target->getType() == Being::NPC)
                        dynamic_cast<NPC*>(target)->talk();
                }
            }
        }

        // Stop attacking if the right key is pressed
        if (!keyboard.isKeyActive(keyboard.KEY_ATTACK)
            && keyboard.isKeyActive(keyboard.KEY_TARGET))
        {
            player_node->stopAttack();
        }

        if (joystick)
        {
            if (joystick->buttonPressed(1))
            {
                FloorItem *item = floorItemManager->findByCoordinates(x, y);

                if (item)
                    player_node->pickUp(item);
            }
            else if (joystick->buttonPressed(2))
            {
                player_node->toggleSit();
            }
        }
    }
}
