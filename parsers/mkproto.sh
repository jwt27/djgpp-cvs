if [ $# -lt 4 ]; then
    echo "$0 <lib_path> <hdr_path> <athunks_out> <cthunks_out>"
    exit 1
fi

extr_proto() {
    readtags -t "$1" "$2" | \
	expand | \
	sed -E \
	    -e 's/.*\/\^(.*;).*/\1/' \
	    -e 's/__P\((.*)\);/\1;/' \
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
ld -melf_i386 -shared -Bsymbolic -o $TL --whole-archive "$1" 2>/dev/null
find $2 -name libm -prune -o -print | \
	ctags -L - --kinds-C=p --pattern-length-limit=0 -f $TF
# https://stackoverflow.com/questions/11003418/calling-shell-functions-with-xargs
export -f extr_proto
list_syms $TL T | xargs -I '{}' bash -c "extr_proto $TF '{}' ASMFUNC" | nl -n ln -v 0 >$3
list_syms $TL U | xargs -I '{}' bash -c "extr_proto $TF '{}' ASMCFUNC" | nl -n ln -v 0 >$4

rm $TF
rm $TL
