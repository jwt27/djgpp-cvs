@echo off

cp ../readme.1st ../zips
cp ../copying* ../zips

call mkdist1 djdev202 Development Kit and Runtime
call mkdist1 djlsr202 Base Library Sources
call mkdist1 djtst202 Test Programs
call mkdist1 djcrx202 For Cross-to-DOS Compiling (subset of djdev)
call mkdist1 djtzn202 Extra Timezone Support Files
call mkdist1 djtzs202 Timezone Sources and Utils

