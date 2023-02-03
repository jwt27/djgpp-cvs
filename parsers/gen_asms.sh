gen_incs() {
    cd "$1" && egrep -R --include="*.h" " ASM\(| ASM_P\(| ASM_AP\(| ASM_N\(| ASM_F\(" . | grep EXTERN | \
	cut -d ":" -f 1 | uniq | sed -E 's/\.\/(.+)/#include "\1"/'
}

gen_incsn() {
    cd "$1" && grep -R --include="*.h" " ASM_N(" . | grep EXTERN | cut -d ":" -f 1 | uniq | \
	sed -E 's/\.\/(.+)/#include "\1"/'
}

gen_asms() {
    egrep -R --include="*.h" " ASM\(| ASM_N\(| ASM_F\(" . "$1" | grep EXTERN | \
	sed -E 's/.+\((.+)\);.*/asmsym _\1/' | sort | uniq
    egrep -R --include="*.h" " ASM_P\(| ASM_AP\(" . "$1" | grep EXTERN | \
	sed -E 's/.+\(.+, (.+)\);.*/asmsym _\1/' | sort | uniq
}

gen_asyms() {
    egrep -R --include="*.h" " ASM\(| ASM_N\(| ASM_F\(" . "$1" | grep EXTERN | \
	sed -E 's/.+\((.+)\);.*/\(void \*\*\)\&__\1,/' | sort | uniq
    egrep -R --include="*.h" " ASM_P\(| ASM_AP\(" . "$1" | grep EXTERN | \
	sed -E 's/.+\(.+, (.+)\);.*/\(void \*\*\)\&__\1,/' | sort | uniq
}

case "$1" in
    0) gen_incs "$2"
    ;;
    1) gen_incsn "$2"
    ;;
    2) gen_asms "$2"
    ;;
    3) gen_asyms "$2"
    ;;
esac
