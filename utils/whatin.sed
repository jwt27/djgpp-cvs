s/^char[^a-zA-Z0-9_]+/[35m&[37m/g
s/[a-z]+_t/[36m&[37m/g
s/\([^a-zA-Z0-9_]\)\(char[^a-zA-Z0-9_]+\)/\1[35m\2[37m/g
s/typedef/[32m&[37m/g
s/struct/[32m&[37m/g
s/double[^a-zA-Z0-9_]*/[32m&[37m/g
s/long[^a-zA-Z0-9_]*/[35m&[37m/g
s/void[^a-zA-Z0-9_]*/[35m&[37m/g
s/const/[35m&[37m/g
s/\([ (,]\)\(int \)/\1[35m\2[37m/g
s/^int[^a-zA-Z0-9_]*/[35m&[37m/g
s/unsigned/[35m&[37m/g
s/[{},;()]/[33m&[37m/g
s/\([^]\)[[]/\1[33m[[37m/g
s/[]]/[33m&[37m/g
