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

#include "updatewindow.h"
#include "ok_dialog.h"
#include "gui.h"
#include "../main.h"
#include "../log.h"
#include "../resources/resourcemanager.h"
#include <curl/curl.h>
#include <sstream>
#include <iostream>
#include <cstdio>
#include <SDL_thread.h>

UpdaterWindow *updaterWindow;
SDL_Thread *thread;
std::string updateHost = "themanaworld.org/files";
std::string currentFile = "news.txt";
bool downloadComplete = true;
int downloadStatus = UPDATE_NEWS;

UpdaterWindow::UpdaterWindow()
    : Window("Updating...")
{
    int h = 300;
    int w = 320;
    setContentSize(w, h);
    
    browserBox = new BrowserBox();
    browserBox->setOpaque(false);
    scrollArea = new ScrollArea(browserBox);
    scrollArea->setDimension(gcn::Rectangle(5, 5, 310, 190));
    label = new gcn::Label("Connecting...");
    label->setPosition(5,205);
    progressBar = new ProgressBar(0.0, 5, 225, w - 10, 40, 37, 70, 23);
    cancelButton = new Button("Cancel");
    cancelButton->setPosition(5, h - 5 - cancelButton->getHeight());
    cancelButton->setEventId("cancel");
    cancelButton->addActionListener(this);
    playButton = new Button("Play");
    playButton->setPosition(cancelButton->getX() + cancelButton->getWidth() + 5,
                            h - 5 - playButton->getHeight());
    playButton->setEventId("play");
    playButton->setEnabled(false);
    playButton->addActionListener(this);
    
    add(scrollArea);
    add(label);
    add(progressBar);
    add(cancelButton);
    add(playButton);
    
    cancelButton->requestFocus();
    setLocationRelativeTo(getParent());
}

UpdaterWindow::~UpdaterWindow()
{
    delete label;
    delete progressBar;
    delete cancelButton;
    delete playButton;
}

void UpdaterWindow::setProgress(float p)
{
    progressBar->setProgress(p);
}

void UpdaterWindow::setLabel(const std::string &str)
{
    label->setCaption(str);
    label->adjustSize();
}

void UpdaterWindow::enable()
{
    playButton->setEnabled(true);
    playButton->requestFocus();
}

void UpdaterWindow::draw(gcn::Graphics *graphics)
{
    Window::draw(graphics);
}

void UpdaterWindow::action(const std::string& eventId)
{
    if (eventId == "cancel") {
        // Skip the updating process
        if (downloadStatus == UPDATE_COMPLETE)
        {
            state = EXIT;
        }
        else {
            downloadStatus = UPDATE_ERROR;
        }
    }
    else if (eventId == "play") {
        state = LOGIN;
    }
    else if (eventId == "ok") {
        state = LOGIN;
    }
}

void UpdaterWindow::loadNews()
{
    browserBox->clearRows();
    ResourceManager *resman = ResourceManager::getInstance();
    int contentsLength;
    std::ifstream newsFile(TMW_DATADIR "data/news.txt");
    if (!newsFile.is_open())
    {
        logger->log("Couldn't load news.txt");
        browserBox->addRow("Error");
        return;
    }

    // Tokenize and add each line separately
    std::string line("");
    while (!newsFile.eof())
    {
        getline(newsFile, line);
        browserBox->addRow(line);
    }
    newsFile.close();
    scrollArea->setVerticalScrollAmount(0);
    setVisible(true);
}

int updateProgress(void *ptr,
                      double t, /* dltotal */
                      double d, /* dlnow */
                      double ultotal,
                      double ulnow)
{
    float progress = d/t;
    if (progress < 0)
    {
        progress = 0.0f;
    }
    std::stringstream progressString;
    progressString << currentFile << " (" << ((int)(progress*100)) << "%)";
    updaterWindow->setLabel(progressString.str().c_str());
    updaterWindow->setProgress(progress);

    if (state != UPDATE || downloadStatus == UPDATE_ERROR) {
        // If the action was canceled return an error code to stop the thread
        return -1;
    }

    return 0;
}

int downloadThread(void *ptr)
{
    CURL *curl;
    CURLcode res;
    FILE *outfile;
    std::string url(updateHost + "/" + currentFile);

    curl = curl_easy_init();
    if (curl)
    {
        // Download current file as a temp file
        logger->log("Downloading: %s", url.c_str());
        // TODO: download in the proper folder (data?)
        outfile = fopen(TMW_DATADIR "data/download.temp", "wb");
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, outfile);
        curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0);
        curl_easy_setopt(curl, CURLOPT_PROGRESSFUNCTION, updateProgress);
        curl_easy_setopt(curl, CURLOPT_PROGRESSDATA, NULL);

        res = curl_easy_perform(curl);

        fclose(outfile);
        curl_easy_cleanup(curl);
        downloadComplete = true;
        if (res != 0) {
            downloadStatus = UPDATE_ERROR;
        }
        else {
            // If the download was successful give the file the proper name
            // else it will be deleted later
            std::string newName(TMW_DATADIR "data/");
            newName += currentFile.c_str();
            rename(TMW_DATADIR "data/download.temp", newName.c_str());
        }
    }

    return 0;
}

void download()
{
    downloadComplete = false;
    thread = SDL_CreateThread(downloadThread, NULL);

    if (thread == NULL) {
        logger->log("Unable to create thread");
        downloadStatus = UPDATE_ERROR;
    }
    else {
        logger->log("Starting download of %s", currentFile.c_str());
    }
}

void checkFile(std::ifstream &in) {
    // Check for XML tag (if it is XML tag it is error)
    // WARNING: this way we can't use an XML file for resources listing
    if (!in.eof())
    {
        std::string line("");
        getline(in, line);
        if (line[0] == '<') {
            logger->log("Error: resources.txt download error (404)");
            downloadStatus = UPDATE_ERROR;
        }
        else {
            // Move the pointer to the beginning of the file
            in.seekg (0, std::ios::beg);
        }
    }
}

void updateData()
{
    updaterWindow = new UpdaterWindow();
    state = UPDATE;
    
    std::string updateHost =
            config.getValue("updatehost", "themanaworld.org/files");
    // Try to download the updates list
    download();
    std::ifstream in;

    while (state == UPDATE)
    {
        // Handle SDL events
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_QUIT:
                    state = EXIT;
                    break;

                case SDL_KEYDOWN:
                    if (event.key.keysym.sym == SDLK_ESCAPE)
                    {
                        state = EXIT;
                    }
                    break;
            }
            
            guiInput->pushInput(event);
        }
        
        switch (downloadStatus) {
            case UPDATE_ERROR:
                SDL_WaitThread(thread, NULL);
                new OkDialog(
                    "Error", ("The update process is incomplete. ",
                    "It is strongly recommended that you try again later"),
                    updaterWindow);
                logger->log("Error during the update process");
                downloadStatus = UPDATE_IDLE;
                break;
            case UPDATE_NEWS:
                // If not already downloading another file
                if (downloadComplete) {
                    // Try to open news.txt
                    updaterWindow->loadNews();
                    // Doesn't matter if it couldn't find news.txt,
                    // go to the next step
                    currentFile = "resources.txt";
                    download();
                    downloadStatus = UPDATE_RESOURCES;
                }
                break;
            case UPDATE_RESOURCES:
                // If not already downloading another file
                if (downloadComplete) {
                    // Check if the list was already accessed
                    if (!in.is_open())
                    {
                        // Try to open resources.txt
                        in.open(TMW_DATADIR "data/resources.txt");
                        if (!in.is_open())
                        {
                            logger->log("Unable to open resources.txt");
                            downloadStatus = UPDATE_ERROR;
                        }
                        else {
                            checkFile(in);
                        }
                    }
                    else {
                        SDL_WaitThread(thread, NULL);
                        if (!in.eof())
                        {
                            // Download each update
                            std::string line("");
                            getline(in, line);
                            // TODO: it should check if file already exists
                            currentFile = line;
                            download();
                        }
                        else {
                            // Download of updates completed
                            downloadStatus = UPDATE_COMPLETE;
                        }
                    }
                }
                break;
            case UPDATE_COMPLETE:
                updaterWindow->enable();
                updaterWindow->setLabel("Completed");
                break;
            case UPDATE_IDLE:
                break;
        }

        gui->logic();

        login_wallpaper->draw(screen, 0, 0);
        gui->draw();
        guiGraphics->updateScreen();
    }
    
    in.close();
    // Remove downloaded files
    remove(TMW_DATADIR "data/news.txt");
    remove(TMW_DATADIR "data/resources.txt");
    remove(TMW_DATADIR "data/download.temp");

    delete updaterWindow;
}
