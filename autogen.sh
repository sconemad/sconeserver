#!/bin/sh

echo "Generating build information..."

aclocal
autoheader
automake --gnits --include-deps --add-missing --copy 
autoconf
rm -rf ./autom4te.cache
