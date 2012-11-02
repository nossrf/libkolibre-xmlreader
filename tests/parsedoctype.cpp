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

#include <iostream>

#include "XmlReader.h"
#include "setup_logging.h"

int main(int argc, char* argv[])
{
    setup_logging();
    if (argc < 2)
    {
        std::cout << "Usage: " << argv[0] << " <file>" << std::endl;
        exit(1);
    }

    LOG4CXX_INFO(logger, "Start test for file '" << argv[1] << "'");

    // Parse file base on file extension
    std::string filename(argv[1]);
    if (filename.substr(filename.find_last_of(".") + 1) == "html")
    {
        XmlReader xmlReader;
        const bool ret = xmlReader.parseHtml(argv[1]);
        if (!ret)
        {
            std::cout << "XmlReader failed to parse " << argv[1] << std::endl;
            exit(1);
        }
    }
    else if (filename.substr(filename.find_last_of(".") + 1) == "xml")
    {
        XmlReader xmlReader;
        const bool ret = xmlReader.parseXml(argv[1]);
        if (!ret)
        {
            std::cout << "XmlReader failed to parse " << argv[1] << std::endl;
            exit(1);
        }
    }

    return 0;
}
