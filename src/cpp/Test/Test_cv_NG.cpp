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
//       Test_cv_NG.cpp
//
// Purpose-
//       Test: condition variable (bogus) example
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

std::mutex m;
//std::condition_variable cv;
std::string data;
bool ready = false;
bool processed = false;

static void worker_thread()
{
    // wait until main() sends data
    std::unique_lock lk(m);
//    cv.wait(lk, []{ return ready; });

    // after the wait, we own the lock
    std::cout << "Worker thread is processing data\n";
    data += " after processing";

    // send data back to main()
    processed = true;
    std::cout << "Worker thread signals data processing completed\n";

    // manual unlocking is done before notifying, to avoid waking up
    // the waiting thread only to block again (see notify_one for details)
    lk.unlock();
//    cv.notify_one();
}

int main()
{
    std::thread worker(worker_thread);

    data = "Example data";
    // send data to the worker thread
    {
        std::lock_guard lk(m);
        ready = true;
        std::cout << "main() signals data ready for processing\n";
    }
//    cv.notify_one();
    std::this_thread::yield(); // ADDED

    // wait for the worker
    {
        std::unique_lock lk(m);
//        cv.wait(lk, []{ return processed; });
    }
    std::cout << "Back in main(), data = " << data << '\n';

    worker.join();
}
