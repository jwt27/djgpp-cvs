#include <stdio.h>
#include <stdlib.h>

int
main (int argc, char **argv)
{
  int res;

  if (argc <= 1)
    {
      /* Assuming we have an echo.exe: */
      res = system ("echo
		William Safire's Rules for Writers:

Remember to never split an infinitive. The passive voice should never be used.
Do not put statements in the negative form. Verbs have to agree with their
subjects. Proofread carefully to see if you words out. If you reread your work,
you can find on rereading a great deal of repetition can be avoided by
rereading and editing. A writer must not shift your point of view. And don't
start a sentence with a conjunction. (Remember, too, a preposition is a
terrible word to end a sentence with.)  Don't overuse exclamation marks!!
Place pronouns as close as possible, especially in long sentences, as of 10
or more words, to their antecedents. Writing carefully, dangling participles
must be avoided. If any word is improper at the end of a sentence, a linking
verb is. Take the bull by the hand and avoid mixing metaphors. Avoid trendy
locutions that sound flaky. Everyone should be careful to use a singular
pronoun with singular nouns in their writing. Always pick on the correct idiom.
The adverb always follows the verb. Last but not least, avoid cliches like the
plague; seek viable alternatives.
> rules.tmp");
      if (!res)
	res = system ("sed s/s/z/g < rules.tmp | tail");
    }
  else
    {
      printf ("Command accepted as : \"%s\"\n", argv[1]);
      res = system (argv[1]);
    }

  if (res) fprintf (stderr, "Exit code was %d.\n", res);
  return 0;
}
