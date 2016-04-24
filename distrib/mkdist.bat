@echo off

cp ../readme.1st ../zips
cp ../copying* ../zips

call mkdist1 djdev206 Development Kit and Runtime
call mkdist1 djlsr206 Base Library Sources
call mkdist1 djtst206 Test Programs
call mkdist1 djcrx206 For Cross-to-DOS Compiling (subset of djdev)
call mkdist1 djtzn206 Extra Timezone Support Files
call mkdist1 djtzs206 Timezone ODSources and Utils

