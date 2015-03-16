#!/usr/bin/env perl

$next = int rand(2000);
while ($_ =  getc) {
    if ($next-- == 0) {
        print " ";
        $next = int rand(2000);
    }
    print;
}
