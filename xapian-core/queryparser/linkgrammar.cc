/** @file linkgrammar.cc
 * @brief LinkGrammar class implementation
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
#ifdef HAVE_LIBLINK_GRAMMAR

#include <xapian/linkgrammar.h>
#include "omassert.h"
#include "constituents.h"
#include <locale.h>
#include <sstream>

using namespace Xapian;

using namespace std;

// Meanings of subscript according to what is mentioned in the
// documentation at
// http://www.abisource.com/projects/link-grammar/dict/introduction.html
// For complete list, please refer to that documentation.

/********************************************************************
 **  Subscript     ****         Meaning                            **
 ********************************************************************
 *    n            ****   Noun                                      *
 *    n-u          ****   Noun, uncountable (mass noun)             *
 *    p            ****   Plural count nouns                        *
 *    s            ****   Singular, mass or count nouns             *
 *    o            ****   Organizations (corporations)              *
 ********************************************************************/
static const string Noun = "n n-u p s o";

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
static const string Verb = "v v-d w w-d q q-d";

/********************************************************************
 **  Subscript     ****         Meaning                            **
 ********************************************************************
 *    a            ****   Adjective                                 *
 *    a-c          ****   Adjective, comparative/relative           *
 *    a-s          ****   Adjective, superlative                    *
 ********************************************************************/
static const string Adjective = "a a-c a-s";

/********************************************************************
 **  Subscript     ****         Meaning                            **
 ********************************************************************
 *    e            ****   Adverbs                                   *
 ********************************************************************/
static const string Adverb = "e";

LinkGrammar::LinkGrammar(const string & language, const int seconds)
{
    // Since Link Grammar is written in C, hence it accepts char * rather
    // than std::string.
    const char * language_ = language.c_str();
    setlocale(LC_ALL, "");
    Opts = parse_options_create();
    // This routine controls the level of description printed to stderr/stdout
    // about the parsing process.
    parse_options_set_verbosity(Opts, 0);
    parse_options_set_min_null_count(Opts, 0);
    parse_options_set_max_null_count(Opts, 2);
    Dict = dictionary_create_lang(language_);
    if (!Dict) {
        error = "Unable to open the dictionary";
        // FIXME: Incorporate this error into Xapian's error handling.
        // I presently am not able to figure out as to how to add the errors,
        // since error.h is a generated file !
        throw error;
    }
    parse_options_set_max_parse_time(Opts, seconds);
}

LinkGrammar::~LinkGrammar()
{
    if (Dict)
        dictionary_delete(Dict);
    if (Opts)
        parse_options_delete(Opts);
}

list<LinkGrammar::pos_info_s>
LinkGrammar::get_pos_sentence(const string & sentence, const bool is_NP_required)
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
    sent = sentence_create(sentence_, Dict);
    if (!sent) {
        error = "Unable to tokenize the sentence";
        // Don't exit or throw an exception, just return an empty list.
        return pos_info;
    }

    // This routine splits, or tokenizes, the sentence into its component
    // words and punctuations. This routine returns zero if successful;
    // else a non-zero value if an error occured.
    if (sentence_split(sent, Opts)) {
        error = "Unable to split the sentence";
        // Don't exit or throw an exception, just return an empty list.
        sentence_delete(sent);
        return pos_info;
    }

    // This routine searches for possible linkages.
    if (!sentence_parse(sent, Opts)) {
        error = "Unable to parse the sentence";
        // Don't exit or throw an exception, just return an empty list.
        sentence_delete(sent);
        return pos_info;
    }

    linkage = linkage_create(0, sent, Opts);
    for (int i = 0; i < linkage_get_num_words(linkage); i++) {
        sentence_word = sentence_get_word(sent, i);

        if (strcmp(sentence_word, "LEFT-WALL") == 0 ||
                strcmp(sentence_word, "RIGHT-WALL") == 0)
            continue;

        linkage_word = linkage_get_word(linkage, i);
        subscript = get_subscript(linkage_word, sentence_word);

        if ((subscript[0] == '!' || subscript[0] == '?') &&
                subscript.length() > 3)
            subscript.erase(0, 3);

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
LinkGrammar::get_linkage_diagram_string(const string & sentence) const
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
LinkGrammar::pos_to_string(unsigned pos_type_value,
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
LinkGrammar::get_pos_description_string(const string & sentence)
{
    ostringstream pos_description;
    list<LinkGrammar::pos_info_s> pos_info;
    pos_info = get_pos_sentence(sentence);
    if (pos_info.empty())
        return pos_description.str();

    list<LinkGrammar::pos_info_s>::iterator it;
    for (it = pos_info.begin(); it != pos_info.end(); it++) {
        pos_description << it->word << "  ->  ";
        pos_description << pos_to_string(it->pos, false) << "\n";
    }
    return pos_description.str();
}


string
LinkGrammar::get_constituent_tree_string(const string & sentence) const
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


Linkage
LinkGrammar::set_default_linkage(const string & sentence) const
{
    Sentence sent;
    const char * sentence_ = sentence.c_str();

    sent = sentence_create(sentence_, Dict);
    if (!sent)
        return NULL;

    if (sentence_split(sent, Opts))
        return NULL;

    if (!sentence_parse(sent, Opts))
        return NULL;

    return linkage_create(0, sent, Opts);
}


string
LinkGrammar::get_subscript(const char * linkage_word,
    const char * sentence_word)
{
    if (strlen(linkage_word) > strlen(sentence_word)) {
        return linkage_word + strlen(sentence_word) + 1;
    } else {
        return "NoSubscript";
    }
}


LinkGrammar::pos_type
LinkGrammar::get_pos_from_subscript(const string subscript)
{
    size_t found;

    if ((found = Noun.find(subscript)) != string::npos)
        return NOUN;

    if ((found = Verb.find(subscript)) != string::npos)
        return VERB;

    if ((found = Adjective.find(subscript)) != string::npos)
        return ADJECTIVE;

    if ((found = Adverb.find(subscript)) != string::npos)
        return ADVERB;

    return NONE;
}


list<string>
LinkGrammar::get_NP(Linkage linkage)
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
LinkGrammar::traverse_for_NP(list<string> list_NP, CNode * current_root)
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
LinkGrammar::is_NP_leaf(CNode *root_of_subtree)
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
