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

#include "mapreader.h"

#include <cassert>
#include <iostream>
#include <zlib.h>

#include "resourcemanager.h"
#include "image.h"

#include "../log.h"
#include "../map.h"
#include "../tileset.h"

#include "../utils/base64.h"
#include "../utils/tostring.h"
#include "../utils/xml.h"

const unsigned int DEFAULT_TILE_WIDTH = 32;
const unsigned int DEFAULT_TILE_HEIGHT = 32;

/**
 * Inflates either zlib or gzip deflated memory. The inflated memory is
 * expected to be freed by the caller.
 */
int
inflateMemory(unsigned char *in, unsigned int inLength,
              unsigned char *&out, unsigned int &outLength)
{
    int bufferSize = 256 * 1024;
    int ret;
    z_stream strm;

    out = (unsigned char*) malloc(bufferSize);

    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    strm.next_in = in;
    strm.avail_in = inLength;
    strm.next_out = out;
    strm.avail_out = bufferSize;

    ret = inflateInit2(&strm, 15 + 32);

    if (ret != Z_OK)
        return ret;

    do
    {
        if (strm.next_out == NULL)
        {
            inflateEnd(&strm);
            return Z_MEM_ERROR;
        }

        ret = inflate(&strm, Z_NO_FLUSH);
        assert(ret != Z_STREAM_ERROR);

        switch (ret) {
            case Z_NEED_DICT:
                ret = Z_DATA_ERROR;
            case Z_DATA_ERROR:
            case Z_MEM_ERROR:
                (void) inflateEnd(&strm);
                return ret;
        }

        if (ret != Z_STREAM_END)
        {
            out = (unsigned char*) realloc(out, bufferSize * 2);

            if (out == NULL)
            {
                inflateEnd(&strm);
                return Z_MEM_ERROR;
            }

            strm.next_out = out + bufferSize;
            strm.avail_out = bufferSize;
            bufferSize *= 2;
        }
    }
    while (ret != Z_STREAM_END);
    assert(strm.avail_in == 0);

    outLength = bufferSize - strm.avail_out;
    (void) inflateEnd(&strm);
    return ret == Z_STREAM_END ? Z_OK : Z_DATA_ERROR;
}

int
inflateMemory(unsigned char *in, unsigned int inLength,
              unsigned char *&out)
{
    unsigned int outLength = 0;
    int ret = inflateMemory(in, inLength, out, outLength);

    if (ret != Z_OK || out == NULL)
    {
        if (ret == Z_MEM_ERROR)
        {
            logger->log("Error: Out of memory while decompressing map data!");
        }
        else if (ret == Z_VERSION_ERROR)
        {
            logger->log("Error: Incompatible zlib version!");
        }
        else if (ret == Z_DATA_ERROR)
        {
            logger->log("Error: Incorrect zlib compressed data!");
        }
        else
        {
            logger->log("Error: Unknown error while decompressing map data!");
        }

        free(out);
        out = NULL;
        outLength = 0;
    }

    return outLength;
}

Map*
MapReader::readMap(const std::string &filename)
{
    // Load the file through resource manager
    ResourceManager *resman = ResourceManager::getInstance();
    int fileSize;
    void *buffer = resman->loadFile(filename, fileSize);
    Map *map = NULL;

    if (buffer == NULL)
    {
        logger->log("Map file not found (%s)", filename.c_str());
        return NULL;
    }

    unsigned char *inflated;
    unsigned int inflatedSize;

    if (filename.find(".gz", filename.length() - 3) != std::string::npos)
    {
        // Inflate the gzipped map data
        inflatedSize =
            inflateMemory((unsigned char*) buffer, fileSize, inflated);
        free(buffer);

        if (inflated == NULL)
        {
            logger->log("Could not decompress map file (%s)",
                    filename.c_str());
            return NULL;
        }
    }
    else
    {
        inflated = (unsigned char*) buffer;
        inflatedSize = fileSize;
    }

    XML::Document doc((char*) inflated, inflatedSize);
    free(inflated);

    xmlNodePtr node = doc.rootNode();

    // Parse the inflated map data
    if (node) {
        if (!xmlStrEqual(node->name, BAD_CAST "map")) {
            logger->log("Error: Not a map file (%s)!", filename.c_str());
        }
        else {
            map = readMap(node, filename);
        }
    } else {
        logger->log("Error while parsing map file (%s)!", filename.c_str());
    }

    return map;
}

Map*
MapReader::readMap(xmlNodePtr node, const std::string &path)
{
    xmlChar *prop;

    // Take the filename off the path
    std::string pathDir = path.substr(0, path.rfind("/") + 1);

    prop = xmlGetProp(node, BAD_CAST "version");
    xmlFree(prop);

    int w = XML::getProperty(node, "width", 0);
    int h = XML::getProperty(node, "height", 0);
    int tilew = XML::getProperty(node, "tilewidth", DEFAULT_TILE_WIDTH);
    int tileh = XML::getProperty(node, "tileheight", DEFAULT_TILE_HEIGHT);
    int layerNr = 0;
    Map *map = new Map(w, h, tilew, tileh);

    for_each_xml_child_node(childNode, node)
    {
        if (xmlStrEqual(childNode->name, BAD_CAST "tileset"))
        {
            Tileset *tileset = readTileset(childNode, pathDir, map);
            if (tileset) {
                map->addTileset(tileset);
            }
        }
        else if (xmlStrEqual(childNode->name, BAD_CAST "layer"))
        {
            logger->log("- Loading layer %d", layerNr);
            readLayer(childNode, map, layerNr);
            layerNr++;
        }
        else if (xmlStrEqual(childNode->name, BAD_CAST "properties"))
        {
            readProperties(childNode, map);
        }
        else if (xmlStrEqual(childNode->name, BAD_CAST "objectgroup"))
        {
            for_each_xml_child_node(objectNode, childNode)
            {
                if (xmlStrEqual(objectNode->name, BAD_CAST "object"))
                {
                    std::string objType = XML::getProperty(objectNode, "type", "");

                    if (objType == "WARP" || objType == "NPC" ||
                        objType == "SCRIPT" || objType == "SPAWN")
                    {
                        // Silently skip server-side objects.
                        continue;
                    }

                    std::string objName = XML::getProperty(objectNode, "name", "");
                    int objX = XML::getProperty(objectNode, "x", 0);
                    int objY = XML::getProperty(objectNode, "y", 0);

                    logger->log("- Loading object name: %s type: %s at %d:%d",
                                objName.c_str(), objType.c_str(), objX, objY);
                    if (objType == "PARTICLE_EFFECT")
                    {
                        map->addParticleEffect(objName, objX, objY);
                    }
                    else
                    {
                        logger->log("   Warning: Unknown object type");
                    }
                }
            }
        }
    }

    map->initializeOverlays();

    return map;
}

void
MapReader::readProperties(xmlNodePtr node, Properties* props)
{
    for_each_xml_child_node(childNode, node)
    {
        if (!xmlStrEqual(childNode->name, BAD_CAST "property"))
            continue;

        // Example: <property name="name" value="value"/>
        xmlChar *name = xmlGetProp(childNode, BAD_CAST "name");
        xmlChar *value = xmlGetProp(childNode, BAD_CAST "value");

        if (name && value) {
            props->setProperty((const char*)name, (const char*)value);
        }

        if (name) xmlFree(name);
        if (value) xmlFree(value);
    }
}

void
MapReader::readLayer(xmlNodePtr node, Map *map, int layer)
{
    int h = map->getHeight();
    int w = map->getWidth();
    int x = 0;
    int y = 0;

    // Load the tile data. Layers are assumed to be map size, with (0,0) as
    // origin.
    for_each_xml_child_node(childNode, node)
    {
        if (!xmlStrEqual(childNode->name, BAD_CAST "data"))
            continue;

        xmlChar *encoding = xmlGetProp(childNode, BAD_CAST "encoding");
        xmlChar *compression = xmlGetProp(childNode, BAD_CAST "compression");

        if (encoding && xmlStrEqual(encoding, BAD_CAST "base64"))
        {
            xmlFree(encoding);

            if (compression && !xmlStrEqual(compression, BAD_CAST "gzip")) {
                logger->log("Warning: only gzip layer compression supported!");
                xmlFree(compression);
                return;
            }

            // Read base64 encoded map file
            xmlNodePtr dataChild = childNode->xmlChildrenNode;
            if (!dataChild)
                continue;

            int len = strlen((const char*)dataChild->content) + 1;
            unsigned char *charData = new unsigned char[len + 1];
            const char *charStart = (const char*)dataChild->content;
            unsigned char *charIndex = charData;

            while (*charStart) {
                if (*charStart != ' ' && *charStart != '\t' &&
                        *charStart != '\n')
                {
                    *charIndex = *charStart;
                    charIndex++;
                }
                charStart++;
            }
            *charIndex = '\0';

            int binLen;
            unsigned char *binData =
                php3_base64_decode(charData, strlen((char*)charData), &binLen);

            delete[] charData;

            if (binData) {
                if (compression) {
                    if (xmlStrEqual(compression, BAD_CAST "gzip")) {
                        // Inflate the gzipped layer data
                        unsigned char *inflated;
                        unsigned int inflatedSize =
                            inflateMemory(binData, binLen, inflated);

                        free(binData);
                        binData = inflated;
                        binLen = inflatedSize;

                        if (inflated == NULL)
                        {
                            logger->log("Error: Could not decompress layer!");
                            xmlFree(compression);
                            return;
                        }
                    }
                    xmlFree(compression);
                }

                for (int i = 0; i < binLen - 3; i += 4) {
                    int gid = binData[i] |
                        binData[i + 1] << 8 |
                        binData[i + 2] << 16 |
                        binData[i + 3] << 24;

                    map->setTileWithGid(x, y, layer, gid);

                    x++;
                    if (x == w) {x = 0; y++;}
                }
                free(binData);
            }
        }
        else {
            // Read plain XML map file
            for_each_xml_child_node(childNode2, childNode)
            {
                if (!xmlStrEqual(childNode2->name, BAD_CAST "tile"))
                    continue;

                int gid = XML::getProperty(childNode2, "gid", -1);
                map->setTileWithGid(x, y, layer, gid);

                x++;
                if (x == w) {
                    x = 0; y++;
                    if (y >= h)
                        break;
                }
            }
        }

        if (y < h)
            std::cerr << "TOO SMALL!\n";
        if (x)
            std::cerr << "TOO SMALL!\n";

        // There can be only one data element
        break;
    }
}

Tileset*
MapReader::readTileset(xmlNodePtr node,
                       const std::string &path,
                       Map *map)
{
    if (xmlHasProp(node, BAD_CAST "source"))
    {
        logger->log("Warning: External tilesets not supported yet.");
        return NULL;
    }

    int firstGid = XML::getProperty(node, "firstgid", 0);
    int tw = XML::getProperty(node, "tilewidth", map->getTileWidth());
    int th = XML::getProperty(node, "tileheight", map->getTileHeight());

    for_each_xml_child_node(childNode, node)
    {
        if (!xmlStrEqual(childNode->name, BAD_CAST "image"))
            continue;

        xmlChar* source = xmlGetProp(childNode, BAD_CAST "source");

        if (source)
        {
            std::string sourceStr = std::string((const char*)source);
            sourceStr.erase(0, 3);  // Remove "../"

            ResourceManager *resman = ResourceManager::getInstance();
            Image* tilebmp = resman->getImage(sourceStr);

            if (tilebmp)
            {
                Tileset *set = new Tileset(tilebmp, tw, th, firstGid);
                tilebmp->decRef();
                xmlFree(source);
                return set;
            }
            else {
                logger->log("Warning: Failed to load tileset (%s)", source);
            }
        }

        break;
    }

    return NULL;
}
