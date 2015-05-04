@echo off

cp ../readme.1st ../zips
cp ../copying* ../zips

call mkdist1 djdev205 Development Kit and Runtime
call mkdist1 djlsr205 Base Library Sources
call mkdist1 djtst205 Test Programs
call mkdist1 djcrx205 For Cross-to-DOS Compiling (subset of djdev)
call mkdist1 djtzn205 Extra Timezone Support Files
call mkdist1 djtzs205 Timezone Sources and Utils

