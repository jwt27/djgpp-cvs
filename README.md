# dj64 tool-chain

## what is that?
dj64 is a djgpp-compatible tool-chain that compiles the djgpp-buildable
sources for DOS. But unlike djgpp that produces 32bit code, dj64
produces 64bit code.<br/>
The resulting programs run on the emulated DOS environment, with eg
[dosemu2](https://github.com/dosemu2/dosemu2) emulator. In theory the 64bit
DOS extender can be written to run such programs under the bare-metal
DOS, but the future of DOS is probably in the emulated environments anyway.

## building and installing
First, you need to install [thunk_gen](https://github.com/stsp/thunk_gen/).
Pre-built packages are available
[for ubuntu](https://code.launchpad.net/~stsp-0/+archive/ubuntu/thunk-gen)
and
[for fedora](https://copr.fedorainfracloud.org/coprs/stsp/dosemu2/).<br/>
Then run `make`.<br/>
For installing run `sudo make install`.<br/>
Like gcc needs binutils in order to produce executables, dj64 needs
[djstub](https://github.com/stsp/djstub/)
package for the same purpose. That package installs `djstubify` and
`djstrip` binaries that are needed for the final building steps.

## installing from pre-built packages
For the ubuntu package please visit
[dj64 ppa](https://code.launchpad.net/~stsp-0/+archive/ubuntu/dj64).
Fedora packages are
[here](https://copr.fedorainfracloud.org/coprs/stsp/dosemu2).

## running
The simplest way to get dj64-built programs running is to use
[dosemu2](https://github.com/dosemu2/dosemu2).<br/>
Get the pre-built dosemu2 packages from
[ubuntu ppa](https://code.launchpad.net/~dosemu2/+archive/ubuntu/ppa)
or from
[copr repo](https://copr.fedorainfracloud.org/coprs/stsp/dosemu2)
or build it from
[sources](https://github.com/dosemu2/dosemu2).
dosemu2 uses the dj64-built command.com called
[comcom64](https://github.com/dosemu2/comcom64/).
You can type `ver` to make sure its the right one, in which case you
are already observing the first dj64-built program in the run. :)

## building your own program
Well, this is the most tricky part. First, a few preparations should be
made to the source code to make it more portable:

- Inline asm should be moved to the separate assembler files and called
  as a functions.
- Non-portable `movedata()` function should be replaced with the
  [fmemcpy*()](https://github.com/stsp/dj64dev/blob/master/include/sys/fmemcpy.h)
  set of functions that are provided by dj64. Their use is very similar to
  that of `movedata()`, except that pointers are used instead of selectors.
- Use macros like DATA_PTR() and PTR_DATA() to convert between the DOS
  offsets and 64bit pointers. Plain type-casts should now be avoided for
  that purpose.
- Update your makefile to involve thunk_gen into a build process.
  This procedure is currently not properly documented, so you need to look
  into
  [this makefile](https://github.com/dosemu2/comcom64/blob/master/src/Makefile)
  of comcom64. Note that the source tree contains the
  [32](https://github.com/dosemu2/comcom64/tree/master/32)
  directory with djgpp-compatible makefile. You can compare them to figure
  out what needs to be changed, or ask me for help on github. :)
  This example shows that the same sources can be built by both dj64 and
  djgpp, meaning a rather good compatibility level.

Once you managed to build the code, you get an executable that you can
run under dosemu2. dj64 is a very young project, so don't expect your
code to work from the first attempt. :) Perhaps you'll need to submit
a bug report or two (or twenty) before you can get it running.

## what's unimplemented
- direct calls from C to assembler code (calls from asm to C supported)
- some crt0 overrides (only `_crt0_startup_flags` override is supported)

This functionality is unsupported not because its difficult to implement,
but rather because I am not using it myself. Once it is needed, it can
be trivially added.

## debugging
Debugging with host gdb is supported. The djstub package provides a
`djstrip` binary to strip the debug info from an executable.<br/>
You need to attach gdb to the running instance of dosemu2, or just
run `dosemu -gdb`. Once the dj64-built program is loaded, gdb will
be able to access its symbols.

## so x86-only?
Of course not! This tool-chain is cross-platform. But the resulting
binaries are unfortunately not. If you want to run your program on
x86_64 and aarch64, you need to produce 2 separate executables.
aarch64-built executable will work on aarch64-built dosemu2.

## why would I need that?
Well, maybe you don't. :) If you don't have any djgpp-built project of
yours or you don't want to move it to 64bits, then you don't need to care
about dj64 project. It was written for dosemu2, and while I'd be happy
if someone uses it on its own, this wasn't an initial intention.<br/>
Also if your djgpp-buildable project is well-written and uses some
portable libraries like allegro, then most likely you already have the
native 64bit ports for modern platforms, rather than for DOS. In that
case you also don't need dj64. But maybe you are interested in a host-gdb
debugging and aarch64 support?<br/>
Summing it up, dj64 is a niche project that may not be useful outside
of dosemu2. But I'd like to be wrong on that. :)

## license
dj64 code is derived from djgpp libc sources, so most files are covered
by GPLv2+, see
[copying.dj](https://github.com/stsp/dj64dev/blob/master/copying.dj)
for details. dj64-specific files are covered by GPLv3+, see
[LICENSE](https://github.com/stsp/dj64dev/blob/master/LICENSE).
