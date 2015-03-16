#!/usr/bin/env perl

$/ = " ";
$first = 1;
while (<>) {
    chomp;
    if (!$first) {
        print " ";
    }
    $first = 0;
    print scalar reverse;
}
