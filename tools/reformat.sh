#!/bin/sh
for f in `find . -name external -prune -o \( -name \*.cpp -o -name \*.h -o -name \*.inc \) -print`  ; do
    clang-format -i $f
done
for f in `find . -name external -prune -o \( -name \*.x\?\? -o -name \*.sch \) -print`  ; do
    xmllint --format --nowarning -o $f - < $f
done
