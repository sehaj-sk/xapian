/** @file linkgrammar_internal.cc
 * @brief LinkGrammar class internals
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

// Ensure that if Link Grammar is not installed then compiling doesn't fail.
#include <config.h>

#include "linkgrammar_internal.h"

#include <link-grammar/link-includes.h>
#include <xapian/linkgrammar.h>
#include <xapian/error.h>
#include "omassert.h"
#include "constituents.h"
#include <locale.h>
#include <sstream>
#include <set>

using namespace Xapian;

using namespace std;

LinkGrammar::Internal::Internal() {}

LinkGrammar::Internal::~Internal() {}

#ifdef HAVE_LIBLINK_GRAMMAR
void
LinkGrammar::Internal::initialize(const string & language, const int seconds)
{
    // Since Link Grammar is written in C, hence it accepts char * rather
    // than std::string.
    const char * language_ = language.c_str();
    setlocale(LC_ALL, "");
    internal_Opts = parse_options_create();
    // This routine controls the level of description printed to stderr/stdout
    // about the parsing process.
    parse_options_set_verbosity(internal_Opts, 0);
    parse_options_set_min_null_count(internal_Opts, 0);
    parse_options_set_max_null_count(internal_Opts, 2);
    internal_Dict = dictionary_create_lang(language_);
    if (!internal_Dict) {
        internal_error = "Unable to open the dictionary.";
        throw LinkGrammarError(internal_error);
    }
    parse_options_set_max_parse_time(internal_Opts, seconds);

    // Meanings of subscript according to what is mentioned in the
    // documentation at
    // http://www.abisource.com/projects/link-grammar/dict/introduction.html
    // For complete list, please refer to that documentation.

    /********************************************************************
     **  Subscript     ****         Meaning                            **
     ********************************************************************
     *    m            ****   Given names that are always masculine     *
     *    f            ****   Given names that are always feminine      *
     *    b            ****   Given names that can be masculine or      *
     *                        feminine                                  *
     *    l            ****   Location (cities, states, towns, etc.)    *
     *    s            ****   US state names and abbreviations          *
     *    n            ****   Noun                                      *
     *    n-u          ****   Noun, uncountable (mass noun)             *
     *    p            ****   Plural count nouns                        *
     *    s            ****   Singular, mass or count nouns             *
     *    o            ****   Organizations (corporations)              *
     ********************************************************************/
    internal_Noun.insert("m");
    internal_Noun.insert("f");
    internal_Noun.insert("b");
    internal_Noun.insert("l");
    internal_Noun.insert("s");
    internal_Noun.insert("n");
    internal_Noun.insert("n-u");
    internal_Noun.insert("p");
    internal_Noun.insert("s");
    internal_Noun.insert("o");

    /********************************************************************
     **  Subscript     ****         Meaning                            **
     ********************************************************************
     *    v            ****   Verb                                      *
     *    v-d          ****   Verb, past tense                          *
     *    w            ****   Verb                                      *
     *    w-d          ****   Verb, past tense                          *
     *    q            ****   Verb, Question-related or paraphrasing    *
     *    q-d          ****   Verb, past tense                          *
     ********************************************************************/
    internal_Verb.insert("v");
    internal_Verb.insert("v-d");
    internal_Verb.insert("w");
    internal_Verb.insert("w-d");
    internal_Verb.insert("q");
    internal_Verb.insert("q-d");

    /********************************************************************
     **  Subscript     ****         Meaning                            **
     ********************************************************************
     *    a            ****   Adjective                                 *
     *    a-c          ****   Adjective, comparative/relative           *
     *    a-s          ****   Adjective, superlative                    *
     ********************************************************************/
    internal_Adjective.insert("a");
    internal_Adjective.insert("a-c");
    internal_Adjective.insert("a-s");

    /********************************************************************
     **  Subscript     ****         Meaning                            **
     ********************************************************************
     *    e            ****   Adverbs                                   *
     ********************************************************************/
    internal_Adverb.insert("e");
}


list<LinkGrammar::pos_info_s>
LinkGrammar::Internal::internal_get_pos_sentence(const string & sentence,
                            const bool is_NP_required)
{
    const char * sentence_ = sentence.c_str();
    const char * sentence_word;
    const char * linkage_word;
    Sentence sent;
    Linkage linkage;
    string subscript;
    pos_type POS;
    list<LinkGrammar::pos_info_s> pos_info;

    // This routine takes the input string and tokenizes it using the word
    // definitions in the Dictionary passed in.
    // FIXME: Above is what the Link Grammar documentation present at
    // http://www.abisource.com/projects/link-grammar/dict/introduction.html
    // says. But by looking at the source code, it appears that this routine
    // just allocates the memory and doesn't do any tokenization. Tokenization
    // is actually done by sentence_split() routine.
    sent = sentence_create(sentence_, internal_Dict);
    if (!sent) {
        internal_error = "Unable to tokenize the sentence";
        // Don't exit or throw an exception, just return an empty list.
        return pos_info;
    }

    // This routine splits, or tokenizes, the sentence into its component
    // words and punctuations. This routine returns zero if successful;
    // else a non-zero value if an error occured.
    if (sentence_split(sent, internal_Opts)) {
        internal_error = "Unable to split the sentence";
        // Don't exit or throw an exception, just return an empty list.
        sentence_delete(sent);
        return pos_info;
    }

    // This routine searches for possible linkages.
    if (!sentence_parse(sent, internal_Opts)) {
        internal_error = "Unable to parse the sentence";
        // Don't exit or throw an exception, just return an empty list.
        sentence_delete(sent);
        return pos_info;
    }

    linkage = linkage_create(0, sent, internal_Opts);
    for (int i = 0; i < linkage_get_num_words(linkage); i++) {
        sentence_word = sentence_get_word(sent, i);

        if (strcmp(sentence_word, "LEFT-WALL") == 0 ||
                strcmp(sentence_word, "RIGHT-WALL") == 0) {
            continue;
        }

        linkage_word = linkage_get_word(linkage, i);
        subscript = get_subscript(linkage_word, sentence_word);

        if ((subscript[0] == '!' || subscript[0] == '?') &&
                subscript.length() > 3) {
            subscript.erase(0, 3);
        }

        POS = get_pos_from_subscript(subscript);
        pos_info.push_back(pos_info_s(sentence_word, POS));
    }

    if (is_NP_required) {
        list<string> list_NP = get_NP(linkage);
        for (list<string>::iterator it = list_NP.begin();
            it != list_NP.end(); it++ ) {
            pos_info.push_back(pos_info_s(*it, NOUNPHRASE));
        }
    }

    sentence_delete(sent);
    linkage_delete(linkage);
    return pos_info;
}


string
LinkGrammar::Internal::internal_get_linkage_diagram_string(const string & sentence) const
{
    Linkage linkage;
    string diagram_string;

    linkage = set_default_linkage(sentence);
    if (linkage) {
        char * diagram = linkage_print_diagram(linkage);
        diagram_string = diagram;
        linkage_free_diagram(diagram);
        linkage_delete(linkage);
    }
    return diagram_string;
}


string
LinkGrammar::Internal::internal_pos_to_string(unsigned pos_type_value,
    bool null_string_for_none_pos) const
{
    switch(pos_type_value) {
        case 0:
            return "NOUN";

        case 1:
            return "VERB";

        case 2:
            return "ADJECTIVE";

        case 3:
            return "ADVERB";

        case 4:
            return "NOUNPHRASE";

        case 5:
            if (null_string_for_none_pos)
                return "";
            else
                return "none";
    }
    // Should not reach here. Since at present the enum pos_type has just
    // six members.
    Assert(false);
    return "";
}


string
LinkGrammar::Internal::internal_get_pos_description_string(const string & sentence)
{
    ostringstream pos_description;
    list<LinkGrammar::pos_info_s> pos_info;
    pos_info = internal_get_pos_sentence(sentence);
    if (pos_info.empty())
        return pos_description.str();

    list<LinkGrammar::pos_info_s>::iterator it;
    for (it = pos_info.begin(); it != pos_info.end(); it++) {
        pos_description << it->word << "  ->  ";
        pos_description << internal_pos_to_string(it->pos, false) << endl;
    }
    return pos_description.str();
}


string
LinkGrammar::Internal::internal_get_constituent_tree_string(const string & sentence) const
{
    Linkage linkage;
    string tree_diagram_string;
    linkage = set_default_linkage(sentence);
    if (linkage) {
        char * diagram = linkage_print_constituent_tree(linkage, 1);
        tree_diagram_string = diagram;
        linkage_free_constituent_tree_str(diagram);
        linkage_delete(linkage);
    }
    return tree_diagram_string;
}

void
LinkGrammar::Internal::free_data()
{
    if (internal_Dict)
        dictionary_delete(internal_Dict);

    if (internal_Opts)
        parse_options_delete(internal_Opts);
}

Linkage
LinkGrammar::Internal::set_default_linkage(const string & sentence) const
{
    Sentence sent;
    const char * sentence_ = sentence.c_str();

    sent = sentence_create(sentence_, internal_Dict);
    if (!sent)
        return NULL;

    if (sentence_split(sent, internal_Opts))
        return NULL;

    if (!sentence_parse(sent, internal_Opts))
        return NULL;

    return linkage_create(0, sent, internal_Opts);
}


const char *
LinkGrammar::Internal::get_subscript(const char * linkage_word,
    const char * sentence_word) const
{
    if (strlen(linkage_word) > strlen(sentence_word)) {
        return linkage_word + strlen(sentence_word) + 1;
    } else {
        return "NoSubscript";
    }
}


LinkGrammar::pos_type
LinkGrammar::Internal::get_pos_from_subscript(const string subscript) const
{
    if (internal_Noun.find(subscript) != internal_Noun.end())
        return NOUN;

    if (internal_Verb.find(subscript) != internal_Verb.end())
        return VERB;

    if (internal_Adjective.find(subscript) != internal_Adjective.end())
        return ADJECTIVE;

    if (internal_Adverb.find(subscript) != internal_Adverb.end())
        return ADVERB;

    return NONE;
}


list<string>
LinkGrammar::Internal::get_NP(Linkage linkage) const
{
    list<string> list_NP;
    CNode * root;
    root = linkage_constituent_tree(linkage);
    if (root == NULL)
        return list_NP;
    list_NP = traverse_for_NP(list_NP, root);
    linkage_free_constituent_tree(root);
    return list_NP;
}


list<string>
LinkGrammar::Internal::traverse_for_NP(list<string> list_NP,
                        CNode * current_root) const
{
    CNode * current_node;
    bool is_NP = false;
    string current_NP;
    if (current_root == NULL)
        return list_NP;

    if (strcmp(current_root->label, "NP")==0 && is_NP_leaf(current_root))
        is_NP = true;

    for (current_node = current_root->child; current_node != NULL;
        current_node = current_node->next) {
        if(is_NP) {
            if (!current_NP.empty()) {
                current_NP += ' ';
            }
            current_NP += current_node->label;
        } else if (current_node->child != NULL) {
            Assert(!is_NP);
            list_NP = traverse_for_NP(list_NP, current_node);
        }
    }
    if (is_NP) {
        is_NP = false;
        list_NP.push_back(current_NP);
    }
    return list_NP;
}


bool
LinkGrammar::Internal::is_NP_leaf(CNode *root_of_subtree) const
{
    CNode * iterate_children_list;
    for (iterate_children_list = root_of_subtree->child;
            iterate_children_list != NULL;
            iterate_children_list = iterate_children_list->next) {
        if (iterate_children_list->child != NULL)
            return false;
    }
    return true;
}

#endif /* HAVE_LIBLINK_GRAMMAR */
