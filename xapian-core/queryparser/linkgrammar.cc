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

#include <xapian/linkgrammar.h>
#include <xapian/types.h>
#include <xapian/error.h>

#include "linkgrammar_internal.h"

using namespace Xapian;

using namespace std;

LinkGrammar::LinkGrammar(const string & language, const int seconds)
{
    Language = language;
    Time = seconds;
}

LinkGrammar::~LinkGrammar()
{
#ifdef HAVE_LIBLINK_GRAMMAR
    internal->free_data();
#endif /* HAVE_LIBLINK_GRAMMAR */
}

list<LinkGrammar::pos_info_s>
LinkGrammar::get_pos_sentence(const string & sentence, const bool is_NP_required)
{
    list<LinkGrammar::pos_info_s> pos_info;
#ifdef HAVE_LIBLINK_GRAMMAR
    internal->initialize(Language, Time);
    internal->internal_get_pos_sentence(sentence, is_NP_required);
#endif /* HAVE_LIBLINK_GRAMMAR */
    // If linkgrammar library is not present, then return an empty list.
    return pos_info;
}


string
LinkGrammar::get_linkage_diagram_string(const string & sentence) const
{
    string diagram_string;
#ifdef HAVE_LIBLINK_GRAMMAR
    internal->initialize(Language, Time);
    diagram_string = internal->internal_get_linkage_diagram_string(sentence);
#endif /* HAVE_LIBLINK_GRAMMAR */
    // If linkgrammar library is not present, then return an empty string.
    return diagram_string;
}


string
LinkGrammar::pos_to_string(unsigned pos_type_value,
    bool null_string_for_none_pos) const
{
    string pos_string;
#ifdef HAVE_LIBLINK_GRAMMAR
    pos_string = internal->internal_pos_to_string(pos_type_value,
                                            null_string_for_none_pos);
#endif /* HAVE_LIBLINK_GRAMMAR */
    // If linkgrammar library is not present, then return an empty string.
    return pos_string;
}


string
LinkGrammar::get_pos_description_string(const string & sentence)
{
    string description_string;
#ifdef HAVE_LIBLINK_GRAMMAR
    internal->initialize(Language, Time);
    description_string = internal->internal_get_pos_description_string(sentence);
    error = internal->internal_error;
#endif /* HAVE_LIBLINK_GRAMMAR */
    // If linkgrammar library is not present, then return an empty string.
    return description_string;
}


string
LinkGrammar::get_constituent_tree_string(const string & sentence) const
{
    string tree_diagram_string;
#ifdef HAVE_LIBLINK_GRAMMAR
    internal->initialize(Language, Time);
    tree_diagram_string = internal->internal_get_pos_description_string(sentence);
#endif /* HAVE_LIBLINK_GRAMMAR */
    // If linkgrammar library is not present, then return an empty string.
    return tree_diagram_string;
}
