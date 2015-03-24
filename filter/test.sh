#!/usr/bin/env bash

rm -f .list
echo -n "Generating test files: "
for i in `seq 1 100`; do
    head -c $RANDOM /dev/urandom >".buffer$i"
    echo ".buffer$i" >>.list
    echo -n "$i "
done
echo "Done!"

for pred in true \
            false \
            ./is_size_even.sh \
            "./grep_quiet.sh -i aa" \
            cat 'wc -l' \
            "echo `(for a in $(seq 1 300) ; do echo $a ; done) | xargs echo -n`"
                do

    if [[ ${#pred} -lt 300 ]] ; then
        pred_name="$pred"
    else
        pred_name="long predicate"
    fi
    echo "Testing $pred_name"
    echo "Running tests"
    # echo for unquoting
    ./filter `echo -n $pred` <.list >.actual

    perl -nE 'BEGIN {$c.=" $_" while($_ = shift) } chomp; say if system("$c $_ 2>/dev/null") == 0' $pred <.list >.expected
    perl -i -ne 'print if $_ ne "\n"' .actual .expected
    cmp .expected .actual
    if [[ $? = 0 ]] ; then
        echo "OK $pred_name"
    elif [[ ${#pred} -ge 300 ]]; then
        echo "Ignoring long predicate result checking"
    else
        echo "Error $pred_name"
        echo "See .list"
        exit 1
    fi
done

rm -f .list
rm -f .buffer*
rm -f .actual .expected

