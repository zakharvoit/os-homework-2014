#!/usr/bin/env bash

for i in `seq 1 100` ; do
   echo `head -n 100 /dev/urandom`  >.buffer.txt
   ./bufcat <.buffer.txt >.actual.txt
   cat .buffer.txt >.expected.txt
   cmp .expected.txt .actual.txt
   if [[ $? != 0 ]] ; then
       echo "Test $i failed see .buffer.txt";
       rm -f .expected.txt .actual.txt
       exit 1
   fi
   echo "Test $i ok"
done

rm -f .expected.txt .actual.txt .buffer.txt
