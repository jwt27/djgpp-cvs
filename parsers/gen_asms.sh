gen_incs() {
    grep -R " ASM(" "$1" | grep EXTERN | cut -d ":" -f 1 | \
	xargs -L 1 basename | sed -E 's/(.+)/#include "\1"/'
}

gen_asms() {
    grep -R " ASM(" . "$1" | grep EXTERN | sed -E 's/.+\((.+)\);/asmsym _\1/'
}

gen_asyms() {
    grep -R " ASM(" . "$1" | grep EXTERN | \
	sed -E 's/.+\((.+)\);/\(void \*\*\)\&__\1,/'
}

case "$1" in
    0) gen_incs "$2"
    ;;
    1) gen_asms "$2"
    ;;
    2) gen_asyms "$2"
    ;;
esac
