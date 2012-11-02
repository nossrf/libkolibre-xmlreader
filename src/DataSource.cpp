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

// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include "InputStream.h"
#include "DataSource.h"
#include "DataStreamHandler.h"

// ---------------------------------------------------------------------------
//  DataSource: Constructors and Destructor
// ---------------------------------------------------------------------------
DataSource::DataSource(std::string url, bool tidyflag)
{
    sUrl = url;
    inpstr = (InputStream *) DataStreamHandler::Instance()->newStream(url,
            tidyflag);
}

DataSource::~DataSource()
{
    delete inpstr;
}

// ---------------------------------------------------------------------------
//  DataSource: Implementation of the input source interface
// ---------------------------------------------------------------------------
InputStream* DataSource::makeStream() const
{
    return (InputStream *) inpstr;
}

inline std::string DataSource::getUrl() const
{
    return sUrl;
}
