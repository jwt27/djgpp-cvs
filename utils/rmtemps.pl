#!/bin/perl
# -*- perl -*-

$rm = 0;
if ($ARGV[0] eq "-rm") {
    $rm = 1;
    print "Actually removing files...\n";
    shift;
} else {
    print "Use -rm to actually remove files\n";
}

open(F, "find . -type f -print |");
while (<F>) {
    $t = 0;
    s/[\r\n]+//g;

    $t = 1 if /\~/;
    $t = 1 if /\#/;
    $t = 1 if /\.(orig|rej)$/;
    #$t = 1 if /\/[a-d][a-z]{7}$/ && ! /zoneinfo\//;

    if ($t) {
	print $_, "\n";
	if ($rm) {
	    unlink($_) || die("unlink($_)");
	}
    }
}
close(F);
