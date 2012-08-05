/** @file termgenerator_internal.cc
 * @brief TermGenerator class internals
 */
/* Copyright (C) 2007,2010,2011,2012 Olly Betts
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

#include "termgenerator_internal.h"

#include <xapian/document.h>
#include <xapian/queryparser.h>
#include <xapian/unicode.h>
#include <xapian/linkgrammar.h>
#include <xapian/error.h>

#include "stringutils.h"

#include <limits>
#include <string>
#include <cstring>

#include "cjk-tokenizer.h"

#ifdef HAVE_ICUUC
#include <unicode/brkiter.h>
#endif

using namespace std;

namespace Xapian {

inline bool
U_isupper(unsigned ch) {
    return (ch < 128 && C_isupper((unsigned char)ch));
}

inline unsigned check_wordchar(unsigned ch) {
    if (Unicode::is_wordchar(ch)) return Unicode::tolower(ch);
    return 0;
}

inline bool
is_wordchar_whitespace(unsigned ch) {
    return Unicode::is_whitespace(ch) || Unicode::is_wordchar(ch);
}

inline bool
is_nonwordchar_whitespace(unsigned ch) {
    return Unicode::is_whitespace(ch) || !Unicode::is_wordchar(ch);
}

inline bool
should_stem(const std::string & term)
{
    const unsigned int SHOULD_STEM_MASK =
	(1 << Unicode::LOWERCASE_LETTER) |
	(1 << Unicode::TITLECASE_LETTER) |
	(1 << Unicode::MODIFIER_LETTER) |
	(1 << Unicode::OTHER_LETTER);
    Utf8Iterator u(term);
    return ((SHOULD_STEM_MASK >> Unicode::get_category(*u)) & 1);
}

/** Value representing "ignore this" when returned by check_infix() or
 *  check_infix_digit().
 */
const unsigned UNICODE_IGNORE = numeric_limits<unsigned>::max();

inline unsigned check_infix(unsigned ch) {
    if (ch == '\'' || ch == '&' || ch == 0xb7 || ch == 0x5f4 || ch == 0x2027) {
	// Unicode includes all these except '&' in its word boundary rules,
	// as well as 0x2019 (which we handle below) and ':' (for Swedish
	// apparently, but we ignore this for now as it's problematic in
	// real world cases).
	return ch;
    }
    // 0x2019 is Unicode apostrophe and single closing quote.
    // 0x201b is Unicode single opening quote with the tail rising.
    if (ch == 0x2019 || ch == 0x201b) return '\'';
    if (ch >= 0x200b && (ch <= 0x200d || ch == 0x2060 || ch == 0xfeff))
	return UNICODE_IGNORE;
    return 0;
}

inline unsigned check_infix_digit(unsigned ch) {
    // This list of characters comes from Unicode's word identifying algorithm.
    switch (ch) {
	case ',':
	case '.':
	case ';':
	case 0x037e: // GREEK QUESTION MARK
	case 0x0589: // ARMENIAN FULL STOP
	case 0x060D: // ARABIC DATE SEPARATOR
	case 0x07F8: // NKO COMMA
	case 0x2044: // FRACTION SLASH
	case 0xFE10: // PRESENTATION FORM FOR VERTICAL COMMA
	case 0xFE13: // PRESENTATION FORM FOR VERTICAL COLON
	case 0xFE14: // PRESENTATION FORM FOR VERTICAL SEMICOLON
	    return ch;
    }
    if (ch >= 0x200b && (ch <= 0x200d || ch == 0x2060 || ch == 0xfeff))
	return UNICODE_IGNORE;
    return 0;
}

inline bool
is_digit(unsigned ch) {
    return (Unicode::get_category(ch) == Unicode::DECIMAL_DIGIT_NUMBER);
}

inline unsigned check_suffix(unsigned ch) {
    if (ch == '+' || ch == '#') return ch;
    // FIXME: what about '-'?
    return 0;
}

// FIXME: add API for this:
#define STOPWORDS_NONE 0
#define STOPWORDS_IGNORE 1
#define STOPWORDS_INDEX_UNSTEMMED_ONLY 2

void
TermGenerator::Internal::index_text(Utf8Iterator itor, termcount wdf_inc,
				    const string & prefix, bool with_positions)
{
    bool cjk_ngram = CJK::is_cjk_enabled();

    int stop_mode = STOPWORDS_INDEX_UNSTEMMED_ONLY;

    if (!stopper) stop_mode = STOPWORDS_NONE;

    while (true) {
	// Advance to the start of the next term.
	unsigned ch;
	while (true) {
	    if (itor == Utf8Iterator()) return;
	    ch = check_wordchar(*itor);
	    if (ch) break;
	    ++itor;
	}

	string term;
	// Look for initials separated by '.' (e.g. P.T.O., U.N.C.L.E).
	// Don't worry if there's a trailing '.' or not.
	if (U_isupper(*itor)) {
	    const Utf8Iterator end;
	    Utf8Iterator p = itor;
	    do {
		Unicode::append_utf8(term, Unicode::tolower(*p++));
	    } while (p != end && *p == '.' && ++p != end && U_isupper(*p));
	    // One letter does not make an acronym!  If we handled a single
	    // uppercase letter here, we wouldn't catch M&S below.
	    if (term.size() > 1) {
		// Check there's not a (lower case) letter or digit
		// immediately after it.
		if (p == end || !Unicode::is_wordchar(*p)) {
		    itor = p;
		    goto endofterm;
		}
	    }
	    term.resize(0);
	}

	while (true) {
	    if (cjk_ngram && CJK::codepoint_is_cjk(*itor)) {
		const string & cjk = CJK::get_cjk(itor);
		for (CJKTokenIterator tk(cjk); tk != CJKTokenIterator(); ++tk) {
		    const string & cjk_token = *tk;
		    if (cjk_token.size() > max_word_length) continue;

		    if (stop_mode == STOPWORDS_IGNORE && (*stopper)(cjk_token))
			continue;

		    if (strategy == TermGenerator::STEM_SOME ||
			strategy == TermGenerator::STEM_NONE) {
			if (with_positions && tk.get_length() == 1) {
			    doc.add_posting(prefix + cjk_token, ++termpos, wdf_inc);
			} else {
			    doc.add_term(prefix + cjk_token, wdf_inc);
			}
		    }

		    if ((flags & FLAG_SPELLING) && prefix.empty())
			db.add_spelling(cjk_token);

		    if (strategy == TermGenerator::STEM_NONE ||
			!stemmer.internal.get()) continue;

		    if (strategy == TermGenerator::STEM_SOME) {
			if (stop_mode == STOPWORDS_INDEX_UNSTEMMED_ONLY &&
			    (*stopper)(cjk_token))
			    continue;

			// Note, this uses the lowercased term, but that's OK
			// as we only want to avoid stemming terms starting
			// with a digit.
			if (!should_stem(cjk_token)) continue;
		    }

		    // Add stemmed form without positional information.
		    string stem;
		    if (strategy != TermGenerator::STEM_ALL) {
			stem += "Z";
		    }
		    stem += prefix;
		    stem += stemmer(cjk_token);
		    if (strategy != TermGenerator::STEM_SOME &&
			with_positions) {
			doc.add_posting(stem, ++termpos, wdf_inc);
		    } else {
			doc.add_term(stem, wdf_inc);
		    }
		}
		while (true) {
		    if (itor == Utf8Iterator()) return;
		    ch = check_wordchar(*itor);
		    if (ch) break;
		    ++itor;
		}
	    }
	    unsigned prevch;
	    do {
		Unicode::append_utf8(term, ch);
		prevch = ch;
		if (++itor == Utf8Iterator() ||
		    (cjk_ngram && CJK::codepoint_is_cjk(*itor)))
		    goto endofterm;
		ch = check_wordchar(*itor);
	    } while (ch);

	    Utf8Iterator next(itor);
	    ++next;
	    if (next == Utf8Iterator()) break;
	    unsigned nextch = check_wordchar(*next);
	    if (!nextch) break;
	    unsigned infix_ch = *itor;
	    if (is_digit(prevch) && is_digit(*next)) {
		infix_ch = check_infix_digit(infix_ch);
	    } else {
		// Handle things like '&' in AT&T, apostrophes, etc.
		infix_ch = check_infix(infix_ch);
	    }
	    if (!infix_ch) break;
	    if (infix_ch != UNICODE_IGNORE)
		Unicode::append_utf8(term, infix_ch);
	    ch = nextch;
	    itor = next;
	}

	{
	    size_t len = term.size();
	    unsigned count = 0;
	    while ((ch = check_suffix(*itor))) {
		if (++count > 3) {
		    term.resize(len);
		    break;
		}
		Unicode::append_utf8(term, ch);
		if (++itor == Utf8Iterator()) goto endofterm;
	    }
	    // Don't index fish+chips as fish+ chips.
	    if (Unicode::is_wordchar(*itor))
		term.resize(len);
	}

endofterm:
	if (term.size() > max_word_length) continue;

	if (stop_mode == STOPWORDS_IGNORE && (*stopper)(term)) continue;

	if (strategy == TermGenerator::STEM_SOME ||
	    strategy == TermGenerator::STEM_NONE) {
	    if (with_positions) {
		doc.add_posting(prefix + term, ++termpos, wdf_inc);
	    } else {
		doc.add_term(prefix + term, wdf_inc);
	    }
	}
	if ((flags & FLAG_SPELLING) && prefix.empty()) db.add_spelling(term);

	if (strategy == TermGenerator::STEM_NONE ||
	    !stemmer.internal.get()) continue;

	if (strategy == TermGenerator::STEM_SOME) {
	    if (stop_mode == STOPWORDS_INDEX_UNSTEMMED_ONLY && (*stopper)(term))
		continue;

	    // Note, this uses the lowercased term, but that's OK as we only
	    // want to avoid stemming terms starting with a digit.
	    if (!should_stem(term)) continue;
	}

	// Add stemmed form without positional information.
	string stem;
	if (strategy != TermGenerator::STEM_ALL) {
	    stem += "Z";
	}
	stem += prefix;
	stem += stemmer(term);
	if (strategy != TermGenerator::STEM_SOME &&
	    with_positions) {
	    doc.add_posting(stem, ++termpos, wdf_inc);
	} else {
	    doc.add_term(stem, wdf_inc);
	}
    }
}

#ifdef HAVE_LIBLINK_GRAMMAR

void
TermGenerator::Internal::index_sentence_with_POS(const string & sentence,
                    termcount wdf_inc, const string & prefix,
                    bool with_positions)
{
    int stop_mode = STOPWORDS_INDEX_UNSTEMMED_ONLY;
    if (!stopper)
        stop_mode = STOPWORDS_NONE;

    LinkGrammar pos_tagger;
    list<LinkGrammar::pos_info_s> pos_info;
    pos_info = pos_tagger.get_pos_sentence(sentence);

    // If the sentence fails to get tokenized or parsed, then the list
    // returned by get_pos_sentence() shall be empty.
    // Under such cases, simply use the index_text() for indexing that
    // sentence.
    if (pos_info.empty()) {
        index_text(Utf8Iterator(sentence), wdf_inc, prefix, with_positions);
        return;
    }

    string word, pos, temp_word;
    Utf8Iterator checker;
    Utf8Iterator end;
    bool is_nonwordchar_inbetween;

    for (list<LinkGrammar::pos_info_s>::iterator it = pos_info.begin();
            it != pos_info.end(); it++) {
        temp_word.resize(0);
        is_nonwordchar_inbetween = false;
        word = it->word;
        pos = pos_tagger.pos_to_string(it->pos);

        if (pos.compare("NOUNPHRASE") != 0 &&
                word.size() > max_word_length)
            continue;

        if (stop_mode == STOPWORDS_IGNORE && (*stopper)(word))
            continue;

        checker = Utf8Iterator(word);
        while (checker != end) {
            while (checker != end && !is_nonwordchar_whitespace(*checker)) {
                Unicode::append_utf8(temp_word, Unicode::tolower(*checker++));
            }
            if (checker == end)
                break;
            if (Unicode::is_whitespace(*checker)) {
                // In case of Noun Phrase, there can be group of words.
                // Under such cases, replace the whitespace between
                // those words with '#'. This makes them appear as
                // a single entity in the term list of the document.
                if (++checker != end && !temp_word.empty())
                    Unicode::append_utf8(temp_word,'#');
            } else {
                // At this instant, iterator currently points to a non-wordchar.
                Utf8Iterator p = find_if(checker, end, is_wordchar_whitespace);
                if (p == end)
                    break;
                if (Unicode::is_whitespace(*p)) {
                    if (++p != end && !temp_word.empty())
                        Unicode::append_utf8(temp_word,'#');
                } else {
                    // It means that there are non-word characters between
                    // two words without a whitespace. (such as "good..boy")
                    // Under this case, if it's not a Noun Phrase, then just
                    // call index_text() for this particular word.
                    // If it's a Noun Phrase, then replace the non-wordchar(s)
                    // with '#'. This will be in agreement with replacement of
                    // whitespace with '#' for Noun Phrase.
                    // FIXME: Is this approach right or not ?
                    if (pos.compare("NOUNPHRASE") != 0) {
                        is_nonwordchar_inbetween = true;
                        break;
                    } else {
                        Unicode::append_utf8(temp_word,'#');
                    }
                }
                checker = p;
            }
        }

        if (is_nonwordchar_inbetween) {
            index_text(Utf8Iterator(word), wdf_inc, prefix, with_positions );
            continue;
        }

        // Ensure that there are no trailing '-' caused due to replacing
        // whitespace with '-' for the case of Noun Phrase.
        while (temp_word[temp_word.length() - 1] == '#')
            temp_word.erase(temp_word.length() - 1);

        if (temp_word.empty())
            continue;

        word = temp_word;
        if (strategy == TermGenerator::STEM_SOME ||
                strategy == TermGenerator::STEM_NONE) {
            if (with_positions) {
                doc.add_posting(prefix + pos + word, ++termpos, wdf_inc);
            } else {
                doc.add_term(prefix + pos + word, wdf_inc);
            }
        }
        if (pos.compare("NOUNPHRASE") == 0 )
            continue;

        if ((flags & FLAG_SPELLING) && prefix.empty())
            db.add_spelling(word);

        if (strategy == TermGenerator::STEM_NONE ||
                !stemmer.internal.get())
            continue;

        if (strategy == TermGenerator::STEM_SOME) {
            if (stop_mode == STOPWORDS_INDEX_UNSTEMMED_ONLY &&
                    (*stopper)(word))
                continue;
            if (!should_stem(word))
                continue;
        }

        // Add stemmed form without positional information.
        string stem;
        if (strategy != TermGenerator::STEM_ALL) {
            stem += "Z";
        }
        stem += prefix;
        stem += pos;
        stem += stemmer(word);
        if (strategy != TermGenerator::STEM_SOME && with_positions) {
            doc.add_posting(stem, ++termpos, wdf_inc);
        } else {
            doc.add_term(stem, wdf_inc);
        }
    }
}

void
TermGenerator::Internal::index_text_with_POS(const string & text,
                    termcount wdf_inc, const string & prefix,
                    bool with_positions)
{
    #ifndef HAVE_ICUUC
        throw TermGeneratorError("ICU breakiterator header file not found.");
    #else
        UnicodeString text_u = UnicodeString(text.c_str());
        UErrorCode status = U_ZERO_ERROR;
        BreakIterator * sentence_breaker;
        sentence_breaker = BreakIterator::createSentenceInstance(Locale::getUS(),
                                                                    status);
        if (U_FAILURE(status)) {
            throw TermGeneratorError("Failed to create sentence break iterator.");
        }
        sentence_breaker->setText(text_u);
        int32_t start_sentence = sentence_breaker->first();
        int32_t end_sentence, length_sentence;
        char * sentence;
        for (end_sentence = sentence_breaker->next();
                end_sentence != BreakIterator::DONE;
                start_sentence = end_sentence,
                end_sentence = sentence_breaker->next()) {
            length_sentence = end_sentence - start_sentence;
            sentence = new char[length_sentence + 1];
            length_sentence = text_u.extract(start_sentence, length_sentence,
                                                sentence, "");
            index_sentence_with_POS(sentence, wdf_inc, prefix, with_positions);
            delete [] sentence;
        }
    #endif /* HAVE_ICUUC */
}
#endif /* HAVE_LIBLINK_GRAMMAR */
}
