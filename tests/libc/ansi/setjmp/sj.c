#include <stdio.h>
#include <setjmp.h>

int main(void)
{
  jmp_buf a, b;
  int i;

  printf("test of setjmp/longjmp\n");

  i = setjmp(a);

  printf("\n");
  printf("ax=%08lx bx=%08lx cx=%08lx dx=%08lx si=%08lx di=%08lx\n",
	 a->__eax, a->__ebx, a->__ecx, a->__edx, a->__esi, a->__edi);
  printf("cs:eip=%04x:%08lx bp=%08lx ss:esp=%04x:%08lx\n",
	 a->__cs, a->__eip, a->__ebp, a->__ss, a->__esp);
  printf("cs=%04x ds=%04x es=%04x fs=%04x gs=%04x ss=%04x fl=%08lx\n",
	 a->__cs, a->__ds, a->__es, a->__fs, a->__gs, a->__ss, a->__eflags);

  setjmp(b);
  printf("\n");
  printf("ax=%08lx bx=%08lx cx=%08lx dx=%08lx si=%08lx di=%08lx\n",
	 b->__eax, b->__ebx, b->__ecx, b->__edx, b->__esi, b->__edi);
  printf("cs:eip=%04x:%08lx bp=%08lx ss:esp=%04x:%08lx\n",
	 b->__cs, b->__eip, b->__ebp, b->__ss, b->__esp);
  printf("cs=%04x ds=%04x es=%04x fs=%04x gs=%04x ss=%04x fl=%08lx\n",
	 b->__cs, b->__ds, b->__es, b->__fs, b->__gs, b->__ss, b->__eflags);

  if (i == 0)
    longjmp(a, 1);
  return 0;
}
