//----------------------------------------------------------------------------
//
//       Copyright (c) 2018 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       Test_library.hpp
//
// Purpose-
//       Document library bringup testing.
//
// Last change date-
//       2018/01/01
//
// Implementation notes-
//       This is a documentation-only file. It actually does nothing.
//
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
//
// Section-
//       Requirements
//
// Purpose-
//       Describe the required library functions.
//
// Documentation-
//       OUTPUTS: The library must support writing to stdout.
//                The library must support reading and writing permanent data.
//       THREADS: The library must support threading and provide requisite
//                synchronization controls.
//       SOCKETS: The library must support listener and bi-directional sockets.
//
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
//
// Testing-
//       Basic test
//
// Purpose-
//       Describe the basic "Hello world" library test.
//
// Documentation-
//       Create a listener thread, wait for it to initialize.
//           The listener thread listens on an internally well-known socket.
//           Each connecting socket runs in a new sever thread.
//           Server thread get client socket requests to read a file, then return
//              return the file data over the socket. Server threads remain
//              running until the client closes the socket.
//       Create at least one client thread in an empty directory.
//           The client thread connects to the the listener thread, then
//              requests at least one file. The returned file is written into
//              the current directory.
//       The main program waits for all clients to complete, then:
//           (OPTIONAL) Verify that no server threads remain running.
//           Close, or have the listener close, the listener socket.
//           Wait for the listener thread to complete.
//       Verify that the client correctly copied the source files.
//           This can be a manual or automated procedure.
//       If possible, use additional library verification tests.
//           For example, insure that no memory leaks occurred.
//
//----------------------------------------------------------------------------

