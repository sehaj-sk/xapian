/** @file pos_index.cc
 * @brief Index a single sentence as a Xapian document using POS support
 *  from Link Grammar. Also show the linkage diagram, pos extracted for the
 *  words of the sentence and the structure of the constituent tree produced
 *  for the given sentence.
 */
/* Copyright (C) 2012 Sehaj Singh Kalra
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

#include <xapian.h>
#include <config.h>

#include <string>
#include <cstdlib> // For exit().
#include <cstring>
#include <iostream>

using namespace std;

int
main(int argc, char **argv)
try {
    if (argc != 2 || argv[1][0] == '-') {
        int rc = 1;
        if (argv[1]) {
            if (strcmp(argv[1], "--version") == 0) {
            cout << "pos_index" << endl;
            exit(0);
            }
            if (strcmp(argv[1], "--help") == 0) {
            rc = 0;
            }
        }
        cout << "Usage: " << argv[0] << " PATH_TO_DATABASE\n"
            "Index a single sentence given by the user as a Xapian document using POS support from Link Grammar.\n"
            "Also show the linkage diagram, pos extracted for the words of the sentence and the structure of the constituent tree produced for the given sentence." << endl;
        exit(rc);
    }

#ifndef HAVE_LIBLINK_GRAMMAR
cout << "Link Grammar library not found\n";
exit(1);
#else

    // Open the database for update, creating a new database if necessary.
    Xapian::WritableDatabase db(argv[1], Xapian::DB_CREATE_OR_OPEN);

    Xapian::TermGenerator indexer;
    Xapian::Stem stemmer("english");
    indexer.set_stemmer(stemmer);
    string line;
    getline(cin, line);

    Xapian::Document doc;
    doc.set_data(line);

    indexer.set_document(doc);
    indexer.index_text_with_POS(line);

    // Add the document to the database.
    db.add_document(doc);

    Xapian::LinkGrammar pos_tagger;

    cout << "The Linkage Diagram for the given sentence:\n";
    cout << pos_tagger.get_linkage_diagram_string(line) << "\n";

    cout << "The words and their corresponding POS are:\n";
    cout << pos_tagger.get_pos_description_string(line) << "\n";

    cout << "The constituent tree structure is:\n";
    cout << pos_tagger.get_constituent_tree_string(line) << "\n";

    // Explicitly commit so that we get to see any errors.  WritableDatabase's
    // destructor will commit implicitly (unless we're in a transaction) but
    // will swallow any exceptions produced.
    db.commit();

#endif /* HAVE_LIBLINK_GRAMMAR */
} catch (const Xapian::Error &e) {
    cout << e.get_description() << endl;
    exit(1);
}

/************************   Sample Testcases  ********************************
*                                                                            *
*   Sample Sentence 1.:                                                      *
*   Barack Obama is a great guy !                                            *
*                                                                            *
*   Output Corresponding to this sentence:                                   *
*                                                                            *
*   The Linkage Diagram for the given sentence:                              *
*                                                                            *
*       +-------------------Xp------------------+                            *
*       |                    +------Ost-----+   |                            *
*       +------Wd------+     |  +-----Ds----+   |                            *
*       |        +--G--+--Ss-+  |    +---A--+   |                            *
*       |        |     |     |  |    |      |   |                            *
*   LEFT-WALL Barack Obama is.v a great.a guy.n !                            *
*                                                                            *
*                                                                            *
*   The words and their corresponding POS are:                               *
*   Barack  ->  none                                                         *
*   Obama  ->  none                                                          *
*   is  ->  VERB                                                             *
*   a  ->  none                                                              *
*   great  ->  ADJECTIVE                                                     *
*   guy  ->  NOUN                                                            *
*   !  ->  none                                                              *
*   Barack Obama  ->  NOUNPHRASE                                             *
*   a great guy  ->  NOUNPHRASE                                              *
*                                                                            *
*                                                                            *
*   The constituent tree structure is:                                       *
*   (S (S (NP Barack Obama)                                                  *
*         (VP is                                                             *
*             (NP a great guy)))                                             *
*      !)                                                                    *
*                                                                            *
*                                                                            *
*   The output of "delve" on the corresponding document of the specified     *
*   database is as follow (document number in your database may vary):       *
*                                                                            *
*   Data for record #1:                                                      *
*   Barack Obama is a great guy !                                            *
*   Term List for record #1: ADJECTIVEgreat NOUNPHRASEa#great#guy            *
*   NOUNPHRASEbarack#obama NOUNguy VERBis ZADJECTIVEgreat ZNOUNguy ZVERBis   *
*   Za Zbarack Zobama a barack obama                                         *
*                                                                            *
*                                                                            *
******************************************************************************
*                                                                            *
*                                                                            *
*   Sample Sentence 2.:                                                      *
*   I met the tour guide,  a notorious addict, near the waterfront.          *
*                                                                            *
*   Output Corresponding to this sentence:                                   *
*                                                                            *
*   The Linkage Diagram for the given sentence:                              *
*                                                                            *
*                             +-----------MXs----------+--------------Xc-    *
*          +--------Os--------+    +---------Xd--------+         +-------    *
*          |     +-----Ds-----+    | +--------Ds-------+---MXsx--+------J    *
*    +-Sp*i+     |     +--AN--+    | |      +-----A----+    +-Xd-+    +--    *
*    |     |     |     |      |    | |      |          |    |    |    |      *
*   I.p met.v-d the tour.n guide.n , a notorious.a addict.n , near.p the     *
*                                                                            *
*   -------------+                                                           *
*   --Xca--------+                                                           *
*   s-----+      |                                                           *
*   -Ds---+      |                                                           *
*         |      |                                                           *
*   waterfront.n .                                                           *
*                                                                            *
*                                                                            *
*   The words and their corresponding POS are:                               *
*   I  ->  NOUN                                                              *
*   met  ->  VERB                                                            *
*   the  ->  none                                                            *
*   tour  ->  NOUN                                                           *
*   guide  ->  NOUN                                                          *
*   ,  ->  none                                                              *
*   a  ->  none                                                              *
*   notorious  ->  ADJECTIVE                                                 *
*   addict  ->  NOUN                                                         *
*   ,  ->  none                                                              *
*   near  ->  NOUN                                                           *
*   the  ->  none                                                            *
*   waterfront  ->  NOUN                                                     *
*   .  ->  none                                                              *
*   I  ->  NOUNPHRASE                                                        *
*   the tour guide  ->  NOUNPHRASE                                           *
*   the waterfront  ->  NOUNPHRASE                                           *
*                                                                            *
*                                                                            *
*   The constituent tree structure is:                                       *
*   (S (NP I)                                                                *
*      (VP met                                                               *
*          (NP (NP the tour guide)                                           *
*              ,                                                             *
*              (NP a notorious addict ,                                      *
*                  (PP near                                                  *
*                      (NP the waterfront))                                  *
*                  .))))                                                     *
*                                                                            *
*                                                                            *
*   The output of "delve" on the corresponding document of the specified     *
*   database is as follow (document number in your database may vary):       *
*                                                                            *
*   Data for record #2:                                                      *
*   I met the tour guide,  a notorious addict, near the waterfront.          *
*   Term List for record #2: ADJECTIVEnotorious NOUNPHRASEi                  *
*   NOUNPHRASEthe#tour#guide NOUNPHRASEthe#waterfront NOUNaddict NOUNguide   *
*   NOUNi NOUNnear NOUNtour NOUNwaterfront VERBmet ZADJECTIVEnotori          *
*   ZNOUNaddict ZNOUNguid ZNOUNi ZNOUNnear ZNOUNtour ZNOUNwaterfront         *
*   ZVERBmet Za Zthe a the                                                   *
*                                                                            *
*****************************************************************************/
