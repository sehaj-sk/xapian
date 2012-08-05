/** @file termgenerator_internal.h
 * @brief TermGenerator class internals
 */
/* Copyright (C) 2007,2012 Olly Betts
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

#ifndef XAPIAN_INCLUDED_TERMGENERATOR_INTERNAL_H
#define XAPIAN_INCLUDED_TERMGENERATOR_INTERNAL_H

#include "xapian/intrusive_ptr.h"
#include <xapian/database.h>
#include <xapian/document.h>
#include <xapian/termgenerator.h>
#include <xapian/stem.h>
#include <config.h>

namespace Xapian {

class Stopper;

class TermGenerator::Internal : public Xapian::Internal::intrusive_base {
    friend class TermGenerator;
    Stem stemmer;
    stem_strategy strategy;
    const Stopper * stopper;
    Document doc;
    termcount termpos;
    TermGenerator::flags flags;
    unsigned max_word_length;
    WritableDatabase db;

  public:
    Internal() : strategy(STEM_SOME), stopper(NULL), termpos(0),
	flags(TermGenerator::flags(0)), max_word_length(64) { }
    void index_text(Utf8Iterator itor,
		    termcount weight,
		    const std::string & prefix,
		    bool with_positions);

    #ifdef HAVE_LIBLINK_GRAMMAR

    void index_sentence_with_POS(const std::string & sentence,
            termcount weight,
            const std::string & prefix,
            bool with_positions);

    void index_text_with_POS(const std::string & text,
            termcount weight,
            const std::string & prefix,
            bool with_positions);

    #endif /* HAVE_LIBLINK_GRAMMAR */
};

}

#endif // XAPIAN_INCLUDED_TERMGENERATOR_INTERNAL_H
