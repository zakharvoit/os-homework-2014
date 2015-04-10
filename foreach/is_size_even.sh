#!/usr/bin/env bash

size=`wc -c < "$@" 2>/dev/null`
let 'size %= 2'
exit $size
