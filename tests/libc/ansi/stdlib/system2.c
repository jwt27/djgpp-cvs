#include <stdio.h>
#include <stdlib.h>

int
main (int argc, char **argv)
{
  int res;

  if (argc <= 1)
    {
      /* Assuming we have a djecho.exe: */
      res = system ("djecho \
\"		William Safire\'s Rules for Writers:\n\
\n\
Remember to never split an infinitive. The passive voice should never be used.\n\
Do not put statements in the negative form. Verbs have to agree with their\n\
subjects. Proofread carefully to see if you words out. If you reread your work,\n\
you can find on rereading a great deal of repetition can be avoided by\n\
rereading and editing. A writer must not shift your point of view. And don\'t\n\
start a sentence with a conjunction. (Remember, too, a preposition is a\n\
terrible word to end a sentence with.)  Don\'t overuse exclamation marks!!\n\
Place pronouns as close as possible, especially in long sentences, as of 10\n\
or more words, to their antecedents. Writing carefully, dangling participles\n\
must be avoided. If any word is improper at the end of a sentence, a linking\n\
verb is. Take the bull by the hand and avoid mixing metaphors. Avoid trendy\n\
locutions that sound flaky. Everyone should be careful to use a singular\n\
pronoun with singular nouns in their writing. Always pick on the correct idiom.\n\
The adverb always follows the verb. Last but not least, avoid cliches like the\n\
plague; seek viable alternatives.\"\
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
