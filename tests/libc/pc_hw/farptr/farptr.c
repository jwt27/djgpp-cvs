#include <sys/farptr.h>

int
main(long l, short s, char c)
{
  asm("\n/* ------------ farpokeb */");
  _farpokeb(s, l, l);
  _farpokeb(s, l, s);
  _farpokeb(s, l, c);
  _farpokeb(s, l, 0x12);

  asm("\n/* ------------ farpokew */");
  _farpokew(s, l, l);
  _farpokew(s, l, s);
  _farpokew(s, l, c);
  _farpokew(s, l, 0x1234);

  asm("\n/* ------------ farpokel */");
  _farpokel(s, l, l);
  _farpokel(s, l, s);
  _farpokel(s, l, c);
  _farpokel(s, l, 0x12345678);

  asm("\n/* ------------ farpeek* */");
  s = _farpeekb(s, l);
  c = _farpeekw(s, l);
  l = _farpeekl(s, l);

  asm("\n/* ------------ farsetsel */");
  _farsetsel(s);

  asm("\n/* ------------ farnspokeb */");
  _farnspokeb(l, l);
  _farnspokeb(l, s);
  _farnspokeb(l, c);
  _farnspokeb(l, 0x12);

  asm("\n/* ------------ farnspokew */");
  _farnspokew(l, l);
  _farnspokew(l, s);
  _farnspokew(l, c);
  _farnspokew(l, 0x1234);

  asm("\n/* ------------ farnspokel */");
  _farnspokel(l, l);
  _farnspokel(l, s);
  _farnspokel(l, c);
  _farnspokel(l, 0x12345678);

  asm("\n/* ------------ farnspeek* */");
  c = _farnspeekb(l);
  l = _farnspeekw(l);
  s = _farnspeekl(l);

  asm("\n/* ------------ */");
  return 0;
}
