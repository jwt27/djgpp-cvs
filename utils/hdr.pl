#!/bin/perl
# -*- perl -*-

$inc = "d:/v2/include";


$pattern = shift;

print "\n";

opendir(INC, $inc);
@files = sort readdir(INC);
closedir(INC);

for $file (@files) {
    next if $file =~ /^\./;
    if ( -d $file ) {
	push(@subs, $file);
    } else {
	&scan_file($file);
    }
}

for $dir (@subs) {
    opendir(INC, "$inc/$dir");
    @files = sort readdir(INC);
    closedir(INC);

    for $file (@files) {
	next if $file =~ /^\./;
	&scan_file("$dir/$file");
    }
}

sub scan_file {
    local($file) = @_;
    open(F, "$inc/$file");
    $tag = '?';
    while (<F>) {
	if (/$pattern/io) {
	    print "$file:$tag: $_";
	}
	$tag = 'A' if /\#ifndef __dj_ENFORCE_ANSI_FREESTANDING/;
	$tag = 'P' if /\#ifndef __STRICT_ANSI__/;
	$tag = 'D' if /\#ifndef _POSIX_SOURCE/;
	$tag = 'I' if /\#ifndef __dj_ENFORCE_FUNCTION_CALLS/;
    }
    close(F);
}

print "\n";
