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
#include <fstream>
#include <cstdlib>
#include <string>
#include <vector>
#include <algorithm>

#include "CacheObject.h"
#include "setup_logging.h"

const size_t BUF_SIZE = 1024; // Z_CHUNK_SIZE;

using namespace std;

int main(int argc, char* argv[])
{
    setup_logging();
    if (argc < 2)
    {
        std::cout << "Usage: " << argv[0] << " [file]" << std::endl;
        exit(1);
    }

    vector<char> filecont, bufcont;
    char buffer[BUF_SIZE] =
    { 0 };
    CacheObject cache("mycache");
    fstream file_op(argv[1], ios::in | ios::binary);
    int res;

    cout << "Start writing to cacheObject buffer" << endl;
    cout << "cacheObject state:" << cache.getState() << endl;

    // Write the file to the cache object
    while (!file_op.eof())
    {
        file_op.read(buffer, BUF_SIZE - 1);
        buffer[file_op.gcount()] = 0;
        res = cache.writeBytes(buffer, file_op.gcount());
        if (res <= 0)
            cout << "error code" << res << endl;
        copy(buffer, buffer + file_op.gcount(), back_inserter(filecont));
    }

    // Close the out buffer and deflate the data
    cache.writeBytes(NULL, 0);
    cache.setContentLength(filecont.size());

    cout << "cacheObject buffer size:" << cache.getBufferSize() << endl;
    cout << "cacheObject content lenght:" << cache.getContentLength() << endl;

    cout << "Start reading from cacheObject buffer" << endl;
    cout << "cacheObject state:" << cache.getState() << endl;

    // Read the file from the cache object
    do
    {
        res = cache.readBytes(buffer, BUF_SIZE - 1);
        copy(buffer, buffer + res, back_inserter(bufcont));
    } while (res != 0);

    if (filecont != bufcont)
    {
        cout << "Cache object deflate - inflate failed" << endl;
        return 1;
    }

    // Cleanup and exit
    file_op.close();

    return 0;
}
