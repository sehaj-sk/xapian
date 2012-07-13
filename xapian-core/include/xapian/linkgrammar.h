/** @file linkgrammar.h
 * @brief Uses Link Grammar to provide POS while indexing
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

#ifndef XAPIAN_INCLUDED_LINKGRAMMAR_H
#define XAPIAN_INCLUDED_LINKGRAMMAR_H

// Ensure that if Link Grammar is not installed then compiling doesn't fail.
#include <config.h>

#ifdef HAVE_LIBLINK_GRAMMAR

// link-includes.h provides the API interface for using Link Grammar
#include <link-grammar/link-includes.h>
#include <xapian/visibility.h>
#include <list>
#include <string>
#include <cstring>

namespace Xapian {

class XAPIAN_VISIBILITY_DEFAULT LinkGrammar {

  public:
    /// Categories of POS. NONE signifies no POS found.
    enum pos_type { NOUN, VERB, ADJECTIVE, ADVERB, NOUNPHRASE, NONE };

    /** Information about a word and its POS. */
    struct pos_info_s {
        /// The word whose POS we are trying to find.
        /// In case of NOUNPHRASE, there can be group of words.
        std::string word;

        /// The POS of the word.
        pos_type pos;

        /// Constructor.
        pos_info_s(const std::string & word_, pos_type pos_)
        : word(word_), pos(pos_) { }
    };

    const char * error;

    /** Constructor.
     *
     *  @param language     The language for the dictionary. If the
     *          corresponding dictionary is not found, then an error is thrown.
     *          [default: "en", meaning english]
     *  @param seconds      Determines the approximate maximum time that
     *          parsing is allowed to take.
     *          [default: 5 seconds]
     */
    LinkGrammar(const std::string & language = "en", const int seconds = 5);

    /// Destructor.
    ~LinkGrammar();

    /** Finds POS for the given sentence.
     *
     *  @param sentence         The sentence for which POS are to be found.
     *  @param is_NP_required   Should Noun Pharse(s) be also found in the
     *          sentence?
     *          [default: true]
     */
    std::list<pos_info_s> get_pos_sentence(const std::string & sentence,
                                const bool is_NP_required = true);

    /** Gives the linkage diagram, prodced by Link Grammar for the given
     *  sentence.
     *
     *  @param sentence     The sentence for which the linkage diagram is
                required.
     */
    std::string get_linkage_diagram_string(const std:: string & sentence) const;

    /** Given the enum pos_type, it returns the corresponding string.
     *
     *  It is basically a mapping from enum pos_type to correspondng string.
     *  For Example, for pos_type NOUN, the string returned shall be "NOUN".
     *
     *  @param pos_type_value           The member of enum pos_type for which
     *          the corresponding string is required.
     *
     *  @param null_string_for_none_pos For the NONE member of enum pos_type,
     *          should a null string be returned or the string "none" should be
     *          returned.
     *          [default: true, meaning return null string for NONE]
     */
    std::string pos_to_string(const unsigned pos_type_value,
                    bool null_string_for_none_pos = true) const;

    /** Gives a string containing the information of the words of the given
     *  sentence and their associated POS.
     *
     *  @param sentence     The sentence for which the POS information
     *          containing string is required.
     */
    std::string get_pos_description_string(const std::string & sentence);

    /** Gives the string contating constituent tree for the given sentence. The
     *  tree depicts the breaking of the sentence into Noun Phrase, Verb
     *  Phrase etc.
     *
     *  @param sentence     The sentence for which the string containing
     *          constituent tree is requred.
     */
    std::string get_constituent_tree_string(const std::string & sentence) const;

  private:
    /// A Dictionary is the programmer's handle on the set of word definitions
    /// that defines the grammar. A user creates a Dictionary from a grammar
    /// file and post-process knowledge file, and then passes it to the various
    /// parsing routines.
    Dictionary Dict;

    /// Parse_Options specify the different parameters that are used to
    /// parse sentences. Examples of the kinds of things that are controlled by
    /// Parse_Options include maximum parsing time and memory, whether to use
    /// null-links, and whether or not to use "panic" mode. This data structure
    /// is passed in to the various parsing and printing routines along with
    /// the sentence.
    Parse_Options Opts;

    /** Given a sentence, it tokenizes it and returns the corresponding linkage.
     *
     *  @param sentence     The sentence for which the linkage is to be found.
     *           Linkage is the Link Grammar API's representation of a parse.
     *           A Linkage can be constructed after a sentence has been parsed,
     *           and can be thought of as a Sentence together with a collection
     *           of links.
     */
    Linkage set_default_linkage(const std::string & sentence) const;

    /** Finds the subscript of the required word.
     *
     *  The subscripts are used to identify the POS associated with the
     *  word. For Example, In the sentence, "I was playing", the Link Grammar
     *  associates the word "playing" to "playing.v". v is the subscript here,
     *  meaning that the word "playing" is a verb.
     *
     *  @param linkage_word     It refers to the word associated with the
     *          linkage. The subscript is present in it along with the
     *          original word.
     *          (Example: "playing.v")
     *  @param sentence_word    It refers to the original word present in the
     *          sentence. Hence the subscript is not present in it.
     *          (Example: "playing")
     */
    const char * get_subscript(const char * linkage_word,
                    const char * sentence_word) const;

    /** Finds POS from the given subcript.
     *
     *  It's basically a mapping of subscipt to pos_type.
     *
     * @param subscript     The subscript from which the POS is to be found.
     */
    pos_type get_pos_from_subscript(const std::string subscript) const;

    /** Finds the Noun Phrases present in the sentence.
     *
     *  It returns a list of the strings corresponding to Noun Phrase.
     *
     * @param linkage   The linkage from which the Noun Phrases have to be
     *          extracted.
     */
    std::list<std::string> get_NP(Linkage linkage) const;

    /** A recursive tree traversal for finding the strings corresponding to
     *  Noun Phrase.
     *
     *  Used by get_NP() method.
     *
     *  @param list_NP          The list which shall be storing the strings
     *          associated with the Noun Phrase.
     *  @param current_root     The pointer to the current root of the subtree.
     */
    std::list<std::string> traverse_for_NP(std::list<std::string> list_NP,
                                CNode * current_root) const;

    /** Checks if the Noun Phrase is present at the leaf of the Tree.
     *
     *  This prevents us from taking into consideration those Noun Phrases which
     *  are further broken down into Noun Phrase, Verb Phrase etc. For Example,
     *  consider the following sentence:
     *
     *  @code
     *  Grammar is useless because there is nothing to say -- Gertrude Stein.
     *  @endcode
     *
     *  This produces the following structure:
     *
     *  @code
     *  (S (NP Grammar)
     *     (VP is
     *         (ADJP useless))
     *     (SBAR because
     *           (S (NP there)
     *              (VP is
     *                  (NP (NP (NP nothing)
     *                          (SBAR (WHNP to)
     *                                (VP say)))
     *                  (NP -- Gertrude Stein .))))))
     *  @endcode
     *
     * In the above structure, we would want to treat "-- Gertrude Stein" as a
     * Noun Phrase since it is at the leaf of this tree, but we don't want to
     * treat "nothing to say" as a Noun Phrase since it's further broken down
     * into Noun Phrase (NP) and SBAR.
     * The conventions used here (NP, SBAR, WHNP etc.) by Link Grammar are
     * those of the Penn Treebank.
     *
     * @param root_of_subtree   The pointer to root of subtree where Noun Phrase
     *          has been detected.
     */
    bool is_NP_leaf(CNode * root_of_subtree) const;
};

}
#endif /* HAVE_LIBLINK_GRAMMAR */
#endif /* XAPIAN_INCLUDED_LINKGRAMMAR_H */
