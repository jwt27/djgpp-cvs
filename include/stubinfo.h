#ifndef __dj_include_stubinfo_h__
#define __dj_include_stubinfo_h__

#include "djdev64/stubinfo.h"

#ifndef __ASSEMBLER__
#include "libc/asmobj.h"
EXTERN ASM_P(_GO32_StubInfo, _stubinfo);
#endif

#endif /* __dj_include_stubinfo_h__ */
