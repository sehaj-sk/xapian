/* expand.h: class for finding expand terms
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

#ifndef _expand_h_
#define _expand_h_

#include "database.h"
#include "termlist.h"
#include "omenquire.h"

#include <queue>
#include <stack>
#include <vector>

class OMExpandDecider {
    public:
	virtual bool want_term(const om_termname & tname) const = 0;
};

class OMExpandDeciderAlways : public virtual OMExpandDecider {
    public:
	bool want_term(const om_termname & tname) const { return true; }
};

class OMExpand {
    private:
        IRDatabase *database;
   
        bool recalculate_maxweight;
	TermList * build_tree(const RSet *rset, const OMExpandWeight *ewt);
    public:
        OMExpand(IRDatabase * database_);

	void expand(om_termcount max_esize,
		    OMESet & eset,
		    const RSet * rset,
		    const OMExpandDecider * decider);
};

inline OMExpand::OMExpand(IRDatabase * database_)
	: database(database_)
{ return; }

#endif /* _expand_h_ */
