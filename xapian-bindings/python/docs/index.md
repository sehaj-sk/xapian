% Python bindings for Xapian

The Python bindings for Xapian are packaged in the <code>xapian</code> module,
and largely follow the  <a href="http://xapian.org/docs/apidoc/html/annotated">C++ API</a>, with the following differences and
additions. Python strings and lists, etc., are converted automatically
in the bindings, so generally it should just work as expected.


The <code>examples</code> subdirectory contains examples showing how to use the
Python bindings based on the simple examples from <code>xapian-examples</code>:
<a href="examples/simpleindex.py">simpleindex.py</a>,
<a href="examples/simplesearch.py">simplesearch.py</a>,
<a href="examples/simpleexpand.py">simpleexpand.py</a>.
There's also 
<a href="examples/simplematchdecider.py">simplematchdecider.py</a>
which shows how to define a MatchDecider in Python.



The Python bindings come with a test suite, consisting of two test files:
<code>smoketest.py</code> and <code>pythontest.py</code>. These are run by the
"<code>make check</code>" command, or may be run manually.  By default, they
will display the names of any tests which failed, and then display a count of
tests which run and which failed.  The verbosity may be increased by setting
the "<code>VERBOSE</code>" environment variable: a value of 1 will display
detailed information about failures, and a value of 2 will display further
information about the progress of tests.


## Exceptions


   Xapian exceptions are translated into Python exceptions with the same names
   and inheritance hierarchy as the C++ exception classes.  The base class of
   all Xapian exceptions is the <code>xapian.Error</code> class, and this in
   turn is a child of the standard python <code>exceptions.Exception</code>
   class.


   This means that programs can trap all xapian exceptions using "<code>except
   xapian.Error</code>", and can trap all exceptions which don't indicate that
   the program should terminate using "<code>except Exception</code>".


## Unicode


   The xapian Python bindings accept unicode strings as well as simple strings
   (ie, "str" type strings) at all places in the API which accept string data.
   Any unicode strings supplied will automatically be translated into UTF-8
   simple strings before being passed to the Xapian core.  The Xapian core is
   largely agnostic about character encoding, but in those places where it does
   process data in a character encoding dependent way it assumes that the data
   is in UTF-8.  The Xapian Python bindings always return string data as simple
   strings.


   Therefore, in order to avoid issues with character encodings, you should
   always pass text data to Xapian as unicode strings, or UTF-8 encoded simple
   strings.  There is, however, no requirement for simple strings passed into
   Xapian to be valid UTF-8 encoded strings, unless they are being passed to a
   text processing routine (such as the query parser, or the stemming
   algorithms).  For example, it is perfectly valid to pass arbitrary binary
   data in a simple string to the <code>xapian.Document.set_data()</code>
   method.


   It is often useful to normalise unicode data before passing it to Xapian -
   Xapian currently has no built-in support for normalising unicode
   representations of data.  The standard python module
   "<code>unicodedata</code>" provides support for normalising unicode: you
   probably want the "<code>NFKC</code>" normalisation scheme: in other words,
   use something like

    unicodedata.normalize('NFKC', u'foo')

   to normalise the string "foo" before passing it to Xapian.


## Iterators


The iterator classes in the Xapian C++ API are wrapped in a "Pythonic" style.
The following are supported (where marked as default iterator, it means
<code>__iter__()</code> does the right
thing so you can for instance use <code>for term in document</code> to
iterate over terms in a Document object):


<table title="Python iterators">
<thead><td>Class</td><td>Method</td><td>Equivalent to</td><td>Iterator type</td></thead>
<tr><td><code>MSet</code></td><td>default iterator</td><td><code>begin()</code></td><td><code>MSetIter</code></td></tr>
<tr><td><code>ESet</code></td><td>default iterator</td><td><code>begin()</code></td><td><code>ESetIter</code></td></tr>
<tr><td><code>Enquire</code></td><td><code>matching_terms()</code></td><td><code>get_matching_terms_begin()</code></td><td><code>TermIter</code></td></tr>
<tr><td><code>Query</code></td><td>default iterator</td><td><code>get_terms_begin()</code></td><td><code>TermIter</code></td></tr>
<tr><td><code>Database</code></td><td><code>allterms()</code> (also as default iterator)</td><td><code>allterms_begin()</code></td><td><code>TermIter</code></td></tr>
<tr><td><code>Database</code></td><td><code>postlist(tname)</code></td><td><code>postlist_begin(tname)</code></td><td><code>PostingIter</code></td></tr>
<tr><td><code>Database</code></td><td><code>termlist(docid)</code></td><td><code>termlist_begin(docid)</code></td><td><code>TermIter</code></td></tr>
<tr><td><code>Database</code></td><td><code>positionlist(docid, tname)</code></td><td><code>positionlist_begin(docid, tname)</code></td><td><code>PositionIter</code></td></tr>
<tr><td><code>Database</code></td><td><code>metadata_keys(prefix)</code></td><td><code>metadata_keys_begin(prefix)</code></td><td><code>TermIter</code></td></tr>
<tr><td><code>Database</code></td><td><code>spellings()</code></td><td><code>spellings_begin(term)</code></td><td><code>TermIter</code></td></tr>
<tr><td><code>Database</code></td><td><code>synonyms(term)</code></td><td><code>synonyms_begin(term)</code></td><td><code>TermIter</code></td></tr>
<tr><td><code>Database</code></td><td><code>synonym_keys(prefix)</code></td><td><code>synonym_keys_begin(prefix)</code></td><td><code>TermIter</code></td></tr>
<tr><td><code>Document</code></td><td><code>values()</code></td><td><code>values_begin()</code></td><td><code>ValueIter</code></td></tr>
<tr><td><code>Document</code></td><td><code>termlist()</code> (also as default iterator)</td><td><code>termlist_begin()</code></td><td><code>TermIter</code></td></tr>
<tr><td><code>QueryParser</code></td><td><code>stoplist()</code></td><td><code>stoplist_begin()</code></td><td><code>TermIter</code></td></tr>
<tr><td><code>QueryParser</code></td><td><code>unstemlist(tname)</code></td><td><code>unstem_begin(tname)</code></td><td><code>TermIter</code></td></tr>
</table>


The pythonic iterators generally return Python objects, with properties
available as attribute values, with lazy evaluation where appropriate.  An
exception is the <code>PositionIter</code> object returned by
fg<code>Database.positionlist</code>, which returns an integer.



The lazy evaluation is mainly transparent, but does become visible in one situation: if you keep an object returned by an iterator, without evaluating its properties to force the lazy evaluation to happen, and then move the iterator forward, the object may no longer be able to efficiently perform the lazy evaluation.  In this situation, an exception will be raised indicating that the information requested wasn't available.  This will only happen for a few of the properties - most are either not evaluated lazily (because the underlying Xapian implementation doesn't evaluate them lazily, so there's no advantage in lazy evaluation), or can be accessed even after the iterator has moved.  The simplest work around is simply to evaluate any properties you wish to use which are affected by this before moving the iterator.  The complete set of iterator properties affected by this is:


* Database.allterms (also accessible as <code>Database.__iter__</code>): **termfreq**
* Database.termlist: **termfreq** and **positer**
* Document.termlist (also accessible as <code>Document.__iter__</code>): **termfreq** and **positer**
* Database.postlist: **positer**


In older releases, the pythonic iterators returned lists representing the
appropriate item when their <code>next()</code> method was called.  These were
removed in Xapian 1.1.0.


## Non-Pythonic Iterators


Before the pythonic iterator wrappers were added, the python bindings provided
thin wrappers around the C++ iterators.  However, these iterators don't behave
like most iterators do in Python, so the pythonic iterators were implemented to
replace them.  The non-pythonic iterators are still available to allow existing
code to continue to work, but they're now deprecated and we plan to remove them
in Xapian 1.3.0.  The documentation below is provided to aid migration away from
them.



   All non-pythonic iterators support <code>next()</code> and
   <code>equals()</code> methods
   to move through and test iterators (as for all language bindings).
   MSetIterator and ESetIterator also support <code>prev()</code>.
   Python-wrapped iterators also support direct comparison, so something like:


    m=mset.begin()
    while m!=mset.end():
      # do something
      m.next()


   C++ iterators are often dereferenced to get information, eg
   <code>(*it)</code>. With Python these are all mapped to named methods, as
   follows:


<table title="Iterator deferencing methods">
<thead><td>Iterator</td><td>Dereferencing method</td></thead>
<tr><td>PositionIterator</td>	<td><code>get_termpos()</code></td></tr>
<tr><td>PostingIterator</td>	<td><code>get_docid()</code></td></tr>
<tr><td>TermIterator</td>	<td><code>get_term()</code></td></tr>
<tr><td>ValueIterator</td>	<td><code>get_value()</code></td></tr>
<tr><td>MSetIterator</td>	<td><code>get_docid()</code></td></tr>
<tr><td>ESetIterator</td>	<td><code>get_term()</code></td></tr>
</table>


   Other methods, such as <code>MSetIterator.get_document()</code>, are
   available unchanged.


## MSet


   MSet objects have some additional methods to simplify access (these
   work using the C++ array dereferencing):


<table title="MSet additional methods">
<thead><td>Method name</td><td>Explanation</td></thead>
<tr><td><code>get_hit(index)</code></td><td>returns MSetItem at index</td></tr>
<tr><td><code>get_document_percentage(index)</code></td><td><code>convert_to_percent(get_hit(index))</code></td></tr>
<tr><td><code>get_document(index)</code></td><td><code>get_hit(index).get_document()</code></td></tr>
<tr><td><code>get_docid(index)</code></td><td><code>get_hit(index).get_docid()</code></td></tr>
</table>


Additionally, the MSet has a property, <code>mset.items</code>, which returns a
list of tuples representing the MSet; this may be more convenient than using
an MSetIter. The members of the tuple are as follows.


<table title="MSet.items member members">
<thead><td>Index</td><td>Contents</td></thead>
<tr><td><code>xapian.MSET_DID</code></td><td>Document id</td></tr>
<tr><td><code>xapian.MSET_WT</code></td><td>Weight</td></tr>
<tr><td><code>xapian.MSET_RANK</code></td><td>Rank</td></tr>
<tr><td><code>xapian.MSET_PERCENT</code></td><td>Percentage weight</td></tr>
<tr><td><code>xapian.MSET_DOCUMENT</code></td><td>Document object</td></tr>
</table>


Two MSet objects are equal if they have the same number and maximum possible
number of members, and if every document member of the first MSet exists at the
same index in the second MSet, with the same weight.


## ESet


The ESet has a property, <code>eset.items</code>, which returns a list of
tuples representing the ESet; this may be more convenient than using the
ESetIterator. The members of the tuple are as follows.


<table title="ESet.items member members">
<thead><td>Index</td><td>Contents</td></thead>
<tr><td><code>xapian.ESET_TNAME</code></td><td>Term name</td></tr>
<tr><td><code>xapian.ESET_WT</code></td><td>Weight</td></tr>
</table>

## Non-Class Functions

The C++ API contains a few non-class functions (the Database factory
functions, and some functions reporting version information), which are
wrapped like so for Python:

* <code>Xapian::version_string()</code> is wrapped as <code>xapian.version_string()</code>
* <code>Xapian::major_version()</code> is wrapped as <code>xapian.major_version()</code>
* <code>Xapian::minor_version()</code> is wrapped as <code>xapian.minor_version()</code>
* <code>Xapian::revision()</code> is wrapped as <code>xapian.revision()</code>
* <code>Xapian::Auto::open_stub()</code> is wrapped as <code>xapian.open_stub()</code>
* <code>Xapian::Chert::open()</code> is wrapped as <code>xapian.chert_open()</code>
* <code>Xapian::Flint::open()</code> is wrapped as <code>xapian.flint_open()</code>
* <code>Xapian::InMemory::open()</code> is wrapped as <code>xapian.inmemory_open()</code>
* <code>Xapian::Remote::open()</code> is wrapped as <code>xapian.remote_open()</code> (both the TCP and "program" versions are wrapped - the SWIG wrapper checks the parameter list to decide which to call).
* <code>Xapian::Remote::open_writable()</code> is wrapped as <code>xapian.remote_open_writable()</code> (both the TCP and "program" versions are wrapped - the SWIG wrapper checks the parameter list to decide which to call).

## Query


   In C++ there's a Xapian::Query constructor which takes a query operator and
   start/end iterators specifying a number of terms or queries, plus an optional
   parameter.  In Python, this is wrapped to accept any Python sequence (for
   example a list or tuple) to give the terms/queries, and you can specify
   a mixture of terms and queries if you wish.  For example:


    subq = xapian.Query(xapian.Query.OP_AND, "hello", "world")
    q = xapian.Query(xapian.Query.OP_AND,
                     [subq, "foo", xapian.Query("bar", 2)])

## MatchAll and MatchNothing

As of 1.1.1, these are wrapped as `xapian.Query.MatchAll` and `xapian.Query.MatchNothing`.

## Enquire


   There is an additional method <code>get_matching_terms()</code> which takes
   an MSetIterator and returns a list of terms in the current query which
   match the document given by that iterator.  You may find this
   more convenient than using the TermIterator directly.


## MatchDecider


Custom MatchDeciders can be created in Python; simply subclass
xapian.MatchDecider, ensure you call the super-constructor, and define a
<code>__call__</code> method that will do the work. The simplest example (which does nothing
useful) would be as follows:


    class mymatchdecider(xapian.MatchDecider):
      def __init__(self):
        xapian.MatchDecider.__init__(self)
    
      def __call__(self, doc):
        return 1

## ValueRangeProcessors


The ValueRangeProcessor class (and its subclasses) provide an operator() method
(which is exposed in python as a <code>__call__()</code> method, making the class instances
into callables).  This method checks whether a beginning and end of a range are
in a format understood by the ValueRangeProcessor, and if so, converts the
beginning and end into strings which sort appropriately.  ValueRangeProcessors
can be defined in python (and then passed to the QueryParser), or there are
several default built-in ones which can be used.



Unfortunately, in C++ the operator() method takes two std::string arguments by
reference, and returns values by modifying these arguments.  This is not
possible in Python, since strings are immutable objects.  Instead, in the
Python implementation, when the <code>__call__</code> method is called, the resulting values
of these arguments are returned as part of a tuple.  The operator() method in
C++ returns a value number; the return value of <code>__call__</code> in python consists of
a 3-tuple starting with this value number, followed by the returned "begin"
value, followed by the returned "end" value.  For example:


    vrp = xapian.NumberValueRangeProcessor(0, '$', True)
    a = '$10'
    b = '20'
    slot, a, b = vrp(a, b)


Additionally, a ValueRangeProcessor may be implemented in Python.  The Python
implementation should override the <code>__call__()</code> method with its own
implementation, and, again, since it cannot return values by reference, it
should return a tuple of (value number, begin, end).  For example:


    class MyVRP(xapian.ValueRangeProcessor):
        def __init__(self):
            xapian.ValueRangeProcessor.__init__(self)
        def __call__(self, begin, end):
            return (7, "A"+begin, "B"+end)


## Apache and mod_python/mod_wsgi

By default, both mod_python and mod_wsgi use a separate sub-interpreter for
each application.  However, Python's sub-interpreter support is incompatible
with the simplified GIL state API which SWIG-generated Python bindings use
by default.  So to avoid deadlocks, you need to tell mod_python and mod_wsgi
to run applications which use Xapian in the main interpreter as detailed below.

This restriction could be removed - the details are in Xapian's bugtracker - see
<a href="http://trac.xapian.org/ticket/364">ticket #364</a> for details of
what needs doing.

### mod_python

You need to set this option in the Apache configuration section for all
mod_python scripts which use Xapian:

    PythonInterpreter main_interpreter

You may also need to use Python <= 2.4 (due to <a
href="http://issues.apache.org/jira/browse/MODPYTHON-217">problems in Python
2.3 with the APIs the code uses</a>).

### mod_wsgi

You need to set the
<a href="http://code.google.com/p/modwsgi/wiki/ConfigurationDirectives#WSGIApplicationGroup">WSGIApplicationGroup option</a> like so:

    WSGIApplicationGroup %{GLOBAL}

The mod_wsgi documentation 
<a href="http://code.google.com/p/modwsgi/wiki/ApplicationIssues#Python_Simplified_GIL_State_API">also discusses this issue</a>.