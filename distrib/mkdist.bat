@echo off

cp ../readme.1st ../zips
cp ../copying* ../zips

call mkdist1 djdev203 Development Kit and Runtime
call mkdist1 djlsr203 Base Library Sources
call mkdist1 djtst203 Test Programs
call mkdist1 djcrx203 For Cross-to-DOS Compiling (subset of djdev)
call mkdist1 djtzn203 Extra Timezone Support Files
call mkdist1 djtzs203 Timezone Sources and Utils

