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

#include "FileStream.h"

#include <sys/stat.h>
#include <sys/types.h>
#include <string.h>
#include <errno.h>
#include <libxml/xmlerror.h>
#include <log4cxx/logger.h>

// create logger which will become a child to logger kolibre.xmlreader
log4cxx::LoggerPtr xmlFileStreamLog(
        log4cxx::Logger::getLogger("kolibre.xmlreader.filestream"));

FileStream::FileStream(const std::string filename, CacheObject *co) :
        cacheObject(0), fTotalBytesRead(0), fSize(0), fp(0)
{
    sFilename = filename;
    LOG4CXX_DEBUG(xmlFileStreamLog, "constructor for " << sFilename);

    if (co != NULL)
    {
        cacheObject = co;
        mode = CACHED;
        bIsOpen = true;
        return;
    }

    // If we do not have a cached object open the file
    mode = READ;
    bIsOpen = false;
}

inline unsigned int FileStream::curPos() const
{
    return fTotalBytesRead;
}

int FileStream::getXmlErrorCode(int error)
{
    switch (error)
    {
#ifndef WIN32
    case EBADMSG: return XML_IO_EBADMSG;
    case ECANCELED: return XML_IO_ECANCELED;
    case EINPROGRESS: return XML_IO_EINPROGRESS;
    case EMSGSIZE: return XML_IO_EMSGSIZE;
    case ENOTSUP: return XML_IO_ENOTSUP;
    case ETIMEDOUT: return XML_IO_ETIMEDOUT;
#endif
    case EACCES:
        return XML_IO_EACCES;
    case EAGAIN:
        return XML_IO_EAGAIN;
    case EBADF:
        return XML_IO_EBADF;
    case EBUSY:
        return XML_IO_EBUSY;
    case ECHILD:
        return XML_IO_ECHILD;
    case EDEADLK:
        return XML_IO_EDEADLK;
    case EDOM:
        return XML_IO_EDOM;
    case EEXIST:
        return XML_IO_EEXIST;
    case EFAULT:
        return XML_IO_EFAULT;
    case EFBIG:
        return XML_IO_EFBIG;
    case EINTR:
        return XML_IO_EINTR;
    case EINVAL:
        return XML_IO_EINVAL;
    case EIO:
        return XML_IO_EIO;
    case EISDIR:
        return XML_IO_EISDIR;
    case EMFILE:
        return XML_IO_EMFILE;
    case EMLINK:
        return XML_IO_EMLINK;
    case ENAMETOOLONG:
        return XML_IO_ENAMETOOLONG;
    case ENFILE:
        return XML_IO_ENFILE;
    case ENODEV:
        return XML_IO_ENODEV;
    case ENOENT:
        return XML_IO_ENOENT;
    case ENOEXEC:
        return XML_IO_ENOEXEC;
    case ENOLCK:
        return XML_IO_ENOLCK;
    case ENOMEM:
        return XML_IO_ENOMEM;
    case ENOSPC:
        return XML_IO_ENOSPC;
    case ENOSYS:
        return XML_IO_ENOSYS;
    case ENOTDIR:
        return XML_IO_ENOTDIR;
    case ENOTEMPTY:
        return XML_IO_ENOTEMPTY;
    case ENOTTY:
        return XML_IO_ENOTTY;
    case ENXIO:
        return XML_IO_ENXIO;
    case EPERM:
        return XML_IO_EPERM;
    case EPIPE:
        return XML_IO_EPIPE;
    case ERANGE:
        return XML_IO_ERANGE;
    case EROFS:
        return XML_IO_EROFS;
    case ESPIPE:
        return XML_IO_ESPIPE;
    case ESRCH:
        return XML_IO_ESRCH;
    case EXDEV:
        return XML_IO_EXDEV;
    }
    return XML_IO_UNKNOWN;
}

FileStream::~FileStream()
{
    LOG4CXX_DEBUG(xmlFileStreamLog, "destructor for " << sFilename);
    LOG4CXX_DEBUG(xmlFileStreamLog, "read " << fTotalBytesRead << "/" << fSize);
    switch (mode)
    {
    case READ:
        if (fp != NULL)
            fclose(fp);
        break;
    case CACHED:
        if (cacheObject != NULL)
            cacheObject->resetState();
        break;
    }
}

void FileStream::useCache(bool setting)
{
    bUseCache = setting;
}

bool FileStream::openStream()
{
    struct stat stat_p;

    if (stat(sFilename.c_str(), &stat_p) != 0)
    {
        mErrorMsg = strerror(errno);
        mErrorCode = ACCESS_DENIED;
        return false;
    }

    // Get the file size
    fSize = stat_p.st_size;

    // Open the file for reading
    fp = fopen(sFilename.c_str(), "r");
    if (fp == NULL)
    {
        mErrorMsg = strerror(errno);
        mErrorCode = READ_FAILED;
        return false;
    }

    bIsOpen = true;
    return bIsOpen;
}

int FileStream::readBytes(char* const toFill, const unsigned int maxToRead)
{
    size_t fBytesRead = 0;

    switch (mode)
    {
    case READ:
        // Check if file is open, if not open it
        if (!bIsOpen)
            if (!openStream())
                return -1;

        LOG4CXX_DEBUG(xmlFileStreamLog, "reading bytes");
        fBytesRead = fread((char *) toFill, 1, maxToRead, fp);

        if (ferror(fp))
        {
            mErrorMsg = strerror(errno);
            mErrorCode = READ_FAILED;
            LOG4CXX_ERROR(xmlFileStreamLog,
                    "failed to read bytes, msg: " << mErrorMsg << "code: " << mErrorCode);

            return -1;
        }

        fTotalBytesRead += fBytesRead;
        LOG4CXX_DEBUG(xmlFileStreamLog,
                "read " << fBytesRead << " bytes from file");

        break;
    case CACHED:
        fBytesRead = cacheObject->readBytes((char *) toFill, maxToRead);
        LOG4CXX_DEBUG(xmlFileStreamLog,
                "read " << fBytesRead << " bytes from cacheObject");
        fTotalBytesRead += fBytesRead;
        break;
    }
    return fBytesRead;
}
