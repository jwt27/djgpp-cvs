#!/usr/bin/env bash

set -e
set -o pipefail

gen_plt_asmc() {
	grep ASMFUNC $1 | \
		sed -E 's/([0-9]+)[ \t]+([^ \t\(]+[ \t]+)+([^ \(]+) *\(.+/ASMCSYM\(\3, \1\)/'
}

gen_plt_asmc $2
