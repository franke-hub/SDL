//----------------------------------------------------------------------------
//
//       Copyright (c) 2013 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       Elc.h **** PRELIMINARY ****
//
// Purpose-
//       English Language Compiler. *NOT IMPLEMENTED*
//
// Last change date-
//       2013/01/01
//
//----------------------------------------------------------------------------
#ifndef ELC_H_INCLUDED
#define ELC_H_INCLUDED

#include <string>

//----------------------------------------------------------------------------
//
// Class-
//       Elc_Sentence
//
// Purpose-
//       Elc sentence structure.
//
// Need SOM- (like DOM)
//       Paragraph
//         \CompoundSentence.
//           Sentence, Sentence.
//             \Subject   \Subject
//              Verb       Verb
//              Object     Object
//         \Sentence.
//         \Sentence.
//           \Subject Verb Object.
//             \The quick brown fox
//                     \jumped over
//                          \the lazy dog.
//             \art. adj. adj. noun
//                     \verb(past tense) prep.
//                          \art. adj. noun
//
// Compounder-
//       In: I went to the store and bought ice cream.
//       Out: Conjunction: AND
//           I went to the store
//           (I) bought ice cream
//
//----------------------------------------------------------------------------
class Elc_Sentence {                // Elc sentence structure
//----------------------------------------------------------------------------
// Elc_Sentence::Constructors
//----------------------------------------------------------------------------
public:
virtual
   ~Elc_Sentence( void );           // Destructor
   Elc_Sentence(                    // Constructor
     std::string*      word_list);  // List of words ending with NULL

//----------------------------------------------------------------------------
// Elc_Sentence::Accessors
//----------------------------------------------------------------------------
public:
//----------------------------------------------------------------------------
//
// Method-
//       hasImplicitSubject
//
// Purpose-
//       TRUE iff the subject phrase for a sentence is implicit
//
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
//
// Method-
//       hasImplicitObject
//
// Purpose-
//       TRUE iff the object phrase for a sentence is implicit
//
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
//
// Method-
//       hasCompound
//
// Purpose-
//       TRUE iff the sentence has compound elements
//
// Example-
//       I went to the store and (then) bought ice cream.
//       I went to the store, (and/then) bought ice cream.
//
//----------------------------------------------------------------------------

public:
//----------------------------------------------------------------------------
//
// Method-
//       getSubject
//
// Purpose-
//       Return the subject phrase for a sentence
//
//----------------------------------------------------------------------------
std::string*                        // List of words ending with NULL
   getSubject(                      // Return the subject phrase for a sentence
     int               index= 0);   // The index (for compound sentences)

//----------------------------------------------------------------------------
//
// Method-
//       getVerb
//
// Purpose-
//       Return the verb phrase for a sentence
//
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
//
// Method-
//       getObject
//
// Purpose-
//       Return the object phrase for a sentence
//
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
//
// Method-
//       getRootWord
//
// Purpose-
//       Return the root word for each word in a sentence
//
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
//
// Method-
//       getRootType
//
// Purpose-
//       Return the root word type for each word in a sentence
//
// Resultant: One of-
//       ARTICLE                    // Article (a, an, the)
//       NOUN                       // Noun
//       VERB                       // Verb
//       CONJUCTION                 // Conjunction (and)
//       PREPOSITION                // Preposition
//       PRONOUN                    // Pronoun
//       ADJECTIVE                  // Adjective
//       ADVERB                     // Adverb
//
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
//
// Method-
//       getWordType
//
// Purpose-
//       Return word type for each word in a sentence (as used)
//
//----------------------------------------------------------------------------
}; // class Elc_Sentence

//----------------------------------------------------------------------------
//
// Class-
//       Elc
//
// Purpose-
//       Elc stuff.
//
//----------------------------------------------------------------------------
class Elc {                         // Elc stuff
//----------------------------------------------------------------------------
// Elc::Attributes
//----------------------------------------------------------------------------
public:
enum Sentence_Element               // Parts of a sentence
{  INVALID_SENTENCE_ELEMENT= 0      // (For internal consistency checking)
,  SUBJECT                          // The subject of the sentence
,  VERB_PHRASE                      // The verb phrase
,  OBJECT                           // The object for the sentence
};

enum Part_of_Speech                 // Parts of speech
{  INVALID_PART_OF_SPEECH= 0        // (For internal consistency checking)
,  ARTICLE                          // Article (a, an, the)
,  NOUN                             // Noun
,  VERB                             // Verb
,  PREPOSITION                      // Preposition
,  INDEFINITE_ARTICLE               // (a, an)
,  PRONOUN                          // Pronoun
,  ADJECTIVE                        // Adjective
,  ADVERB                           // Adverb
};

//----------------------------------------------------------------------------
// Elc::Constructors
//----------------------------------------------------------------------------
public:
virtual
   ~Elc( void );                    // Destructor
   Elc( void );                     // (Default) Constructor

private:                            // Disallowed:
   Elc(const Elc&);                 // Copy constructor
Elc&
   operator=(const Elc&);           // Assignment operator

//----------------------------------------------------------------------------
// Elc::Accessors
//----------------------------------------------------------------------------
public:
// None defined

//----------------------------------------------------------------------------
//
// Method-
//       findRoot
//
// Purpose-
//       Find the root of a word.
//
// Examples-
//       after =>    prep: after
//       boat =>     noun: boat
//       fasted =>   verb: fast
//       faster =>    adj: fast
//       help=> noun/verb: help (CONTEXT SENSITIVE)
//       lonely =>    adj: alone (EXCEPTION)
//       quicken =>  verb: From adj: quick
//       quickly =>   adv: From adj: quick
//       quicker =>   adj: quick
//       quickest =>  adj: quick
//       wanting =>   adj: From verb: want
//
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
//
// Method-
//       findRootType
//
// Purpose-
//       Find the root type for a word.
//
//----------------------------------------------------------------------------
public:
}; // class Elc

#endif // ELC_H_INCLUDED
