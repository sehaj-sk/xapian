/* da_database.h: C++ class definition for DA access routines
 *
 * ----START-LICENCE----
 * Copyright 1999,2000 Dialog Corporation
 * 
 * This program is free software; you can redistribute it and/or 
 * modify it under the terms of the GNU General Public License as 
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
 * USA
 * -----END-LICENCE-----
 */

#ifndef _da_database_h_
#define _da_database_h_

#include <map>
#include <vector>

#include "utils.h"
#include "omassert.h"

#include "dbpostlist.h"
#include "termlist.h"
#include "database.h"

#include "errno.h"

// Anonymous declarations (definitions in daread.h)
struct postings;
struct DAfile;
struct terminfo;
struct termvec;

// But it turns out we need to include this anyway
// FIXME - try and sort this out sometime.
#include "daread.h"

class DAPostList : public virtual DBPostList {
    friend class DADatabase;
    private:
	struct postings * postlist;
	om_docid  currdoc;

	om_termname tname;
	om_doccount termfreq;

	DAPostList(const om_termname & tname_,
		   struct postings * postlist_,
		   om_doccount termfreq_);
    public:
	~DAPostList();

	om_doccount get_termfreq() const;

	om_docid  get_docid() const;     // Gets current docid
	om_weight get_weight() const;    // Gets current weight
	PostList *next(om_weight w_min);          // Moves to next docid
	PostList *skip_to(om_docid did, om_weight w_min);  // Moves to next docid >= specified docid
	bool   at_end() const;        // True if we're off the end of the list

	string intro_term_description() const;
};

inline om_doccount
DAPostList::get_termfreq() const
{
    return termfreq;
}

inline om_docid
DAPostList::get_docid() const
{
    Assert(!at_end());
    Assert(currdoc != 0);
    return currdoc;
}

inline bool
DAPostList::at_end() const
{
    Assert(currdoc != 0);
    if (currdoc == MAXINT) return true;
    return false;
}

inline string
DAPostList::intro_term_description() const
{
    return tname + ":" + inttostring(termfreq);
}



class DATermListItem {
    public:
	om_termname tname;
	om_termcount wdf;
	om_doccount termfreq;

	DATermListItem(om_termname tname_,
		       om_termcount wdf_,
		       om_doccount termfreq_)
		: tname(tname_),
		  wdf(wdf_),
		  termfreq(termfreq_)
	{ return; }
};
 
class DATermList : public virtual DBTermList {
    friend class DADatabase;
    private:
	vector<DATermListItem>::iterator pos;
	vector<DATermListItem> terms;
	bool have_started;
	om_doccount dbsize;

	DATermList(struct termvec *tv, om_doccount dbsize_);
    public:
	om_termcount get_approx_size() const;

	OMExpandBits get_weighting() const; // Gets weight info of current term
	const om_termname get_termname() const;
	om_termcount get_wdf() const; // Number of occurences of term in current doc
	om_doccount get_termfreq() const;  // Number of docs indexed by term
	TermList * next();
	bool   at_end() const;
};

inline om_termcount DATermList::get_approx_size() const
{
    return terms.size();
}

inline const om_termname DATermList::get_termname() const
{
    Assert(!at_end());
    Assert(have_started);
    return pos->tname;
}

inline om_termcount DATermList::get_wdf() const
{
    Assert(!at_end());
    Assert(have_started);
    return pos->wdf;
}

inline om_doccount DATermList::get_termfreq() const
{
    Assert(!at_end());
    Assert(have_started);
    return pos->termfreq;
}

inline TermList * DATermList::next()
{
    if(have_started) {
	Assert(!at_end());
	pos++;
    } else {
	have_started = true;
    }
    return NULL;
}

inline bool DATermList::at_end() const
{
    Assert(have_started);
    if(pos == terms.end()) {
	DebugMsg("TERMLIST " << this << " ENDED " << endl);
	return true;
    }
    return false;
}




class DATerm {
    friend DADatabase;
    private:
	DATerm(struct terminfo * ti_,
	       om_termname tname_,
	       struct DAfile * DA_t_ = NULL);
        struct terminfo * get_ti() const;

	mutable bool terminfo_initialised;
        mutable struct terminfo ti;
        mutable struct DAfile * DA_t;
    public:
	om_termname tname;
};

inline
DATerm::DATerm(struct terminfo * ti_,
	       om_termname tname_,
	       struct DAfile * DA_t_)
	: terminfo_initialised(false)
{
    if (ti_) {
	ti = *ti_;
	terminfo_initialised = true;
    }
    tname = tname_;
    DA_t = DA_t_;
}

inline struct terminfo *
DATerm::get_ti() const
{
    if (!terminfo_initialised) {
	DebugMsg("Getting terminfo" << endl);
	int len = tname.length();
	if(len > 255) abort();
	byte * k = (byte *) malloc(len + 1);
	if(k == NULL) throw bad_alloc();
	k[0] = len + 1;
	tname.copy((char*)(k + 1), len);

	int found = DAterm(k, &ti, DA_t);
	free(k);

	if(found == 0) abort();
	terminfo_initialised = true;
    }
    return &ti;
}

class DADatabase : public virtual IRDatabase {
    friend class DatabaseBuilder;
    friend class DADocument;
    private:
	bool   opened;
	struct DAfile * DA_r;
	struct DAfile * DA_t;

	mutable map<om_termname, DATerm> termmap;

	// Stop copy / assignment being allowed
	DADatabase& operator=(const DADatabase&);
	DADatabase(const DADatabase&);

	// Look up term in database
	const DATerm * term_lookup(const om_termname & tname) const;

	// Get a record
	struct record * get_record(om_docid did) const;

	DADatabase();
	void open(const DatabaseBuilderParams & params);
    public:
	~DADatabase();

	om_doccount  get_doccount() const;
	om_doclength get_avlength() const;

	om_doccount get_termfreq(const om_termname & tname) const;
	bool term_exists(const om_termname & tname) const;

	DBPostList * open_post_list(const om_termname & tname, RSet * rset) const;
	DBTermList * open_term_list(om_docid did) const;
	IRDocument * open_document(om_docid did) const;

	void make_term(const om_termname & tname) {
	    throw OmUnimplemented("DADatabase::make_term() not implemented");
	}
	om_docid make_doc(const om_docname & ) {
	    throw OmUnimplemented("DADatabase::make_doc() not implemented");
	}
	void make_posting(const om_termname &, unsigned int, unsigned int) {
	    throw OmUnimplemented("DADatabase::make_posting() not implemented");
	}
};

inline om_doccount
DADatabase::get_doccount() const
{
    Assert(opened);
    return DA_r->itemcount;
}

inline om_doclength
DADatabase::get_avlength() const
{
    Assert(opened);
    return 1;
}

inline om_doccount
DADatabase::get_termfreq(const om_termname & tname) const
{
    PostList *pl = open_post_list(tname, NULL);
    om_doccount freq = 0;
    if(pl) freq = pl->get_termfreq();
    delete pl;
    return freq;
}

#endif /* _da_database_h_ */
