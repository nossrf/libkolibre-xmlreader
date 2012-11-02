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

#ifdef HAVE_CONFIG_H
#include "config.h"
#ifdef HAVE_LIBTIDY
#include "DataStreamHandler.h"
#include "CacheObject.h"
#include "TidyStream.h"

#include <log4cxx/logger.h>

// create logger which will become a child to logger kolibre.xmlreader
log4cxx::LoggerPtr xmlTidyStreamLog(
        log4cxx::Logger::getLogger("kolibre.xmlreader.tidystream"));

using namespace std;

TidyStream::TidyStream(const string url, InputStream *in) :
        sURL(url), inStream(in), fTotalBytesRead(0), cacheObject(0),
            bUseCache(true), bTidied(false)
{
    mErrorMsg = "unknown error";
    mErrorCode = NONE;

    LOG4CXX_DEBUG(xmlTidyStreamLog, "constructor for '" << sURL << "'");

    // Make sure not to tidy .smil files
    if (sURL.rfind(".smil") == sURL.length() - 5)
        mode = PASSTROUGH;
    else
        mode = TIDY;
}

TidyStream::~TidyStream()
{
    LOG4CXX_DEBUG(xmlTidyStreamLog, "destructor for '" << sURL << "'");

    switch (mode)
    {
    case TIDY:
        tidyBufFree(&outbuf);
        break;
    case CACHED:
        if (cacheObject != NULL)
            cacheObject->resetState();
        break;
    case PASSTROUGH:
        delete inStream;
        break;
    }
}

void TidyStream::useCache(bool setting)
{
    bUseCache = setting;
}

int TidyStream::readBytes(char* const toFill, const unsigned int maxToRead)
{
    // Check if the instream has been tidied already, return -1 on error
    if ((mode == TIDY) && !bTidied)
        if (!Perform())
            return -1;

    int fBytes = 0;
    switch (mode)
    {
    case TIDY:
    {
        unsigned long fBytesLeft = outbuf.size - fTotalBytesRead;

        if (fBytesLeft >= maxToRead)
            fBytes = maxToRead;
        else
            fBytes = fBytesLeft;

        LOG4CXX_DEBUG(xmlTidyStreamLog,
                "Read " << fBytes << " bytes from outbuf");
        memcpy(toFill, outbuf.bp + fTotalBytesRead, fBytes);

        fTotalBytesRead += fBytes;
        break;
    }

    case CACHED:
        fBytes = cacheObject->readBytes((char *) toFill, maxToRead);
        // On error propagate
        if (fBytes < 0)
        {
            mErrorMsg = inStream->getErrorMsg();
            mErrorCode = inStream->getErrorCode();
        }
        else
            fTotalBytesRead += fBytes;

        LOG4CXX_DEBUG(xmlTidyStreamLog,
                "Read " << fBytes << " bytes from cacheObject");
        break;

    case PASSTROUGH:
        fBytes = inStream->readBytes(toFill, maxToRead);

        // On error propagate
        if (fBytes < 0)
        {
            mErrorMsg = inStream->getErrorMsg();
            mErrorCode = inStream->getErrorCode();
        }
        else
            fTotalBytesRead += fBytes;

        LOG4CXX_DEBUG(xmlTidyStreamLog,
                "Read " << fBytes << " bytes from inStream");
        break;
    }

    return fBytes;
}

unsigned int TidyStream::curPos() const
{
    return fTotalBytesRead;
}

bool TidyStream::Perform()
{
    if (mode == TIDY)
    {
        TidyBuffer errbuf;
        TidyBuffer docbuf;

        // Clear the buffers
        tidyBufInit(&errbuf);
        tidyBufInit(&docbuf);
        tidyBufInit(&outbuf);

        // Create a tidydoc structure
        tdoc = tidyCreate();

        // Set the options
        tidyOptSetBool(tdoc, TidyForceOutput, yes); /* try harder */
        tidyOptSetBool(tdoc, TidyXhtmlOut, yes);
        tidyOptSetBool(tdoc, TidyMakeClean, yes);
        tidyOptSetBool(tdoc, TidyMakeBare, yes);
        tidyOptSetBool(tdoc, TidyQuiet, yes);
        tidyOptSetInt(tdoc, TidyWrapLen, 0);

        tidySetCharEncoding(tdoc, "raw");

        tidySetErrorBuffer(tdoc, &errbuf);

        char buf[4096];
        int bytesRead = 0;

        do
        {
            bytesRead = inStream->readBytes(buf, 4096);
            if (bytesRead < 0)
            {
                LOG4CXX_WARN(xmlTidyStreamLog,
                        "An error occurred in inStream, fowarding");
                mErrorMsg = inStream->getErrorMsg();
                mErrorCode = inStream->getErrorCode();
                delete inStream;
                return false;
            }

            LOG4CXX_DEBUG(xmlTidyStreamLog,
                    "copying " << bytesRead << " bytes to tidybuf");
            tidyBufAppend(&docbuf, buf, bytesRead);

        } while (bytesRead != 0);

        // We are finished with the inStream at this point, delete it
        delete inStream;
        inStream = NULL;

        LOG4CXX_DEBUG(xmlTidyStreamLog, "Done reading from inStream");

        // Get the cacheObject if it exists
        cacheObject = DataStreamHandler::Instance()->getCacheObject(sURL);
        if (cacheObject != NULL)
            cacheObject->resetState();

        if (cacheObject != NULL && cacheObject->getTidyFlag())
        {
            LOG4CXX_DEBUG(xmlTidyStreamLog,
                    "using already tidied copy in cache");
            mode = CACHED;
        }
        else
        {
            // If the object did not exist in cache, create a new one
            if (cacheObject == NULL)
            {
                LOG4CXX_DEBUG(xmlTidyStreamLog, "creating new cacheObject");
                cacheObject = new CacheObject(sURL.c_str());
            }

            int err = 0;

            LOG4CXX_DEBUG(xmlTidyStreamLog, "parsing buffer");
            err = tidyParseBuffer(tdoc, &docbuf); /* parse the input */
            if (err >= 0)
            {
                LOG4CXX_DEBUG(xmlTidyStreamLog, "tidyCleanAndRepair");
                err = tidyCleanAndRepair(tdoc); /* fix any problems */
                /*
                 if ( err >= 0 ) {
                 LOG4CXX_DEBUG(xmlTidyStreamLog, "tidyRunDiagnostics");
                 err = tidyRunDiagnostics(tdoc); // load tidy error buffer
                 if ( err >= 0 ) {
                 //dumpNode( tdoc, tidyGetRoot(tdoc), 0 ); // walk the tree
                 fprintf(Gstderr, "%s\n", errbuf.bp); // show errors
                 }
                 }
                 */
            }

            LOG4CXX_DEBUG(xmlTidyStreamLog, "tidySaveBuffer");
            tidySaveBuffer(tdoc, &outbuf);

            LOG4CXX_DEBUG(xmlTidyStreamLog, "tidySaveBuffer to cache");
            if (cacheObject->writeBytes((const char*) outbuf.bp,
                    outbuf.size) == outbuf.size)
            {
                cacheObject->writeBytes(NULL, 0);
                cacheObject->resetState();
                cacheObject->setTidyFlag(true);
                if (bUseCache)
                    DataStreamHandler::Instance()->addCacheObject(sURL,
                            cacheObject);
            }
            else
            {
                LOG4CXX_ERROR(xmlTidyStreamLog,
                        "failed to store " << outbuf.size << " bytes in cacheObject");
            }
        }

        tidyBufFree(&docbuf);
        tidyBufFree(&errbuf);
    }

    bTidied = true;
    return true;
}
#endif
#endif
