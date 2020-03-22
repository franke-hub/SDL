//----------------------------------------------------------------------------
//
//       Copyright (c) 2010 Frank Eskesen.
//
//       This file is free content, distributed under the MIT license.
//       (See accompanying file LICENSE.MIT or the original contained
//       within https://opensource.org/licenses/MIT)
//
//----------------------------------------------------------------------------
//
// Title-
//       Object.h
//
// Purpose-
//       Graphical User Interface: Object (base class)
//
// Last change date-
//       2010/01/01
//
// Implementation notes-
//       This is the base class for all items that can be rendered.
//       Methods are provided for manipulating the Object tree.
//       The Window object is at the top of this tree, and other Objects
//       are placed (by the application) as desired.
//
//----------------------------------------------------------------------------
#ifndef GUI_OBJECT_H_INCLUDED
#define GUI_OBJECT_H_INCLUDED

#ifndef GUI_TYPES_H_INCLUDED
#include "Types.h"                  // This include is guaranteed
#endif

#include "namespace.gui"            // Graphical User Interface
//----------------------------------------------------------------------------
// Forward references
//----------------------------------------------------------------------------
class Action;                       // For getAction, addAction, delAction
class Buffer;                       // For getBuffer
class ObjectVisitor;                // For visit
class Window;                       // For getWindow

//----------------------------------------------------------------------------
//
// Class-
//       Object
//
// Purpose-
//       The Object controls the logical position of an item that can be
//       rendered within a tree of such objects, and provides utility
//       functions available to derived Objects.
//
// Usage note-
//       Given a Object tree such as:
//       Window w
//         Buffer b
//           Object o1
//             Object o.10->Object o.11->Object o.12
//           Object o2
//             Object o.20->Object o.21->Object o.22
//
//       The visit (and therefore render) sequence is:
//         w, b, o.10, o.11, o.12, o.20, o.21, o.22
//       Note that the LAST object rendered overwrites prior objects.
//
//       The above tree could be created by the sequence:
//       Window w;                  // (IS Buffer b)
//       Object o2(&w)              // (Placeholder)
//       Object o.22(&o2);
//       Object o.21(&o2);
//       Object o.20(&o2);
//       Object o1(&w)              // (Placeholder)
//       Object o.12(&o1);
//       Object o.11(&o1);
//       Object o.10(&o1);
//
// Usage note-
//       An Object instance can only be at one place within one tree.
//
// Usage note-
//       The Object methods do not have side effects. More explicitly,
//       while the insert(), lower(), raise(), and remove() methods can
//       change the appearance of the Window, nothing happens within
//       these methods that makes the change visible.
//
//       There are two ways to expose the changes in the Window:
//       1) window.render(); renders (redraws) the entire Window.
//       2) RenderVisitor visitor;  // Create a RenderVisitor instance
//          object.visit(visitor);  // Render the subtree of the object
//          object.change();        // Expose the change upward to the Window
//
// Error returns-
//       "Corrupt tree" object.remove(arg)
//       arg->getParent()==&object, arg->prior()==NULL, but object.child!=arg.
//       Fix: This error can be ignored (since the object to be removed
//       wasn't on the tree in the first place,) but something is fishy
//       because Object tree is inconsistent.
//
//       "Parent not NULL" object.insert(arg)
//       The argument's parent is not NULL. (It's already on a tree.)
//       Fix: Use arg->getParent()->remove();
//
//       "Parent is NULL" object.raise(arg) or object.lower(arg)
//       The object instance has no parent, it cannot be raised or lowered.
//       Fix: Use arg->getParent()->insert(&object);
//
//       "Parents differ" object.raise(arg) or object.lower(arg)
//       The argument does not have the same parent as the object itself.
//       Fix: Use arg->getParent()->remove(); object.getParent()->insert(arg);
//
//       "Parents differ" object.remove(arg)
//       arg->getParent() != &object. The argument is not a child of object.
//       Fix: Use arg->getParent()->remove(); This will either succeed or
//       do unpleasant things if arg->getParent() == NULL.
//
//       "this==argument" object.raise(arg) or object.lower(arg)
//       The argument is the object, e.g. object.raise(&object).
//       Fix: Don't do that. You should also expect NULL arguments to raise,
//       lower, insert, or remove to do unpleasant things.
//
//----------------------------------------------------------------------------
class Object : public Attributes {  // Object base class
//----------------------------------------------------------------------------
// Object::Enumerations and typedefs
//----------------------------------------------------------------------------
public:
enum Attribute                      // The attribute list
{  VISIBLE                          // Visible
,  TRANSPARENT                      // Transparent
,  HAS_FOCUS                        // Has FOCUS (Is selected for input)
,  HAS_LOCUS                        // Has LOCUS (Mouse is over Object)
,  HAS_HOCUS                        // Has HOCUS (Is selected insert/delete)
,  HAS_POCUS                        // Has POCUS (Is selected for drag/drop)
,  ATTRIBUTE_COUNT                  // Number of attributes

// Synonyms
,  HAS_KEYBOARD=  HAS_FOCUS         // Is selected for input (Has FOCUS)
,  HAS_MOUSEOVER= HAS_LOCUS         // Mouse is over Object  (Has LOCUS)
,  HAS_INSDEL=    HAS_HOCUS         // Is selected for insert/delete
,  HAS_DRAGDROP=  HAS_POCUS         // Is selected for drag/drop
}; // enum Attribute

//----------------------------------------------------------------------------
// Object::Attributes
//----------------------------------------------------------------------------
private:                            // Not modifiable by derived Objects
Object*                parent;      // Parent Object
Object*                peer;        // Next peer Object
Object*                child;       // First child Object
Action*                action;      // First Action item

protected:
Color_t                color;       // Object (background) Color
const char*            name;        // Object name

//----------------------------------------------------------------------------
// Object::Constructors
//----------------------------------------------------------------------------
public:
virtual
   ~Object( void );                 // Destructor
   Object(                          // Constructor
     Object*           parent = NULL); // -> Parent Object

//----------------------------------------------------------------------------
//
// Public method-
//       Object::getAction          (Get first Action item)
//       Object::getBuffer          (Get containing Buffer*)
//       Object::getChild           (Get first child)
//       Object::getColor           (Get <background> Color)
//       Object::getName            (Get name)
//       Object::getParent          (Get parent)
//       Object::getPeer            (Get next peer)
//       Object::getPixel           (Get Pixel*)
//       Object::getWindow          (Get containing Window*)
//
//       Object::setColor           (Set <background> Color)
//       Object::setName            (Set name)
//       Object::setPixel           (Set <foreground> Color)
//
//       Object::addAction          (Add Action item)
//       Object::delAction          (Delete Action item)
//
// Purpose-
//       Accessor methods.
//
// Usage notes-
//       Object set methods do not call change. Derived objects may
//       (at their option) invoke change as a side-effect.
//
// Usage notes-
//       Attributes are object recommendations. They are not enforced,
//       but are followed by all GUI Objects. Methods getPixel and
//       setPixel do not inspect Attributes.
//
//----------------------------------------------------------------------------
public:
inline Action*                      // The first Action item
   getAction( void ) const          // Get first Action item
{  return action;
}

Buffer*                             // Associated Buffer
   getBuffer( void ) const;         // Access Buffer

inline Object*                      // The first child Object
   getChild( void ) const           // Get first child Object
{  return child;
}

inline Color_t                      // Associated Color
   getColor( void ) const           // Get the Object's Color
{  return color;
}

inline const char*                  // Associated name
   getName( void ) const            // Access name
{  return name;
}

inline Object*                      // The parent Object
   getParent( void ) const          // Get parent Object
{  return parent;
}

inline Object*                      // The next peer Object
   getPeer( void ) const            // Get next peer Object
{  return peer;
}

virtual Pixel*                      // The Pixel* (NULL if x/y not valid)
   getPixel(                        // Get Pixel at
     XOffset_t         x,           // X (horizontal) offset
     YOffset_t         y) const;    // Y (vertical) offset

Window*                             // Associated Window
   getWindow( void ) const;         // Access Window

virtual void
   setColor(                        // Set (background) Color
     Color_t           color);      // To this Color

inline void
   setName(                         // Modify name
     const char*       name)        // The new name
{  this->name= name;
}

virtual Pixel*                      // The Pixel* (NULL if x/y not valid)
   setPixel(                        // Set Pixel at
     XOffset_t         x,           // X (horizontal) offset
     YOffset_t         y,           // Y (vertical) offset
     Color_t           color);      // The Pixel value

void
   addAction(                       // Add Action item
     Action*           action);     // The Action item

void
   delAction(                       // Delete Action item
     Action*           action);     // The Action item

//----------------------------------------------------------------------------
//
// Public method-
//       Object::change
//
// Purpose-
//       Indicate that this Object has been changed, reflecting the
//       change upward to the device window (which displays it.)
//
// Usage note-
//       The change methods are only meaningful in derived classes.
//       The Object methods simply bump the request up to its parent.
//
// Usage note-
//       Change reflects an action that has already occurred up to the
//       enclosing Window. The object and tree remain constant throughout
//       this procedure, but the offset and length passed upward do not.
//
// Implementation note-
//       When the Window object sees the change reflection, it reflects
//       the change out to its associated device. The Window itself does
//       not change, so the const attribute is appropriate.
//
//----------------------------------------------------------------------------
public:
virtual void
   change(                          // Reflect change
     const XYOffset&   offset,      // Offset (in Object)
     const XYLength&   length) const; // Length (in Object)

virtual void
   change( void ) const;            // Reflect change

//----------------------------------------------------------------------------
//
// Public method-
//       Object::range
//
// Purpose-
//       Calculate visible range.
//
//-----------------------------------------------------`----------------------
public:
virtual Buffer*                     // The target Buffer
   range(                           // Calculate visible range (in target)
     XYOffset&         offset,      // OUTPUT: Visible offset
     XYLength&         length) const; // OUTPUT: Visible length

//----------------------------------------------------------------------------
//
// Public method-
//       Object::redraw
//
// Purpose-
//       Redraw reflects a request for action on the part of the enclosing
//       Window. Like change, this request is reflected upward through the
//       object tree, possibly changing the offset and length.
//
// Implementation note-
//       When the Window object sees the redraw reflection, it initiates
//       render operations on the appropriate subtree. Since render can and
//       does modify certain Objects and this operation occurs before the
//       redraw completes, the const attribute is not appropriate here.
//
//----------------------------------------------------------------------------
public:
virtual void
   redraw(                          // Redraw part of the Object
     const XYOffset&   offset,      // Offset
     const XYLength&   length);     // Length

virtual void
   redraw( void );                  // Redraw the Object

//----------------------------------------------------------------------------
//
// Public method-
//       Object::render
//
// Purpose-
//       Render this Object, drawing its content. This method is normally
//       not called directly, but as a result of redraw.
//       In the base class this method does nothing.
//
//----------------------------------------------------------------------------
public:
virtual void
   render( void );                  // Draw Object content

//----------------------------------------------------------------------------
//
// Public method-
//       Object::visit(ObjectVisitor&)
//
// Purpose-
//       Visit this Object and all its children.
//
//----------------------------------------------------------------------------
public:
virtual void
   visit(                           // Visit the Object tree
     ObjectVisitor&    visitor);    // The ObjectVisitor

//----------------------------------------------------------------------------
//
// Public method-
//       Object::visit(ObjectVisitor&, const XYOffset&, const XYLength&)
//
// Purpose-
//       Visit this Object and all its children that are within the specified
//       boundary criteria. The resultant is the LAST Object instance on the
//       tree that satisfies the offset/length criteria and which does NOT
//       return NULL when visited. Note that, since Objects do not have
//       offset or length attributes, they cannot satisfy such criteria.
//       However, a Bounds child of an Object may satisfy the criteria.
//
// Usage note-
//       Normally, the resultant would be the visible object that completely
//       satisfied the boundary conditions. However, the default ObjectVisitor
//       does NOT examine the VISIBLE attribute, so an invisible resultant is
//       possible.
//
//----------------------------------------------------------------------------
public:
virtual Object*                     // Resultant, or NULL if none
   visit(                           // Visit the Object tree using
     ObjectVisitor&    visitor,     // This ObjectVisitor for
     const XYOffset&   offset,      // This offset and
     const XYLength&   length);     // This length

//----------------------------------------------------------------------------
//
// Public method-
//       Object::insert             // No known derived method instance
//
// Purpose-
//       Insert an Object onto this Object's child list.
//
//----------------------------------------------------------------------------
public:
virtual const char*                 // Exception message (NULL OK)
   insert(                          // Insert a child Object
     Object*           object);     // The Object to insert

//----------------------------------------------------------------------------
//
// Public method-
//       Object::lower              // No known derived method instance
//
// Purpose-
//       Lower the priority of this Object within the parent Object.
//       (This Object will then preceed the specified Object.)
//
//----------------------------------------------------------------------------
public:
virtual const char*                 // Exception message (NULL OK)
   lower(                           // Lower Object
     Object*           object);     // Below this Object

virtual const char*                 // Exception message (NULL OK)
   lower( void );                   // Lower Object (to lowest priority)

//----------------------------------------------------------------------------
//
// Public method-
//       Object::raise              // No known derived method instance
//
// Purpose-
//       Raise the priority of this Object within the parent Object.
//       (This Object will then follow the specified Object.)
//
//----------------------------------------------------------------------------
public:
virtual const char*                 // Exception message (NULL OK)
   raise(                           // Raise Object
     Object*           object);     // Above this Object

virtual const char*                 // Exception message (NULL OK)
   raise( void );                   // Raise Object (to highest priority)

//----------------------------------------------------------------------------
//
// Public method-
//       Object::remove             // No known derived method instance
//
// Purpose-
//       Remove a Object from this Object's child list.
//
//----------------------------------------------------------------------------
public:
virtual const char*                 // Exception message (NULL OK)
   remove(                          // Remove a child Object
     Object*           object);     // The Object to remove

//----------------------------------------------------------------------------
//
// Private method-
//       Object::prior              // Private method
//
// Purpose-
//       Address the peer that preceeds this Object in the parent list.
//
//----------------------------------------------------------------------------
private:
Object*                             // -> Prior peer Object
   prior( void ) const;             // Address Prior Object
}; // class Object

//----------------------------------------------------------------------------
//
// Class-
//       ObjectVisitor
//
// Purpose-
//       Abstract ObjectVisitor base class.
//
// Usage-
//       A return value of NULL terminates the visit. Child objects are not
//       visited (but peer objects are.)
//
//----------------------------------------------------------------------------
class ObjectVisitor {               // Visitor
//----------------------------------------------------------------------------
// ObjectVisitor::Methods
//----------------------------------------------------------------------------
public:
virtual Object*                     // The visited Object
   visit(                           // Visit an Object
     Object*           object)      // The Object to visit
{  return object;                   // The base class does almost nothing
}
}; // class ObjectVisitor

//----------------------------------------------------------------------------
//
// Class-
//       RenderVisitor
//
// Purpose-
//       Render the object subtree.
//
// Usage notes-
//       Object::visit invokes Buffer::upload for Buffer objects when an
//       ObjectVisitor is of this class. Starting a RenderVisitor on a
//       Buffer object modifies the Buffer object's parent Buffer. (This
//       has no effect for Window buffers, which have no parent Buffer.)
//
//----------------------------------------------------------------------------
class RenderVisitor : public ObjectVisitor { // RenderVisior
public:
virtual Object*                     // The visited Object
   visit(                           // Visit an Object
     Object*           object)      // The Object to visit
{  object->render();
   return object;
}
}; // class RenderVisitor
#include "namespace.end"

#endif // GUI_OBJECT_H_INCLUDED
