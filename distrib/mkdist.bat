@echo off

cp ../readme.1st ../zips
cp ../copying* ../zips

call mkdist1 djdev204 Development Kit and Runtime
call mkdist1 djlsr204 Base Library Sources
call mkdist1 djtst204 Test Programs
call mkdist1 djcrx204 For Cross-to-DOS Compiling (subset of djdev)
call mkdist1 djtzn204 Extra Timezone Support Files
call mkdist1 djtzs204 Timezone Sources and Utils

