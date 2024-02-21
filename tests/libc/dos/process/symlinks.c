/* Testsuite for symlink handling aspect in launching program.
 * This is by no means a regression testsuite, you have to decide
 * if bad stuff happens from output.
 *
 * Shell script handling will be checked only if you have bash installed.
 */
#include <process.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(void)
{
   symlink("args.exe", "linkargs.exe");
   system("args.exe 1 2 3");
   system("linkargs.exe 4 5 6");
   spawnl(P_WAIT, "args.exe", "args.exe", "what", NULL);
   spawnl(P_WAIT, "linkargs.exe", "linkargs.exe", "where", "why", NULL);
   remove("linkargs.exe");
   symlink("C:/command.com", "linkcom.com");
   system("linkcom.com");
   remove("linkcom.com");
   symlink("test.sh", "linksh.sh");
   system("test.sh");
   system("linksh.sh");
   remove("linksh.sh");
   symlink("/dev/env/DJDIR/bin/ls.exe", "/dev/env/DJDIR/bin/sl.exe");
   system("sl -l");
   remove("/dev/env/DJDIR/bin/sl.exe");
   return 0;
}
