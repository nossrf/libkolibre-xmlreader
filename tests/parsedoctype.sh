#!/bin/sh

if [ -x /usr/bin/gdb ]; then
    PREFIX="libtool --mode=execute gdb -return-child-result -x ${srcdir:-.}/run --args"
fi

$PREFIX ./parsedoctype ${srcdir:-.}/testdata/html_4.0_strict.html
$PREFIX ./parsedoctype ${srcdir:-.}/testdata/html_4.0_transitional_internal.html
$PREFIX ./parsedoctype ${srcdir:-.}/testdata/xhtml_1.0_strict.html
$PREFIX ./parsedoctype ${srcdir:-.}/testdata/xhtml_1.0_transitional_internal.html
$PREFIX ./parsedoctype ${srcdir:-.}/testdata/dtbook_2005_1_strict.xml
$PREFIX ./parsedoctype ${srcdir:-.}/testdata/dtbook_2005_1_strict_interal.xml
