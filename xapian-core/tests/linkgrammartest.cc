/* linkgrammartest.cc: Tests of Xapian::LinkGrammar
 *
 * Copyright (C) 2012 Sehaj Singh Kalra
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA
 */

#include <config.h>

#include <xapian.h>

#include <iostream>
#include <string>

#include "testsuite.h"
#include "testutils.h"

using namespace std;

#ifdef HAVE_LIBLINK_GRAMMAR

static bool test_lg_throw_error()
{
    try {
        throw Xapian::LinkGrammarError("pseudo_test");
    } catch (const Xapian::LinkGrammarError & e) {
        return true;
    }
    return false;
}

// To ensure that if new POS are added in future, then change is made to
// the method pos_to_string as well.
// Same holds if removing an existing POS
// So in that case, update the info about new POS here as well.
static bool test_lg_pos_to_string1()
{
    try {
        Xapian::LinkGrammar pos_tagger;
        unsigned pos_type_value[] = {0, 1, 2, 3, 4, 5};
        string POS[] = {"NOUN", "VERB", "ADJECTIVE", "ADVERB", "NOUNPHRASE", ""};

        for(unsigned i = 0; i < sizeof(pos_type_value)/sizeof(unsigned); i++)
            TEST_STRINGS_EQUAL(pos_tagger.pos_to_string(pos_type_value[i]), POS[i]);
        return true;
    } catch (const Xapian::LinkGrammarError &e) {
        return false;
    } catch (const Xapian::Error &e) {
        return false;
    } catch (...) {
        return false;
    }
}

static bool test_lg_pos_to_string2()
{
    try {
        Xapian::LinkGrammar pos_tagger;
        unsigned pos_type_value[] = {0, 1, 2, 3, 4, 5};
        string POS[] = {"NOUN", "VERB", "ADJECTIVE", "ADVERB", "NOUNPHRASE", "none"};

        for(unsigned i = 0; i < sizeof(pos_type_value)/sizeof(unsigned); i++)
            TEST_STRINGS_EQUAL(pos_tagger.pos_to_string(pos_type_value[i], false), POS[i]);
        return true;
    } catch (const Xapian::LinkGrammarError &e) {
        return false;
    } catch (const Xapian::Error &e) {
        return false;
    } catch (...) {
        return false;
    }
}

static const char * test_sentences[] = {
    "Barack Obama is a great guy !",
    "I met the tour guide,  a notorious addict, near the waterfront.",
    "List the positions that require a knowledge of Microsoft Excel",
    "What is the Fifa ranking of Germany?",
    "Which city in South America has the largest popultation size?",
    "The red balloon soared upwards.",
    "The heavy bags are downstairs",
    "Several accidents have been reported involving passengers falling from trains",
    NULL
};

static const char * test_description_string[] = {

"Barack  ->  none\n\
Obama  ->  none\n\
is  ->  VERB\n\
a  ->  none\n\
great  ->  ADJECTIVE\n\
guy  ->  NOUN\n\
!  ->  none\n\
Barack Obama  ->  NOUNPHRASE\n\
a great guy  ->  NOUNPHRASE\n",



"I  ->  NOUN\n\
met  ->  VERB\n\
the  ->  none\n\
tour  ->  NOUN\n\
guide  ->  NOUN\n\
,  ->  none\n\
a  ->  none\n\
notorious  ->  ADJECTIVE\n\
addict  ->  NOUN\n\
,  ->  none\n\
near  ->  NOUN\n\
the  ->  none\n\
waterfront  ->  NOUN\n\
.  ->  none\n\
I  ->  NOUNPHRASE\n\
the tour guide  ->  NOUNPHRASE\n\
the waterfront  ->  NOUNPHRASE\n",



"List  ->  VERB\n\
the  ->  none\n\
positions  ->  NOUN\n\
that  ->  none\n\
require  ->  VERB\n\
a  ->  none\n\
knowledge  ->  NOUN\n\
of  ->  none\n\
Microsoft  ->  none\n\
Excel  ->  none\n\
the positions  ->  NOUNPHRASE\n\
a knowledge  ->  NOUNPHRASE\n\
Microsoft Excel  ->  NOUNPHRASE\n",



"what  ->  none\n\
is  ->  VERB\n\
the  ->  none\n\
Fifa  ->  none\n\
ranking  ->  NOUN\n\
of  ->  none\n\
Germany  ->  NOUN\n\
?  ->  none\n\
the Fifa ranking  ->  NOUNPHRASE\n\
Germany  ->  NOUNPHRASE\n",



"which  ->  none\n\
city  ->  NOUN\n\
in  ->  none\n\
South  ->  none\n\
America  ->  NOUN\n\
has  ->  VERB\n\
the  ->  none\n\
largest  ->  ADJECTIVE\n\
popultation  ->  NOUN\n\
size  ->  NOUN\n\
?  ->  none\n\
South America  ->  NOUNPHRASE\n",



"the  ->  none\n\
red  ->  ADJECTIVE\n\
balloon  ->  NOUN\n\
soared  ->  VERB\n\
upwards  ->  ADVERB\n\
.  ->  none\n\
The red balloon  ->  NOUNPHRASE\n",



"the  ->  none\n\
heavy  ->  ADJECTIVE\n\
bags  ->  NOUN\n\
are  ->  VERB\n\
downstairs  ->  none\n\
The heavy bags  ->  NOUNPHRASE\n",

"several  ->  none\n\
accidents  ->  NOUN\n\
have  ->  VERB\n\
been  ->  VERB\n\
reported  ->  VERB\n\
involving  ->  none\n\
passengers  ->  NOUN\n\
falling  ->  VERB\n\
from  ->  none\n\
trains  ->  NOUN\n\
Several accidents  ->  NOUNPHRASE\n\
reported involving passengers  ->  NOUNPHRASE\n\
trains  ->  NOUNPHRASE\n",

NULL
};

static bool test_lg_get_pos_description_string()
{
    try {
        Xapian::LinkGrammar pos_tagger("en", 20);
        string output;
        const char **p1 = test_sentences;
        const char **p2 = test_description_string;
        for (; *p1 && *p2; p1++, p2++) {
            output = pos_tagger.get_pos_description_string(*p1);
            TEST_STRINGS_EQUAL(*p2, output);
        }
    } catch (const Xapian::LinkGrammarError &e) {
        return false;
    } catch (const Xapian::Error &e) {
        return false;
    } catch (...) {
        return false;
    }
    return true;
}

static const char * test_tree_string[] = {

"(S (S (NP Barack Obama)\n\
      (VP is\n\
          (NP a great guy)))\n\
   !)\n",


"(S (NP I)\n\
   (VP met\n\
       (NP (NP the tour guide)\n\
           ,\n\
           (NP a notorious addict ,\n\
               (PP near\n\
                   (NP the waterfront))\n\
               .))))\n",


"(S (VP List\n\
       (NP (NP the positions)\n\
           (SBAR (WHNP that)\n\
                 (S (VP require\n\
                        (NP (NP a knowledge)\n\
                            (PP of\n\
                                (NP Microsoft Excel)))))))))\n",


"(S What\n\
   (S (VP is\n\
          (NP (NP the Fifa ranking)\n\
              (PP of\n\
                  (NP Germany)))))\n\
   ?)\n",


"(S Which city\n\
   (PP in\n\
       (NP South America))\n\
   (VP has\n\
       (NP the\n\
           (ADJP largest)\n\
           popultation size))\n\
   ?)\n",


"(S (NP The red balloon)\n\
   (VP soared\n\
       (ADVP upwards))\n\
   .)\n",


"(S (NP The heavy bags)\n\
   (VP are\n\
       (PP downstairs)))\n",


"(S (NP Several accidents)\n\
   (VP have\n\
       (VP been\n\
           (NP (NP reported involving passengers)\n\
               (VP falling\n\
                   (PP from\n\
                       (NP trains)))))))\n",

NULL
};

static bool test_lg_get_constituent_tree_string()
{
    try {
        Xapian::LinkGrammar pos_tagger("en", 20);
        string output;
        const char **p1 = test_sentences;
        const char **p2 = test_tree_string;
        for (; *p1 && *p2; p1++, p2++) {
            output = pos_tagger.get_constituent_tree_string(*p1);
            TEST_STRINGS_EQUAL(*p2, output);
        }
    } catch (const Xapian::LinkGrammarError &e) {
        return false;
    } catch (const Xapian::Error &e) {
        return false;
    } catch (...) {
        return false;
    }
    return true;
}

/// Test cases for the LinkGrammar.
static const test_desc tests[] = {
    TESTCASE(lg_throw_error),
    TESTCASE(lg_pos_to_string1),
    TESTCASE(lg_pos_to_string2),
    TESTCASE(lg_get_pos_description_string),
    TESTCASE(lg_get_constituent_tree_string),
    END_OF_TESTCASES
};
#endif /* HAVE_LIBLINK_GRAMMAR */

int main(int argc, char **argv)
try {
    test_driver::parse_command_line(argc, argv);

    #ifdef HAVE_LIBLINK_GRAMMAR
        return test_driver::run(tests);
    #else
        return true;
    #endif /* HAVE_LIBLINK_GRAMMAR */
} catch (const char * e) {
    cout << e << endl;
    return 1;
}
