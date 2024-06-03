# dj64dev development suite

## what is that?
dj64dev is a development suite that allows to cross-build 64bit programs
for DOS. It consists of 2 parts: dj64 tool-chain and djdev64 suite.

### dj64 tool-chain
dj64 is a djgpp-compatible tool-chain that compiles the djgpp-buildable
sources for DOS. But unlike djgpp that produces 32bit code, dj64
produces 64bit code.<br/>
The resulting programs run on the emulated DOS environment, with eg
[dosemu2](https://github.com/dosemu2/dosemu2) emulator. In theory the 64bit
DOS extender can be written to run such programs under the bare-metal
DOS, but the future of DOS is probably in the emulated environments anyway.

### djdev64 suite
djdev64 suite is a set of libraries and headers that are needed to
implement the "DJ64" and "DJ64STUB" DPMI extensions on a DPMI host.<br/>
"DJ64" is an extension that allows the dj64-built programs to access
the 64bit world.<br/>
"DJ64STUB" is an optional DPMI extension that implements a loader for
dj64-built programs. If "DJ64STUB" extension is missing, you need to have
the full loader inside the program's stub.<br/>
[djstub](https://github.com/stsp/djstub/) project provides both loader-less
and loader-enabled stubs, but the default is the loader-less ministub that
relies on a "DJ64STUB" loader inside DPMI host.

If you have an asm-written DPMI server without an ability to talk to
C-written code, then you likely can't have dj64 support in it, as writing
the "DJ64" DPMI extension by hands, without using djdev64 suite, is too
difficult or impossible.

In addition to that, dj64-built programs rely on a few DPMI-1.0 functions.
Namely, shared memory functions
[0xd00](https://www.delorie.com/djgpp/doc/dpmi/api/310d00.html),
[0xd01](https://www.delorie.com/djgpp/doc/dpmi/api/310d01.html)
and optionally also "Free Physical Address Mapping" function
[0x801](https://www.delorie.com/djgpp/doc/dpmi/api/310801.html)
which is used to unmap shared memory regions without actually destroying
them. DPMI host is not required to implement such a specific 0x801
functionality, but the shared memory support is mandatory.

## building and installing
First, you need to install [thunk_gen](https://github.com/stsp/thunk_gen/).
Pre-built packages are available
[for ubuntu](https://code.launchpad.net/~stsp-0/+archive/ubuntu/thunk-gen)
and
[for fedora](https://copr.fedorainfracloud.org/coprs/stsp/dosemu2/).<br/>
Then run `make`.<br/>
For installing run `sudo make install`.<br/>
Like gcc should be accompanied with binutils in order to produce executables,
dj64 need to be accompanied with
[djstub](https://github.com/stsp/djstub/)
package for the same purpose. That package installs `djstubify`, `djstrip`
and `djlink` binaries that are needed for the final building steps.

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

## inspecting
You may want to analyze the structure of the dj64-built files to get
the more detailed view of its architecture. You can use `djstubify -i`
for that task:
```
$ djstubify -i comcom64.exe
dj64 file format
Overlay 0 (i386/ELF DOS payload) at 23368, size 30548
Overlay 1 (x86_64/ELF host payload) at 53916, size 87048
Overlay 2 (x86_64/ELF host debug info) at 140964, size 174936
Overlay name: comcom64.exe
Stub version: 4
Stub flags: 0x0b07
```
As can be seen, the executable consists of 3 overlays. If you use
`djstrip` on it, then only 2 remain. Overlay name is needed for
debugger support, for which we use the GNU debuglink technique.<br/>
Stub flags are used to create the shared memory regions with the
[0xd00](https://www.delorie.com/djgpp/doc/dpmi/api/310d00.html)
DPMI-1.0 function. They are not documented in a DPMI spec, so their
support by the DPMI host for dj64 is actually optional.

We can compare that structure with the regular djgpp-built executable:
```
$ djstubify -i comcom32.exe
exe/djgpp file format
COFF payload at 2048
```
Nothing interesting here, except that we see djgpp uses COFF format
instead of ELF. But what if we re-stub the old executable?
```
$ djstubify comcom32.exe
$ djstubify -i comcom32.exe
dj64 file format
Overlay 0 (i386/COFF DOS payload) at 23368, size 256000
```
Now this executable is identified as having the dj64 file format, but
of course it still has just 1 COFF overlay. Sorry but the conversion
from COFF to ELF is not happening. :) But our loaders support both
COFF and ELF formats, so dj64/COFF combination is also functional,
albeit never produced by the dj64 tool-chain itself.

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
