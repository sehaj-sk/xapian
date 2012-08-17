===================
Use of Link Grammar
===================

..  contents:: Table of contents
	:depth: 2


Introduction
============

The Link Grammar Parser is a syntactic parser of English, based on link
grammar, an original theory of English syntax.

Given a sentence, the system assigns to it a syntactic structure, which
consists of a set of labeled links connecting pairs of words.
The parser also produces a "constituent" representation of a sentence
(showing noun phrases, verb phrases, etc.).

The library is written in C.



Interface
=========

Xapian::LinkGrammar is the C++ interface that interacts with the Link Grammar
library (C library).

Given a sentence, it extracts POS (Part of Speech) of the corresponding
words of the sentence and the syntactic structure assigned to the sentence.

Xapian::LinkGrammar considers five different types of POS:

	1. Noun
	2. Verb
	3. Adjective
	4. Adverb
	5. Noun-Phrase

Noun-Phrases, usually consisting of a group of words, are also considered as
POS.

While indexing the text or parsing the query, these POS are prefixed to
their corresponding words.

Given a sentence, the list of words in the sentence, and their corresponding
POS can be recieved using::

    Xapian::LinkGrammar pos_tagger;
    list<pos_info_s> pos_info = pos_tagger.get_pos_sentence(sentence, is_NP_required);

The ``is_NP_required`` parameter is optional and defaults to true.
It tells whether you want to consider Noun-Phrases as POS or not.


Rather than recieving a list you can also recieve a string for the
visual representation of the words of the sentence and the corresponding
POS using::

    pos_tagger.get_pos_description_string(sentence);


The constituent tree depicting the breaking of sentences into Noun-Phrase,
Verb-Phrase etc. can be recieved using::

    pos_tagger.get_constituent_tree_string(sentence);


The linkage diagram (diagram contatining the links associated with the
sentence) corresponding to the setence can be recieved using::

    pos_tagger.get_linkage_diagram_string(sentence);



POS based Indexing
==================

For POS based indexing, the text is broken down into consituent sentences
using ICU sentence-break iterator class.


The sentence-break iterator locates sentence boundaries.
The exact rules used for locating each type of boundary are described
in a pair of documents from the Unicode Consortium. Unicode Standard
`Annex 14 <http://www.unicode.org/unicode/reports/tr14/>`_ gives
the rules for locating line boundaries , while `technical report 29
<http://www.unicode.org/unicode/reports/tr29/>`_ describe character, word
and sentence boundaries.


After breaking the text into sentences, the individual sentences are fed to
the Link Grammar interface to get the POS associated with different words.


The POS of the words are prefixed to them while
adding them to the list of words of the document via `add_term
<http://xapian.org/docs/sourcedoc/html/classXapian_1_1Document.html#730bb59dda98f19c61fe67360f0adb3a>`_
method.


    There are two API's to use the POS based indexing:

    - ``index_text_with_POS`` indexes the text by prefixing the POS to the words. The positional informtation is also stored.::

            Xapian::TermGenerator indexer;
            indexer.index_text_with_POS(text, is_single_sentence, wdf_inc, prefix);

    Except the *text* parameter, all other parameters are optional and defaults
    to false, 1, and empty string respectively.


    - ``index_text_with_POS_without_positions`` is same as the above method with the difference that the positional informtation is also not stored.::

            indexer.index_text_with_POS_without_positions(text, is_single_sentence, wdf_inc, prefix)


For giving the user the freedom to use their own sentence breaking algorithm
(apropriate to their specific text to be indexed), the user can set the
argument *is_single_sentence* to true in the above mentioned API methods
and call either of the methods for individual sentences.



POS based Query Parsing
=======================

For using POS based parsing of the query, the user needs to enable the
``FLAG_FREE_FORM``.

It can be done in the following manner::

	QueryParser qp;
	qp.parse_query(query, parser.FLAG_FREE_FORM);


This flag is used for unstructured query, i.e. natural language query.
These queries generally don't follow the rules of the syntax of queryparser
and are of free form.
Under this flag, only normal terms and phrased terms are considered.
Since this flag is for free form queries, hence normal rules of queryparser
like '+' denotes LOVE, '-' denotes HATE etc. don't apply here.
Thus while scanning the query, bracketes, love, hate, quotes etc. are not
treated separately.

The query is fed to the LinkGarmmar interface and the POS associated with
different words are prefixed to them.
Like in the case of indexing, here also Noun-Phrases are treated as POS.

Breaking down of words is done in the same way as done in POS based indexing.
