#!perl
# -*- perl -*-

$rm = 0;
if ($ARGV[0] eq "-rm") {
    $rm = 1;
    print "Actually removing files...\n";
    shift;
}

open(F, "find . -type f -print |");
while (<F>) {
    $t = 0;
    s/\r//g;

    $t = 1 if /\~/;
    $t = 1 if /\#/;
    $t = 1 if /\/[a-h]{8}$/;

    if ($t) {
	print;
	if ($rm) {
	    s/[\r\n]//g;
	    unlink($_) || die("unlink($_)");
	}
    }
}
close(F);
