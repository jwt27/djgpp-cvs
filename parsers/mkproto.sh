#!/usr/bin/env bash

if [ $# -lt 4 ]; then
    echo "generate thunk prototypes compatible with fdpp's thunk_gen & plt"
    echo "$0 <lib_path> <hdr_path> <athunks_out> <cthunks_out>"
    exit 1
fi

extr_proto() {
    readtags -t "$1" "$2" | \
	expand | \
	sed -E \
	    -e 's/.*\/\^(.*;).*/\1/' \
	    -e 's/__P\((.*)\);/\1;/' \
	    -e 's/__attribute__\(\(__(.*)__\)\)/\U\1/' \
	    -e 's/__attribute__\(\((.*)\)\)/\U\1/' \
	    -e "s/(.*)( [^ (]+)( *\([^(]+.*;)/\1 $3 \2\3/" \
	       | \
	tr -s '[:blank:]'
}

list_syms() {
    nm -A "$1" | grep " $2 " | tr -s '[:blank:]' | cut -d " " -f 3 | \
	sed 's/^_//'
}

TF=/tmp/tagsxx
TL=/tmp/a.so
set -e

ld -melf_i386 -shared -Bsymbolic -o $TL "$2" --whole-archive "$1" 2>/dev/null
shift 2
PRUNES="-name libm -prune -o -name machine -prune"
PRUNES="$PRUNES -o -name string.h -prune"
# bad/temporary prunes below
PRUNES="$PRUNES -o -name setjmp.h -prune"
find $1 $PRUNES -o -print | \
	ctags -L - --kinds-C=p --pattern-length-limit=0 -f $TF
shift
# https://stackoverflow.com/questions/11003418/calling-shell-functions-with-xargs
export -f extr_proto
list_syms $TL T | xargs -I '{}' bash -c "extr_proto $TF '{}' ASMFUNC" | nl -n ln -v 0 >$1
list_syms $TL U | xargs -I '{}' bash -c "extr_proto $TF '{}' ASMCFUNC" | nl -n ln -v 0 >$2
list_syms $TL U | xargs readtags -t $TF | expand | \
	cut -d " " -f 1 | nl -n ln -v 0 | expand | tr -s '[:blank:]' | \
	sed -E 's/([^ ]+) +(.*)/asmcfunc \2,\1/' >$3
shift 3

rm $TF
rm $TL
