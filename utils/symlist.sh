echo `nm -g ../lib/libc.a | grep " [TDBC] " | grep -v " __" | sed 's/^.* [TDBC] _//' | sort`
