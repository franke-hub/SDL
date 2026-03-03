//----------------------------------------------------------------------------
//
//       Copyright (c) 2026 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
// SPDX-License-Identifier: GPL-3.0-only
//----------------------------------------------------------------------------
//
// Title-
//       Test_cv_OK.cpp
//
// Purpose-
//       Test: condition variable (good) example
//
// Last change date-
//       2026/02/26
//
//----------------------------------------------------------------------------
#include <condition_variable>
#include <iostream>
#include <mutex>
#include <string>
#include <thread>

static void delay() // Delay before next sequence
{  std::this_thread::sleep_for(std::chrono::milliseconds(2500)); }

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
class Event {
private:
volatile uint32_t      posted= false;
std::condition_variable
                       cv;
std::mutex             mutex;

public:
void post()
{
   delay();
   std::unique_lock<decltype(mutex)> lock(mutex);

   posted= true;
   cv.notify_all();
}

void wait( void )
{  std::unique_lock<decltype(mutex)> lock(mutex);

   while( !posted )
     cv.wait(lock);
}
}; // class Event

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
Event A;
Event B;
Event C;

std::string data;

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
static void run_one()
{
    A.wait(); // Wait until thread_two starts

    std::cout << "thread_one is running (after thread_two was starting)\n";
    data += " => running-one";

    B.post(); // allow thread_two to continue
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
static void run_two()
{
    delay();
    std::cout << "thread_two is starting (before thread_one is running)\n";
    data += " => starting-two";

    A.post(); // allow thread_one to start

    B.wait(); // wait for thread_one to finish

    std::cout << "thread_two is running (after thread_one was running)\n";
    data += " => running-two";

    C.post(); // allow the main thread to continue
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
int main()
{
    data = "main...";

    std::thread thread_one(run_one);
    delay();
    std::thread thread_two(run_two);

    C.wait(); // Wait for thread_two to complete
    data += " => ...main";
    std::cout << "Back in main()\n" << "data = '" << data << "'\n";

    thread_two.join();
    thread_one.join();
}
