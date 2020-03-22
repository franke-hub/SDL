//----------------------------------------------------------------------------
//
//       Copyright (c) 2014 Frank Eskesen.
//
//       This file is free content, distributed under the MIT license.
//       (See accompanying file LICENSE.MIT or the original contained
//       within https://opensource.org/licenses/MIT)
//
//----------------------------------------------------------------------------
//
// Title-
//       Main.cpp
//
// Purpose-
//       Compilation test.
//
// Last change date-
//       2014/01/01
//
// Usage-
//       Compile only
//
//----------------------------------------------------------------------------
#include <assert.h>
#include <stdio.h>

#if 0 // Many of these do not compile
#include "Behavioral/ChainOfResponsibility.h"
#include "Behavioral/Command.h"
#include "Behavioral/Interpreter.h"
#include "Behavioral/Iterator.h"
#include "Behavioral/Mediator.h"
#include "Behavioral/Memento.h"
#include "Behavioral/Observer.h"
#include "Behavioral/State.h"
#include "Behavioral/Strategy.h"
#include "Behavioral/TemplateMethod.h"
#include "Behavioral/Visitor.h"

#include "Creational/AbstractBuilder.h"
#include "Creational/AbstractCreator.h"
#include "Creational/AbstractFactory.h"
#include "Creational/Interface.h"
#include "Creational/Prototype.h"
#include "Creational/Singleton.h"

#include "Structural/Adapter.h"
#include "Structural/Bridge.h"
#include "Structural/Composite.h"
#include "Structural/Decorator.h"
#include "Structural/Facade.h"
#include "Structural/Flyweight.h"
#include "Structural/Proxy.h"
#endif

#include "Creational/Interface.h"

//----------------------------------------------------------------------------
//
// Subroutine-
//       main
//
// Purpose-
//       Mainline code.
//
//----------------------------------------------------------------------------
extern int                          // Return code
   main(                            // Mainline code
     int             argc,          // Argument count
     char*           argv[])        // Argument array
{
   InterfaceSampleClient  interfaceSample;
   interfaceSample.run();

   return 0;
}

