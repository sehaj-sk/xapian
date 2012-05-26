Following are some extensions that may be included in the Query syntax :

First
~~~~~~
**WHAT** : A quick way for making ANY character word_char directly in the query, without having to implicitly specify in
the queryparser code.

**WHY** : To give the user freedom to specify words cotaining characters without the need to implicitly declare them.

For example, for searching a word like::

    "file_format"

**HOW** : This corresponds to the idea which I was discussing with olly, about the escaping character. That time I think
(NOT SURE) that we did not discuss this use of escape character.

The approach shall be simple, if we detect the escape character (suppose we make ' # ' as the escaping character) then if
a character follows it then treat it as a wordchar.
This shall need the change that a character is a wordchar if ( previous_character == '#' || is_wordchar(ch)) .

For example - the query "``file#_new#.pdf``" will make the token ``TERM("file_new.pdf")``, i.e. the ``#`` shall be removed
and the character following it will be a wordchar.


Second
~~~~~~~

**WHAT** : Single Character Wildcard.

Example Query::

    xapi$n

**WHY** : The user may want to search for words which differ at one character only, (maybe he/she doesn't know how to spell a
word and is uncertain about a character :-) !)

**HOW** : Not quite sure. Maybe we can add this as a new token with two informations, one shall be the string and other will
be the position where $ occurs.
The string can be stored in ``Term->name`` and the position of occurence of $ in ``Term->pos`` (Here Term is used for the
instance of class Term corrsponding to the token).
But for that the support might have to be added in Class Query and in the searching concepts.


Third
~~~~~~

**WHAT** : Field Grouping (Boolean Filters)

Example Query::

    search_term site:(+xapian +google -yahoo)

Presently the supported query syntax for boolean filter is - "``filter:term``"

Non-Boolean filters already have such a mode.

For example - for the query "``search site:(xapian AND google)``", if we map the field "site" to prefix "T", then the query
object generated is "``Query((Zwatch@1 OR (ZTfast@2 AND ZTfood@3)))``".

**WHY** : It shall make the query syntax more user friendly. For example, if i have a large database with filter as "site"
and I want to get results of a query from some sites, s1, s2, ....., sn . In the present query syntax I shall have to do is
"``query site:s1 site:s2 site:s3``" and so on.

This approach shall ease out the syntax as follow::

    query site:(+s1 -s2 +s3) and so on.

The effect shall be even more profound when we are dealing with multiple filters.

**HOW** : This shall need change only in the lexer. When the filter is detected, and if the next character is ' ( '
then iterate till we doesn't find the character ' ) ' or we reach the end of the iterator, calling parser in between with
different tokens.

If while iterating, if we detect a '-' then call the parsrer with a token HATE and then the token BOOLEAN_FILTER with the
corresponding value.

The parser shall deal it using the grammar rule - ``prob ::= HATE BOOLEAN_FILTER.``

While we are iterating, if we detect a '+' then ignore the character and call the parser with token BOOLEAN_FILTER with
the corresponding value.

The reason being that LOVE BOOLEAN_FILTER is same as BOOLEAN_FILTER

The parser shall deal it using the grammar rule - prob ::= BOOLEAN_FILTER.

Also if neither '+' nor '-' is specified then take it to be the same as LOVE BOOLEAN_FILTER and apply the above mentioned rule.

For example - "``search_term site:(+xapian -yahoo google)``" shall be same as "``search_term site:(+xapian, -yahoo +google)``"

It maybe be better if instead of calling the parser again and again, we just store the
information about the terms and call the parser once only. But at present I don't think there is any attribute of Class
Term which provides this feature.

For this to be supported, we may store different values in whitespace separated format in the Term.name , and then do the
rest in the code associated with the
grammar rule corresponding of this.


Fourth
~~~~~~~

**WHAT** : Multiple Field Search Operator

Example Query :

    (title description):watches

This is just like the opposite of Field Grouping

**WHY** : There are many times when different fields overlap eah other. When it does so, then this shall provide a convenient
method to form queries

**HOW** : The problem here is that when we detect a ' ( ', then that will correspond to the token BRA.

Now to traverse the pointer further to see that we have a ':' (in the future characters) or not isn't a wise choice.

So probably we can add a * in front of the prefixes ! This shall look like sort of expanding, which in a way we are doing,
we are enumerating the different preixes corresonding to the term.

So the query shall look like::

    *(title description):watches

This is also consistent with the next proposed addition in which we shall have ALL Field Search Operator. There ``*:word``
means to do this for all the filters registered.

So whenever we see the character ' * ' FOLLOWED BY A WHITESPACE, we shall see whether the next character is '(' (for this
extension) or ':' (for the next extension proposed).
If it is then we shall correspondigly call the parser the number of times equal to the number of filters present in the
BRACKETS, with corresponding characters.

Again, it may be better if we could just store the information in the instance of Class State and call the parser just once
rather than calling it more than once.

Fifth
~~~~~~

**WHAT** : ALL Field Search Operator

Example Query::

    *:search_word

**WHY** : The are cases when maybe we want to search something in all the filters, like in the case if we have less and
overlaping fields.

**HOW** : The same way as mentioned above. If we detect the character ' ``*`` ', then check for the next character. If it is
' ``:`` ', then it corresponds to this case.
For this we could simply store null string in Term.unstemmed, and the string corresponding to the search word in Term.name
thus making the token corresponding to BOOL_FILTER.

Now in the rules corresponding to BOOLEAN_FILTER, we could simply check whether Term.unstemmed is empty or not. If it is,
then we shall correspondingly form the query by prefizing the search word with all the filters that have been specified.
This shall thus avoid us the pain of making a new token or anything like that.

