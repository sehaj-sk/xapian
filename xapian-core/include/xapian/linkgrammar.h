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

#include <xapian/intrusive_ptr.h>
#include <xapian/types.h>
#include <xapian/visibility.h>
#include <list>
#include <set>
#include <string>
#include <cstring>

namespace Xapian {

class XAPIAN_VISIBILITY_DEFAULT LinkGrammar {

  public:
    /// Class representing the linkgrammar internals.
    class Internal;
    /// @private @internal Reference counted internals.
    Xapian::Internal::intrusive_ptr<Internal> internal;

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

    /// Copy constructor.
    LinkGrammar(const LinkGrammar & o);

    /// Assignment.
    LinkGrammar & operator=(const LinkGrammar & o);

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

    std::string Language;

    int Time;
};

}
#endif /* XAPIAN_INCLUDED_LINKGRAMMAR_H */
