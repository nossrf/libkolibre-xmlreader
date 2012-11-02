/*
 * Copyright (C) 2012 Kolibre
 *
 * This file is part of kolibre-xmlreader.
 *
 * Kolibre-xmlreader is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 2.1 of the License, or
 * (at your option) any later version.
 *
 * Kolibre-xmlreader is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with kolibre-xmlreader. If not, see <http://www.gnu.org/licenses/>.
 */

#include "CacheObject.h"

#include <cstring>
#include <cstdlib>
#include <log4cxx/logger.h>

// create logger which will become a child to logger kolibre.xmlreader
log4cxx::LoggerPtr xmlCacheObjLog(
        log4cxx::Logger::getLogger("kolibre.xmlreader.cacheobject"));

#define DEBUG_ZLIB 1

using namespace std;

const char CacheObject::dictionary[] =
        "\"http://wwwSMILorg/TR/REC-smil/SMIL10<smil>smil</head><body>\"-//W3C//DTDcontent=\"Daisy<?xml version=\"1.0\" encoding=\"iso-8859-1\"?>npt=0<region id=\"txtView\"/>endsync=\"last\"<meta name=\"dc:identifier\" content=mpg\"<meta name=\"ncc:totalElapsedTime\" content=<seq><meta name=\"ncc:generator\" content=</seq><meta name=\"dc:format\" content=<meta<meta name=\"dc:title\" content=booktext<meta name=\"ncc:timeInThisSmil\" content=<ref<layout>endsync=\"last\"></layout></par><!DOCTYPE smil PUBLIC \"-//W3C//DTD SMIL 1.0//EN\" \"http://www.w3.org/TR/REC-smil/SMIL10.dtd\"><par<body><text</body><audio<smil>clip-end=\"</head>clip-begin=\"</smil>smil\"<head>/><seq>mp3\"</seq>src=\"<par endsync=\"last\">id=\"</par>";

CacheObject::CacheObject(const char *url) :
        pSrcUrl(0), iHttpCode(0), eState(EMPTY), bTidyFlag(false), c_stream(0), zBuffer(
                0), zBufferAllocCount(0), zBufferSize(0), zBufferPos(0)
{
    pSrcUrl = strdup(url);
    pEtag = NULL;
    pLast_modified = NULL;
    pLocation = NULL;
    lContent_length = 0;

    LOG4CXX_DEBUG(xmlCacheObjLog, "constructor for " << pSrcUrl);
    resetState();
    resetBuffer();
}

CacheObject::~CacheObject()
{
    LOG4CXX_DEBUG(xmlCacheObjLog, "destructor for " << pSrcUrl);

    resetState();
    resetBuffer();
    if (c_stream != NULL)
    {
        free(c_stream);
        c_stream = NULL;
    }
    if (pSrcUrl != NULL)
    {
        free(pSrcUrl);
        pSrcUrl = NULL;
    }
    if (pLast_modified != NULL)
    {
        free(pLast_modified);
        pLast_modified = NULL;
    }
    if (pEtag != NULL)
    {
        free(pEtag);
        pEtag = NULL;
    }
}

void CacheObject::resetState()
{
    // Depending on the state end the deflate or inflate process
    switch (eState)
    {
    case WRITE:
        LOG4CXX_DEBUG(xmlCacheObjLog, "Resetting state for " << pSrcUrl);
        if (c_stream)
            deflateEnd(c_stream);
        eState = FULL;
        break;
    case READ:
        LOG4CXX_DEBUG(xmlCacheObjLog, "Resetting state for " << pSrcUrl);
        if (c_stream)
            inflateEnd(c_stream);
        eState = FULL;
        break;
    default:
        break;
    }

    // Initialize the z_stream
    if (c_stream != NULL)
        free(c_stream);

    c_stream = (z_stream *) malloc(sizeof(z_stream));

    if (c_stream != NULL)
    {
        memset(c_stream, 0, sizeof(z_stream));
        c_stream->zalloc = Z_NULL;
        c_stream->zfree = Z_NULL;
        c_stream->opaque = Z_NULL;
    }
    else
        LOG4CXX_ERROR(xmlCacheObjLog, "Failed to allocate memory");
}

const std::string &CacheObject::getErrorMsg()
{
    return mErrorMsg;
}

CacheObject::CacheState CacheObject::getState()
{
    switch (eState)
    {
    case EMPTY:
        return EMPTY;
    case FULL:
        return FULL;
    default:
        break;
    }
    return BUSY;
}

void CacheObject::resetBuffer()
{

    // free zBuffer memory
    if (zBuffer != NULL)
    {
        LOG4CXX_DEBUG(xmlCacheObjLog, "Resetting buffers for " << pSrcUrl);
        free(zBuffer);
        zBuffer = NULL;
    }

    zBufferAllocCount = 0;
    zBufferPos = 0;
    zBufferSize = 0;

    eState = EMPTY;
}

CacheObject *CacheObject::getObject()
{
    return this;
}

void CacheObject::setEtag(const char *etag)
{
    if (pEtag != NULL)
        free(pEtag);
    pEtag = strdup(etag);
}

char *CacheObject::getEtag() const
{
    return pEtag;
}

void CacheObject::setLastModified(const char* last_modified)
{
    if (pLast_modified != NULL)
        free(pLast_modified);
    pLast_modified = strdup(last_modified);
}

char *CacheObject::getLastModified() const
{
    return pLast_modified;
}

void CacheObject::setLocation(const char* location)
{
    if (pLocation != NULL)
        free(pLocation);
    pLocation = strdup(location);
}

char *CacheObject::getLocation() const
{
    return pLocation;
}

void CacheObject::setContentLength(const unsigned long content_length)
{
    lContent_length = content_length;
}

unsigned long CacheObject::getContentLength() const
{
    return lContent_length;
}

void CacheObject::setHttpCode(const int httpcode)
{
    iHttpCode = httpcode;
}

int CacheObject::getHttpCode() const
{
    return iHttpCode;
}

unsigned long CacheObject::getBufferSize() const
{
    return zBufferSize;
}

void CacheObject::setTidyFlag(bool flag)
{
    bTidyFlag = flag;
}

bool CacheObject::getTidyFlag()
{
    return bTidyFlag;
}

unsigned int CacheObject::writeBytes(const char *buffer, const size_t bytes)
{

    LOG4CXX_DEBUG(xmlCacheObjLog,
            "writeBytes(const char * " << &buffer << ", const size_t " << bytes << ")");

    if (eState == READ || eState == FULL)
    {
        resetState();
        resetBuffer();
    }

    // Initialize zlib if we haven't done so already
    if (eState == EMPTY)
    {
        LOG4CXX_DEBUG(xmlCacheObjLog,
                "Initializing zlib deflate for " << pSrcUrl);
        deflateInit(c_stream, Z_BEST_SPEED);
        deflateSetDictionary(c_stream, (const Bytef*) dictionary,
                sizeof(dictionary));

        eState = WRITE;
    }

    c_stream->next_in = (Bytef *) buffer;
    c_stream->avail_in = bytes;

    int doFlush = (buffer == NULL && bytes == 0) ? Z_FINISH : Z_NO_FLUSH;

    int err = Z_OK;

    do
    {
        c_stream->next_out = (Bytef *) c_stream_buffer;
        c_stream->avail_out = Z_CHUNK_SIZE;
        LOG4CXX_DEBUG(xmlCacheObjLog,
                "Deflate status before:" << " avail_in: " << c_stream->avail_in << " bytes," << " total_in: " << c_stream->total_in << " bytes," << " avail_out: " << c_stream->avail_out << " bytes," << " total_out: " << c_stream->total_out << " bytes");

        err = deflate(c_stream, doFlush);
        LOG4CXX_DEBUG(xmlCacheObjLog,
                "Deflate status after: " << " avail_in: " << c_stream->avail_in << " bytes," << " total_in: " << c_stream->total_in << " bytes," << " avail_out: " << c_stream->avail_out << " bytes," << " total_out: " << c_stream->total_out << " bytes");

        if (c_stream->msg)
            LOG4CXX_DEBUG(xmlCacheObjLog,
                    "Message from encoder: " << c_stream->msg);

        if (err != Z_OK && err != Z_STREAM_END)
        {

            switch (err)
            {
            case Z_ERRNO:
                LOG4CXX_ERROR(xmlCacheObjLog, "compress error Z_ERRNO");
                return 0;
            case Z_STREAM_ERROR:
                LOG4CXX_ERROR(xmlCacheObjLog, "compress error Z_STREAM_ERROR");
                return 0;
            case Z_DATA_ERROR:
                LOG4CXX_ERROR(xmlCacheObjLog, "compress error Z_DATA_ERROR");
                return 0;
            case Z_MEM_ERROR:
                LOG4CXX_ERROR(xmlCacheObjLog, "compress error Z_MEM_ERROR");
                return 0;
            case Z_VERSION_ERROR:
                LOG4CXX_ERROR(xmlCacheObjLog, "compress error Z_VERSION_ERROR");
                return 0;
                // Non fatal errors
            case Z_BUF_ERROR:
                LOG4CXX_WARN(xmlCacheObjLog, "compress error Z_BUF_ERROR");
                break;
            case Z_STREAM_END:
                LOG4CXX_WARN(xmlCacheObjLog, "compress error Z_STREAM_END");
                break;
            case Z_NEED_DICT:
                LOG4CXX_WARN(xmlCacheObjLog, "compress error Z_NEED_DICT");
                break;
            default:
                LOG4CXX_ERROR(xmlCacheObjLog, "compress error unknown " << err);
                break;
            }
        }
        else
        {
            unsigned compressed = Z_CHUNK_SIZE - c_stream->avail_out;
            while (zBufferSize < c_stream->total_out)
            {
                zBufferAllocCount++;
                zBufferSize = zBufferAllocCount * Z_CHUNK_SIZE;
                LOG4CXX_DEBUG(xmlCacheObjLog,
                        "Reallocating zBuffer to " << zBufferSize << " bytes");

                zBuffer = (char *) realloc(zBuffer, zBufferSize * sizeof(char));
                if (zBuffer == NULL)
                {
                    LOG4CXX_ERROR(xmlCacheObjLog,
                            "Failed to allocate memory for zBuffer");
                    zBufferSize = 0;
                }
            }

            if (compressed != 0 && (zBufferSize - zBufferPos) >= compressed)
            {
                LOG4CXX_DEBUG(xmlCacheObjLog,
                        "Copying " << compressed << " bytes to zBuffer of size " << zBufferSize << " at pos " << zBufferPos << " for " << pSrcUrl);
                memcpy(zBuffer + zBufferPos, c_stream_buffer, compressed);
                zBufferPos += compressed;

            }
            else if (zBufferSize < c_stream->total_out)
            {
                LOG4CXX_ERROR(xmlCacheObjLog,
                        "Not enough memory in zBuffer have: " << zBufferSize << " need: " << c_stream->total_out);
                return 0;
            }
        }
    } while (c_stream->avail_in != 0
            || (err == Z_OK && c_stream->avail_out == 0));
    //If deflate returns Z_OK and with zero avail_out, it must be called again after making room in the output buffer because there might be more output pending.

    if (doFlush == Z_FINISH)
    {
        zBufferSize = zBufferPos;
        // Reallocate zBuffer to correct size
        zBuffer = (char *) realloc(zBuffer, zBufferSize * sizeof(char));
        LOG4CXX_DEBUG(xmlCacheObjLog,
                "Final size of zBuffer: " << zBufferSize << " for " << pSrcUrl);
        eState = FULL;
    }

    return bytes;
}

int CacheObject::readBytes(const char *buffer, const size_t bytes)
{

    LOG4CXX_DEBUG(xmlCacheObjLog,
            "readBytes(const char * " << &buffer << ", const size_t " << bytes << ")");

    // If the cache is empty return 0
    if (eState == EMPTY || zBuffer == NULL)
        return 0;

    if (eState == WRITE)
        resetState();

    if (eState == FULL)
    {
        LOG4CXX_DEBUG(xmlCacheObjLog,
                "Initializing zlib inflate for " << pSrcUrl);
        inflateInit(c_stream);
        inflateSetDictionary(c_stream, (const Bytef*) dictionary,
                sizeof(dictionary));
        eState = READ;

        c_stream->next_in = (Bytef *) zBuffer;
        c_stream->avail_in = zBufferSize;
    }

    int err = Z_OK;
    int doFlush = Z_NO_FLUSH;

    do
    {
        memset((void*) buffer, 0, bytes);
        c_stream->next_out = (Bytef*) buffer;
        c_stream->avail_out = bytes;

        LOG4CXX_DEBUG(xmlCacheObjLog,
                "Inflate status before:" << " avail_in: " << c_stream->avail_in << " bytes," << " avail_out: " << c_stream->avail_out << " bytes");

        err = inflate(c_stream, doFlush);
        LOG4CXX_DEBUG(xmlCacheObjLog,
                "Inflate status after: " << " avail_in: " << c_stream->avail_in << " bytes," << " avail_out: " << c_stream->avail_out << " bytes");

        if (c_stream->msg)
            LOG4CXX_DEBUG(xmlCacheObjLog,
                    "Message from decoder: " << c_stream->msg);

        switch (err)
        {
        case Z_NEED_DICT:
            LOG4CXX_DEBUG(xmlCacheObjLog, "decompress error Z_NEED_DICT");
            inflateSetDictionary(c_stream, (const Bytef*) dictionary,
                    sizeof(dictionary));
            break;

        case Z_ERRNO:
            LOG4CXX_ERROR(xmlCacheObjLog, "decompress error Z_ERRNO");
            return 0;
        case Z_STREAM_ERROR:
            LOG4CXX_ERROR(xmlCacheObjLog, "decompress error Z_STREAM_ERROR");
            return 0;
        case Z_DATA_ERROR:
            LOG4CXX_ERROR(xmlCacheObjLog, "decompress error Z_DATA_ERROR");
            return 0;
        case Z_MEM_ERROR:
            LOG4CXX_ERROR(xmlCacheObjLog, "decompress error Z_MEM_ERROR");
            return 0;
        case Z_BUF_ERROR:
            LOG4CXX_ERROR(xmlCacheObjLog, "decompress error Z_BUF_ERROR");
            break;
        case Z_VERSION_ERROR:
            LOG4CXX_ERROR(xmlCacheObjLog, "decompress error Z_VERSION_ERROR");
            return 0;

        case Z_STREAM_END:
            LOG4CXX_DEBUG(xmlCacheObjLog,
                    "decompress Z_STREAM_END for " << pSrcUrl);
            break;

        case Z_OK:
            break;
        }
    } while (c_stream->avail_out != 0 && c_stream->avail_in != 0);

    LOG4CXX_DEBUG(xmlCacheObjLog,
            "Read " << bytes - c_stream->avail_out << " bytes from zBuffer for " << pSrcUrl);

    return (bytes - c_stream->avail_out);
}

