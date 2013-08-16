#!/bin/bash

## Copyright (C) 2012 Kolibre
#
# This file is part of kolibre-clientcore.
#
# Kolibre-clientcore is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License as published by
# the Free Software Foundation, either version 2.1 of the License, or
# (at your option) any later version.
#
# Kolibre-clientcore is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with kolibre-clientcore. If not, see <http://www.gnu.org/licenses/>.
#

if [ $# -lt 2 ]; then
    echo "usage: $0 <test case> <request> [test parameters]"
    echo "       test case indicates which test to run e.g. ./parsetest"
    echo "       request indicates which file you would like to fetch e.g. ncc.html"
    echo "       test parameters are used to tweak the test execution, use them with care"
    echo
    echo "note:  specify full path for parameters <test case>"
    echo
    exit 1
fi

if [ -x /usr/bin/gdb ]; then
    PREFIX="libtool --mode=execute gdb --return-child-result -batch -x ${srcdir:-.}/run --args"
fi

if [ ! -f $1 ]; then
    echo "test case '$1' does not exist"
    exit 1
fi

file=`readlink -f $0`
path=`dirname $file`
datafolder="${path}/testdata"
if [ ! -d $datafolder ]; then
    echo "testdata folder '$datafolder' does not exist"
    exit 1
fi

request="$datafolder/$2"
if [ ! -f $request ]; then
    echo "requested file '$2' does not exist in testdata"
    exit 1
fi

start_fakehttpserver()
{
    socat=$(which socat)
    if [ $? -ne 0 ]; then
        echo "socat not found, please install package socat"
        exit 1
    fi

    local protocol=$1
    local port=$2

    file=`readlink -f $0`
    path=`dirname $file`
    fakehttp="${path}/fakehttpresponder.sh"
    datafolder="${path}/testdata"
    if [ $protocol = 'http' ]; then
        socat TCP4-LISTEN:$PORT,fork,tcpwrap=script EXEC:"$fakehttp $datafolder" &
    else
        echo "you must create a certificate file and a private key file in order to use https"
        exit 2
        cert="${path}/ssl-server-crt.crt"
        key="${path}/ssl-private-key.key"
        socat OPENSSL-LISTEN:$PORT,fork,tcpwrap=script,cert=$cert,key=$key,verify=0 EXEC:"$fakehttp $datafolder" &
    fi

    sleep 1
}

kill_fakehttpserver()
{
    local port=$1

    line=`ps ax | grep socat | grep $port`
    pid=`echo $line | cut -d ' ' -f1`
    kill $pid
}

port_in_use()
{
    local port=$1

    #echo "checking if port $port is in use"
    output=`lsof -i -P -n | grep ":$port"`
    if [ ! -z "$output" ]; then
        return 0 # port is in use
    fi
    return 1 # port is not in use
}

# find available port
PORT=$$
if [ $PORT -lt 1025 ]; then
    PORT=`expr $PORT + 1024`
fi

while port_in_use $PORT; do
    echo "looping"
    PORT=`expr $PORT + 1`
done

# use http as default protocol (other options: 'https')
PROTOCOL='http'

# setup test parameters
binary=$1
uri="$PROTOCOL://localhost:$PORT/$2"
# shift input parameters
shift
shift

start_fakehttpserver $PROTOCOL $PORT $@

$PREFIX $binary $uri $@
retval=$?

kill_fakehttpserver $PORT

if [ $retval = 0 ]
then
    exit 0
else
    exit -1
fi
