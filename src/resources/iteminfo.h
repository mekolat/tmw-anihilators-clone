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

#ifndef _TMW_ITEMINFO_H_
#define _TMW_ITEMINFO_H_

#include <string>

class Image;

/**
 * Defines a class for storing item infos.
 */
class ItemInfo
{
    public:
        /**
         * Constructor.
         */
        ItemInfo():
            mImageName(""),
            mImage(NULL),
            mArt(0),
            mType(0),
            mWeight(0),
            mSlot(0)
        {
        }

        /**
         * Destructor.
         */
        ~ItemInfo();

        void
        setArt(short art) { mArt = art; }

        short
        getArt() const { return mArt; }

        void
        setName(const std::string &name) { mName = name; }

        const std::string&
        getName() const { return mName; }

        void
        setImage(const std::string &image);

        Image*
        getImage() const { return mImage; }

        void
        setDescription(const std::string &description)
        {
            mDescription = description;
        }

        const std::string&
        getDescription() const { return mDescription; }

        void
        setEffect(const std::string &effect) { mEffect = effect; }

        const std::string&
        getEffect() const { return mEffect; }

        void
        setType(short type) { mType = type; }

        short
        getType() const { return mType; }

        void
        setWeight(short weight) { mWeight = weight; }

        short
        getWeight() const { return mWeight; }

        void
        setSlot(char slot) { mSlot = slot; }

        char
        getSlot() const { return mSlot; }

    protected:
        std::string mImageName;

        /* TODO (BL): I do not think the item info should keep a reference to
         * the item icon. It would probably be better if this was kept in the
         * Item class, so that the images can be lazily instantiated and also
         * unloaded when no longer used.
         */
        Image *mImage;
        short mArt;
        std::string mName;
        std::string mDescription;
        std::string mEffect;
        short mType;
        short mWeight;
        char mSlot;
};

#endif
