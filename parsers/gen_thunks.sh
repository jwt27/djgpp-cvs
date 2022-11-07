#!/usr/bin/env bash

set -e
set -o pipefail

$1 1 <$2 | sort | uniq | autom4te -l m4sugar $3/thunks.m4 -
$1 2 <$2
