/********************************************************************************/
/* Copyright (c) 2004                                                           */
/* Daniel Sleator, David Temperley, and John Lafferty                           */
/* All rights reserved                                                          */
/*                                                                              */
/* Use of the link grammar parsing system is subject to the terms of the        */
/* license set forth in the LICENSE file included with this software,           */ 
/* and also available at http://www.link.cs.cmu.edu/link/license.html           */
/* This license allows free redistribution and use in source and binary         */
/* forms, with or without modification, subject to certain conditions.          */
/*                                                                              */
/********************************************************************************/

// Modified to check for presence of Link Grammar library.  Sehaj Singh Kalra 2012-07-11

#ifndef _CONSTITUENTSH_
#define _CONSTITUENTSH_

// Ensure that if Link Grammar is not installed then compiling doesn't fail.
#include <config.h>
#if HAVE_LIBLINK_GRAMMAR

#include <link-grammar/link-includes.h>

/* Invariant: Leaf if child==NULL */
struct CNode_s {
  char  * label;
  CNode * child;
  CNode * next;
  int   start, end;
};

#endif /* HAVE_LIBLINK_GRAMMAR */
#endif /* _CONSTITUENTSH_ */
