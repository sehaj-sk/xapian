#include <xapian.h>

#include <string>
#include <cstring>
#include <list>
#include <iostream>

using namespace std;

class emoticon_finder
{

// Emoticon has the following basic structure
// 1. Eye:     [ :;8*= ]
// 2. Subeye:  [ ,' ]
// 3. Nose:    [ -^ ]
// 4. Mouth:   [ )(][/\dDpPsScCoO#@$|}{ ]

inline bool
is_eye(unsigned ch) {
    return (ch && ch < 128 && strchr(":;8*=", ch) != NULL);
}

inline bool
is_not_eye(unsigned ch) {
    return !is_eye(ch);
}

inline bool
is_sub_eye(unsigned ch) {
    return (ch && ch < 128 && strchr(",'", ch) != NULL);
}

inline bool
is_nose(unsigned ch) {
    return (ch && ch < 128 && strchr("-^", ch) != NULL);
}

inline bool
is_mouth(unsigned ch) {
    return (ch && ch < 128 && strchr(")(][/\\dDpPsScCoO#@$|}{", ch) != NULL);
}

inline bool
is_extra_bracket(unsigned ch) {
    return (ch && ch < 128 && strchr(")", ch) != NULL);
}

  public:
    int emoticon_number;
    list<string> emoticon_list;

    string
    emoticon_extractor(const string &qs)
    {
        emoticon_number = 0;
        Xapian::Utf8Iterator it(qs), end;
        string new_qs, partial_emoticon;

    enum {
	NOT_STARTED, EYES, SUB_EYES, NOSE, MOUTH
    } emoticon_mode = NOT_STARTED;

    start:
	while (it != end && Xapian::Unicode::is_wordchar(*it))
	    Xapian::Unicode::append_utf8(new_qs, *it++);

	if (it == end) {
	    return new_qs;
	}

        while (it != end && !Xapian::Unicode::is_wordchar(*it) && !Xapian::Unicode::is_whitespace(*it) && is_not_eye(*it))
            Xapian::Unicode::append_utf8(new_qs, *it++);
        
        if(Xapian::Unicode::is_wordchar(*it)) {
            goto start;
        }
        
        // Assuming that either whitespace or any non_word_character(s)
        // is must before the emoticon
        if (!Xapian::Unicode::is_whitespace(*it) && is_not_eye(*it)) {
            goto start;
        }  else if (Xapian::Unicode::is_whitespace(*it)) {
	    // Multiple whitespace.
	    do {
	        Xapian::Unicode::append_utf8(new_qs, *it++);
	    } while (it != end && Xapian::Unicode::is_whitespace(*it));
	} else {
	    // To make sure that in query like "http://" , ":/" is not treated
	    // as an emoticon
	    if (Xapian::Unicode::is_wordchar(int(new_qs[new_qs.size()-1]))) {
	        do {
	            Xapian::Unicode::append_utf8(new_qs, *it++);
	        } while (it != end && Xapian::Unicode::is_wordchar(*it));
	        goto start;
	    }
	    emoticon_mode = EYES;
	    Xapian::Unicode::append_utf8(partial_emoticon, *it++);
        }

        if(emoticon_mode != EYES) {
            while (it != end && !Xapian::Unicode::is_wordchar(*it) && is_not_eye(*it))
                Xapian::Unicode::append_utf8(new_qs, *it++);
        }
            
        if (it == end) {
            new_qs.append(partial_emoticon);
            return new_qs;
        }
        
        if (Xapian::Unicode::is_wordchar(*it)) {
            new_qs.append(partial_emoticon);
            partial_emoticon.resize(0);
            emoticon_mode = NOT_STARTED;
            goto start;
        }
        // Eyes are necessary in an emoticon
        if(emoticon_mode != EYES) {
            emoticon_mode = EYES;
            Xapian::Unicode::append_utf8(partial_emoticon, *it++);
        }
        // Sub Eyes are not necessary in an emoticon
        if (is_sub_eye(*it)) {
            emoticon_mode = SUB_EYES;
            Xapian::Unicode::append_utf8(partial_emoticon, *it++);
        }
        // Nose is not necessary in an emoticon
        if (is_nose(*it)) {
            emoticon_mode = NOSE;
            Xapian::Unicode::append_utf8(partial_emoticon, *it++);
        }
        // Mouth is necessary in an emoticon
        if (is_mouth(*it)) {
            emoticon_mode = MOUTH;
            Xapian::Unicode::append_utf8(partial_emoticon, *it++);
            // For possible ')' at the end of emoticon whih may be used to
            // represent degree of happiness or sadness or any other feeling
            while(it != end && is_extra_bracket(*it))
                Xapian::Unicode::append_utf8(partial_emoticon, *it++);
        }
        
        // Assuming that just after the emoticon, there should be no
        // wordchar. This shall prevent from detecting the cases that shouldn't
        // be treated as emoticons, for example, query like "new :Parser"
        // should not treat ":P" as emoticon.
        if (emoticon_mode == MOUTH && (it == end || !Xapian::Unicode::is_wordchar(*it))) {
            new_qs.append(" ");
            ++emoticon_number;
            emoticon_list.push_back(partial_emoticon);
        } else {
            new_qs.append(partial_emoticon);
        }
        
        if (it == end) {
            return new_qs;
        }
        
        emoticon_mode = NOT_STARTED;
        partial_emoticon.resize(0);
        goto start;
        
    }

};

int
main(int argc, char **argv)
{
    emoticon_finder extractor;

    if (argc > 1) {
        cout << "Input String given: " << argv[1] << "\n";
        cout << "New string after extracting emoticons: " << extractor.emoticon_extractor(argv[1]) << "\n";
        cout << "Number of emoticons present: " << extractor.emoticon_number << "\n";
        list<string>::iterator it_emoticon;
        cout << "List of emoticon(s) present: ";
        for ( it_emoticon = extractor.emoticon_list.begin() ; it_emoticon != extractor.emoticon_list.end(); it_emoticon++ )
            cout << " " << *it_emoticon;
        cout << "\n";
    } else {
        cout << "Please enter a string from which emoticon needs to be exctracted. \n";
    }
}


/* A Few Sample Testcases

Input String given: Hey Hi! :-) Wassup :P
New string after extracting emoticons: Hey Hi!   Wassup
Number of emoticons present: 2
List of emoticon(s) present:  :-) :P

Input String given: linux humor :)
New string after extracting emoticons: linux humor  
Number of emoticons present: 1
List of emoticon(s) present:  :)

Input String given: Het wordt pas echt leuk als het hard staatclear >:)
New string after extracting emoticons: Het wordt pas echt leuk als het hard staatclear > 
Number of emoticons present: 1
List of emoticon(s) present:  :)

Input String given: he said that....:-)))
New string after extracting emoticons: he said that.... 
Number of emoticons present: 1
List of emoticon(s) present:  :-)))

Input String given: http://www.xapian.org is cool =)
New string after extracting emoticons: http://www.xapian.org is cool  
Number of emoticons present: 1
List of emoticon(s) present:  =)

Input String given: Euro 2012 Arrives :'-)
New string after extracting emoticons: Euro 2012 Arrives  
Number of emoticons present: 1
List of emoticon(s) present:  :'-)

Input String given: fbml has been deprecated ;-[
New string after extracting emoticons: fbml has been deprecated  
Number of emoticons present: 1
List of emoticon(s) present:  ;-[

*/
