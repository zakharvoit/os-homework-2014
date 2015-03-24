#!/bin/zsh
#!/usr/bin/env bash

rm -f .list
echo -n "Generating test files: "
for i in `seq 1 100`; do
    head -c $RANDOM /dev/urandom >".buffer$i"
    echo ".buffer$i" >>.list
    echo -n "$i "
done
echo "Done!"

for pred in true false ./is_size_even.sh "./grep_quiet.sh -i aa" cat "wc -l" ; do
    echo "Testing predicate $pred"
    echo "Running tests"
    ./filter $pred <.list >.actual
    perl -nE 'BEGIN {$c.=" $_" while($_ = shift) } chomp; say if system("$c $_ 2>/dev/null") == 0' $pred <.list >.expected
    perl -i -ne 'print if $_ ne "\n"' .actual
    cmp .expected .actual
    if [[ $? = 0 ]] ; then
        echo "OK $pred"
    else
        echo "Error $pred"
        echo "See .list"
        exit 1
    fi
done

rm -f .list
rm -f .buffer*
rm -f .actual .expected

