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

#include <string>
#include <assert.h>

#include "HttpStream.h"

using namespace std;

int main(int argc, char* argv[])
{
    string url;

    // urls with username and password
    url = "http://foo:bar@www.localhost.com:1234/foobar";
    assert(url2hostname(url) == "www.localhost.com");
    assert(url2username(url) == "foo");
    assert(url2password(url) == "bar");

    url = "http://foo:bar@www.localhost.com:1234";
    assert(url2hostname(url) == "www.localhost.com");
    assert(url2username(url) == "foo");
    assert(url2password(url) == "bar");

    url = "http://foo:bar@www.localhost.com";
    assert(url2hostname(url) == "www.localhost.com");
    assert(url2username(url) == "foo");
    assert(url2password(url) == "bar");

    // urls with username and password and no protocol
    url = "foo:bar@www.localhost.com:1234/foobar";
    assert(url2hostname(url) == "");
    assert(url2username(url) == "");
    assert(url2password(url) == "");

    url = "foo:bar@www.localhost.com:1234";
    assert(url2hostname(url) == "");
    assert(url2username(url) == "");
    assert(url2password(url) == "");

    url = "foo:bar@www.localhost.com";
    assert(url2hostname(url) == "");
    assert(url2username(url) == "");
    assert(url2password(url) == "");

    // urls with username only
    url = "http://foo@www.localhost.com:1234/foobar";
    assert(url2hostname(url) == "www.localhost.com");
    assert(url2username(url) == "foo");
    assert(url2password(url) == "");

    url = "http://foo@www.localhost.com:1234";
    assert(url2hostname(url) == "www.localhost.com");
    assert(url2username(url) == "foo");
    assert(url2password(url) == "");

    url = "http://foo@www.localhost.com";
    assert(url2hostname(url) == "www.localhost.com");
    assert(url2username(url) == "foo");
    assert(url2password(url) == "");

    // urls with username only and no protocol
    url = "foo@www.localhost.com:1234/foobar";
    assert(url2hostname(url) == "");
    assert(url2username(url) == "");
    assert(url2password(url) == "");

    url = "foo@www.localhost.com:1234";
    assert(url2hostname(url) == "");
    assert(url2username(url) == "");
    assert(url2password(url) == "");

    url = "foo@www.localhost.com";
    assert(url2hostname(url) == "");
    assert(url2username(url) == "");
    assert(url2password(url) == "");

    return 0;
}
