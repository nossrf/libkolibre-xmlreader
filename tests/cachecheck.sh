#!/bin/sh

if [ -x /usr/bin/gdb ]; then
    PREFIX="libtool --mode=execute gdb -return-child-result -x ${srcdir:-.}/run --args"
fi

$PREFIX ./cachecheck ${srcdir:-.}/testdata/sample3.xml
