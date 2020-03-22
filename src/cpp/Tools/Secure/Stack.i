//----------------------------------------------------------------------------
//
//       Copyright (c) 2007 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       Stack.i
//
// Purpose-
//       Stack inline methods.
//
// Last change date-
//       2007/01/01
//
//----------------------------------------------------------------------------
#ifndef STACK_I_INCLUDED
#define STACK_I_INCLUDED

#ifndef __CHECKING__
#undef  __CHECKING__                // If defined, extra checking
#endif

//----------------------------------------------------------------------------
//
// Method-
//       StackBase::~StackBase
//
// Function-
//       Destructor.
//
//----------------------------------------------------------------------------
   StackBase::~StackBase( void )    // Destructor.
{
}

//----------------------------------------------------------------------------
//
// Method-
//       StackBase::StackBase
//
// Function-
//       Constructor.
//
//----------------------------------------------------------------------------
   StackBase::StackBase( void )     // Constructor.
:  top(-1)
,  bot(-1)
{
}

//----------------------------------------------------------------------------
//
// Method-
//       StackBase::reset
//
// Function-
//       Reset the stack, making it empty.
//
//----------------------------------------------------------------------------
void INLINE
   StackBase::reset( void )         //  Reset the stack, making it empty
{
   top= (-1);
   bot= (-1);
}

//----------------------------------------------------------------------------
//
// Method-
//       Stack::coherencyDebug
//
// Function-
//       Debugging tool.
//
//----------------------------------------------------------------------------
template<class T, int depth>
void INLINE
   Stack<T,depth>::coherencyDebug( void ) const
{
   printf("Stack(%p) top(%d) bot(%d)\n", this, top, bot);
}

//----------------------------------------------------------------------------
//
// Method-
//       Stack::~Stack
//
// Function-
//       Destructor.
//
//----------------------------------------------------------------------------
template<class T, int depth>
   Stack<T,depth>::~Stack( void )    // Destructor.
{
}

//----------------------------------------------------------------------------
//
// Method-
//       Stack::Stack
//
// Function-
//       Constructor.
//
//----------------------------------------------------------------------------
template<class T, int depth>
   Stack<T,depth>::Stack( void )    // Constructor.
:  StackBase()
{
}

//----------------------------------------------------------------------------
//
// Method-
//       Stack::fifo
//
// Function-
//       Push element onto the Stack, FIFO ordering.
//
//----------------------------------------------------------------------------
template<class T, int depth>
void INLINE
   Stack<T,depth>::fifo(            // Push element onto the Stack
     T                 element)     // The element to push
{
   if( top < 0 )                    // If the Stack is empty
   {
     top= 0;
     bot= 0;
     array[0]= element;
     return;
   }

   bot++;                           // Allocate a slot
   if( bot >= depth )               // If wrap
     bot= 0;                        // Wrap

   #ifdef __CHECKING__
     if( top == bot )               // If overflow
     {
       fprintf(stderr, "Stack::fifo(), stack overflow\n");
       exit(EXIT_FAILURE);
     }
   #endif

   array[bot]= element;
}

//----------------------------------------------------------------------------
//
// Method-
//       Stack::lifo
//
// Function-
//       Push element onto the Stack, LIFO ordering.
//
//----------------------------------------------------------------------------
template<class T, int depth>
void INLINE
   Stack<T,depth>::lifo(            // Push element onto the Stack
     T                 element)     // The element to push
{
   if( top < 0 )                    // If the Stack is empty
   {
     top= 0;
     bot= 0;
     array[0]= element;
     return;
   }

   top--;                           // Allocate a slot
   if( top < 0 )                    // If wrap
     top= depth - 1;                // Wrap

   #ifdef __CHECKING__
     if( top == bot )               // If overflow
     {
       fprintf(stderr, "Stack::lifo(), stack overflow\n");
       exit(EXIT_FAILURE);
     }
   #endif

   array[top]= element;
}

//----------------------------------------------------------------------------
//
// Method-
//       Stack::pull
//
// Function-
//       Pull element from the Stack.
//
//----------------------------------------------------------------------------
template<class T, int depth>
T INLINE                            // Resultant
   Stack<T,depth>::pull( void )     // Pull element from the Stack
{
   int                 oldTop;      // Resultant (index)

   #ifdef __CHECKING__
     if( top < 0 )                  // If the Stack is empty
     {
       fprintf(stderr, "Stack::pull(), stack underflow\n");
       exit(EXIT_FAILURE);
     }
   #endif

   oldTop= top;                     // Save resultant index
   if( top == bot )                 // If the stack is now empty
   {
     top= (-1);
     bot= (-1);
   }
   else
   {
     top++;
     if( top >= depth )
       top= 0;
   }

   return array[oldTop];
}

#endif // STACK_I_INCLUDED
