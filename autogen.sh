#!/usr/bin/env bash
autoreconf --install &&
aclocal &&
automake --add-missing &&
autoconf
