//----------------------------------------------------------------------------
//
//       Copyright (c) 2007-2021 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       Dumpobj.cpp
//
// Purpose-
//       Dump object file.
//
// Last change date-
//       2021/07/17
//
//----------------------------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#if defined(_OS_WIN) || defined(_OS_CYGWIN)
#include <windows.h>

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#define __SOURCE__       "DUMPOBJ " // Source file

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#define MAX_SECTIONS             32 // Maximum number of sections

//----------------------------------------------------------------------------
// Macros
//----------------------------------------------------------------------------
#define INTFIELD(name) \
            printf("%10ld, 0x%.8lx = %s\n", (long)name, (long)name, #name)

//----------------------------------------------------------------------------
// Local data areas
//----------------------------------------------------------------------------
static int             fileArgc;    // File argument number
static FILE*           fileHand;    // Current file handle
static char*           fileName;    // Current file name

static unsigned        sectCount;   // Number of sections
static IMAGE_DOS_HEADER
                       dosHeader;   // DOS header data
static IMAGE_NT_HEADERS
                       ntsHeader;   // NT Headers
static IMAGE_SECTION_HEADER
                       section[MAX_SECTIONS]; // Section data

static char            sw_debug;    // TRUE iff -debug
static char            sw_verbose;  // TRUE iff -verbose

//----------------------------------------------------------------------------
//
// Section-
//       obsolete
//
// Purpose-
//       Container for code which may eventually get used.
//
//----------------------------------------------------------------------------
#if 0
static IMAGE_DOS_HEADER
                       d02Header;   // DOS header data 2
static IMAGE_OS2_HEADER
                       os2Header;   // OS2 header data
static IMAGE_VXD_HEADER
                       o32Header;   // VXD header data
static IMAGE_OPTIONAL_HEADER
                       optHeader;   // Optional header
static IMAGE_ROM_OPTIONAL_HEADER
                       romHeader;   // ROM Optional header

   //-------------------------------------------------------------------------
   // Read the secondary file header
   //-------------------------------------------------------------------------
   if( seekf(dosHeader.e_lfanew) != 0 )
     return 1;
   L= readf(&d02Header, sizeof(d02Header));
   if( L <= 0 )
     return 1;

   //-------------------------------------------------------------------------
   // Display it
   //-------------------------------------------------------------------------
   if( sw_verbose )
   {
     printf("\n");
     printf("d02Header:\n");
     INTFIELD(d02Header.e_magic);      // Magic number
     INTFIELD(d02Header.e_cblp);       // Bytes on last page of file
     INTFIELD(d02Header.e_cp);         // Pages in file
     INTFIELD(d02Header.e_crlc);       // Relocations
     INTFIELD(d02Header.e_cparhdr);    // Size of header in paragraphs
     INTFIELD(d02Header.e_minalloc);   // Minimum extra paragraphs needed
     INTFIELD(d02Header.e_maxalloc);   // Maximum extra paragraphs needed
     INTFIELD(d02Header.e_ss);         // Initial (relative) SS value
     INTFIELD(d02Header.e_sp);         // Initial SP value
     INTFIELD(d02Header.e_csum);       // Checksum
     INTFIELD(d02Header.e_ip);         // Initial IP value
     INTFIELD(d02Header.e_cs);         // Initial (relative) CS value
     INTFIELD(d02Header.e_lfarlc);     // File address of relocation table
     INTFIELD(d02Header.e_ovno);       // Overlay number
     INTFIELD(d02Header.e_oemid);      // OEM identifier (for e_oeminfo)
     INTFIELD(d02Header.e_oeminfo);    // OEM information; e_oemid specific
     INTFIELD(d02Header.e_lfanew);     // File address of new exe header
   }

   //-------------------------------------------------------------------------
   // Validate it
   //-------------------------------------------------------------------------
   // TODO: Validate

   //-------------------------------------------------------------------------
   // Read the secondary file header
   //-------------------------------------------------------------------------
   if( seekf(dosHeader.e_lfanew) != 0 )
     return 1;
   L= readf(&os2Header, sizeof(os2Header));
   if( L <= 0 )
     return 1;

   //-------------------------------------------------------------------------
   // Display it
   //-------------------------------------------------------------------------
   if( sw_verbose )
   {
     printf("\n");
     printf("os2Header:\n");
     INTFIELD(os2Header.ne_magic);         // Magic number
     INTFIELD(os2Header.ne_ver);           // Version number
     INTFIELD(os2Header.ne_rev);           // Revision number
     INTFIELD(os2Header.ne_enttab);        // Offset of Entry Table
     INTFIELD(os2Header.ne_cbenttab);      // Number of bytes in Entry Table
     INTFIELD(os2Header.ne_crc);           // Checksum of whole file
     INTFIELD(os2Header.ne_flags);         // Flag word
     INTFIELD(os2Header.ne_autodata);      // Automatic data segment number
     INTFIELD(os2Header.ne_heap);          // Initial heap allocation
     INTFIELD(os2Header.ne_stack);         // Initial stack allocation
     INTFIELD(os2Header.ne_csip);          // Initial CS:IP setting
     INTFIELD(os2Header.ne_sssp);          // Initial SS:SP setting
     INTFIELD(os2Header.ne_cseg);          // Count of file segments
     INTFIELD(os2Header.ne_cmod);          // Entries in Module Reference Table
     INTFIELD(os2Header.ne_cbnrestab);     // Size of non-resident name table
     INTFIELD(os2Header.ne_segtab);        // Offset of Segment Table
     INTFIELD(os2Header.ne_rsrctab);       // Offset of Resource Table
     INTFIELD(os2Header.ne_restab);        // Offset of resident name table
     INTFIELD(os2Header.ne_modtab);        // Offset of Module Reference Table
     INTFIELD(os2Header.ne_imptab);        // Offset of Imported Names Table
     INTFIELD(os2Header.ne_nrestab);       // Offset of Non-resident Names Table
     INTFIELD(os2Header.ne_cmovent);       // Count of movable entries
     INTFIELD(os2Header.ne_align);         // Segment alignment shift count
     INTFIELD(os2Header.ne_cres);          // Count of resource segments
     INTFIELD(os2Header.ne_exetyp);        // Target Operating system
     INTFIELD(os2Header.ne_flagsothers);   // Other .EXE flags
     INTFIELD(os2Header.ne_pretthunks);    // offset to return thunks
     INTFIELD(os2Header.ne_psegrefbytes);  // offset to segment ref. bytes
     INTFIELD(os2Header.ne_swaparea);      // Minimum code swap area size
     INTFIELD(os2Header.ne_expver);        // Expected Windows version number
   }

   //-------------------------------------------------------------------------
   // Validate it
   //-------------------------------------------------------------------------
   // TODO: Validate

   //-------------------------------------------------------------------------
   // Read the secondary file header
   //-------------------------------------------------------------------------
   if( seekf(dosHeader.e_lfanew) != 0 )
     return 1;
   L= readf(&o32Header, sizeof(o32Header));
   if( L <= 0 )
     return 1;

   //-------------------------------------------------------------------------
   // Display it
   //-------------------------------------------------------------------------
   if( sw_verbose )
   {
     printf("\n");
     printf("vxdHeader:\n");
     INTFIELD(o32Header.e32_magic);        // Magic number
     INTFIELD(o32Header.e32_border);       // The byte ordering for the VXD
     INTFIELD(o32Header.e32_worder);       // The word ordering for the VXD
     INTFIELD(o32Header.e32_level);        // The EXE format level for now = 0
     INTFIELD(o32Header.e32_cpu);          // The CPU type
     INTFIELD(o32Header.e32_os);           // The OS type
     INTFIELD(o32Header.e32_ver);          // Module version
     INTFIELD(o32Header.e32_mflags);       // Module flags
     INTFIELD(o32Header.e32_mpages);       // Module # pages
     INTFIELD(o32Header.e32_startobj);     // Object # for instruction pointer
     INTFIELD(o32Header.e32_eip);          // Extended instruction pointer
     INTFIELD(o32Header.e32_stackobj);     // Object # for stack pointer
     INTFIELD(o32Header.e32_esp);          // Extended stack pointer
     INTFIELD(o32Header.e32_pagesize);     // VXD page size
     INTFIELD(o32Header.e32_lastpagesize); // Last page size in VXD
     INTFIELD(o32Header.e32_fixupsize);    // Fixup section size
     INTFIELD(o32Header.e32_fixupsum);     // Fixup section checksum
     INTFIELD(o32Header.e32_ldrsize);      // Loader section size
     INTFIELD(o32Header.e32_ldrsum);       // Loader section checksum
     INTFIELD(o32Header.e32_objtab);       // Object table offset
     INTFIELD(o32Header.e32_objcnt);       // Number of objects in module
     INTFIELD(o32Header.e32_objmap);       // Object page map offset
     INTFIELD(o32Header.e32_itermap);      // Object iterated data map offset
     INTFIELD(o32Header.e32_rsrctab);      // Offset of Resource Table
     INTFIELD(o32Header.e32_rsrccnt);      // Number of resource entries
     INTFIELD(o32Header.e32_restab);       // Offset of resident name table
     INTFIELD(o32Header.e32_enttab);       // Offset of Entry Table
     INTFIELD(o32Header.e32_dirtab);       // Offset of Module Directive Table
     INTFIELD(o32Header.e32_dircnt);       // Number of module directives
     INTFIELD(o32Header.e32_fpagetab);     // Offset of Fixup Page Table
     INTFIELD(o32Header.e32_frectab);      // Offset of Fixup Record Table
     INTFIELD(o32Header.e32_impmod);       // Offset of Import Module Name Table
     INTFIELD(o32Header.e32_impmodcnt);    // Number of entries in Import Module Name Table
     INTFIELD(o32Header.e32_impproc);      // Offset of Import Procedure Name Table
     INTFIELD(o32Header.e32_pagesum);      // Offset of Per-Page Checksum Table
     INTFIELD(o32Header.e32_datapage);     // Offset of Enumerated Data Pages
     INTFIELD(o32Header.e32_preload);      // Number of preload pages
     INTFIELD(o32Header.e32_nrestab);      // Offset of Non-resident Names Table
     INTFIELD(o32Header.e32_cbnrestab);    // Size of Non-resident Name Table
     INTFIELD(o32Header.e32_nressum);      // Non-resident Name Table Checksum
     INTFIELD(o32Header.e32_autodata);     // Object # for automatic data object
     INTFIELD(o32Header.e32_debuginfo);    // Offset of the debugging information
     INTFIELD(o32Header.e32_debuglen);     // The length of the debugging info. in bytes
     INTFIELD(o32Header.e32_instpreload);  // Number of instance pages in preload section of VXD file
     INTFIELD(o32Header.e32_instdemand);   // Number of instance pages in demand load section of VXD file
     INTFIELD(o32Header.e32_heapsize);     // Size of heap - for 16-bit apps
     INTFIELD(o32Header.e32_winresoff);
     INTFIELD(o32Header.e32_winreslen);
     INTFIELD(o32Header.e32_devid);        // Device ID for VxD
     INTFIELD(o32Header.e32_ddkver);       // DDK version for VxD
   }

   //-------------------------------------------------------------------------
   // Validate it
   //-------------------------------------------------------------------------
   // TODO: Validate

   //-------------------------------------------------------------------------
   // Read the optional file header
   //-------------------------------------------------------------------------
   if( seekf(dosHeader.e_lfanew) != 0 )
     return 1;
   L= readf(&optHeader, sizeof(optHeader));
   if( L <= 0 )
     return 1;

   //-------------------------------------------------------------------------
   // Display it
   //-------------------------------------------------------------------------
   if( sw_verbose )
   {
     printf("\n");
     printf("optHeader:\n");
     INTFIELD(optHeader.Magic);
     INTFIELD(optHeader.MajorLinkerVersion);
     INTFIELD(optHeader.MinorLinkerVersion);
     INTFIELD(optHeader.SizeOfCode);
     INTFIELD(optHeader.SizeOfInitializedData);
     INTFIELD(optHeader.SizeOfUninitializedData);
     INTFIELD(optHeader.AddressOfEntryPoint);
     INTFIELD(optHeader.BaseOfCode);
     INTFIELD(optHeader.BaseOfData);
     INTFIELD(optHeader.ImageBase);
     INTFIELD(optHeader.SectionAlignment);
     INTFIELD(optHeader.FileAlignment);
     INTFIELD(optHeader.MajorOperatingSystemVersion);
     INTFIELD(optHeader.MinorOperatingSystemVersion);
     INTFIELD(optHeader.MajorImageVersion);
     INTFIELD(optHeader.MinorImageVersion);
     INTFIELD(optHeader.MajorSubsystemVersion);
     INTFIELD(optHeader.MinorSubsystemVersion);
     INTFIELD(optHeader.Reserved1);
     INTFIELD(optHeader.SizeOfImage);
     INTFIELD(optHeader.SizeOfHeaders);
     INTFIELD(optHeader.CheckSum);
     INTFIELD(optHeader.Subsystem);
     INTFIELD(optHeader.DllCharacteristics);
     INTFIELD(optHeader.SizeOfStackReserve);
     INTFIELD(optHeader.SizeOfStackCommit);
     INTFIELD(optHeader.SizeOfHeapReserve);
     INTFIELD(optHeader.SizeOfHeapCommit);
     INTFIELD(optHeader.LoaderFlags);
     INTFIELD(optHeader.NumberOfRvaAndSizes);
   }

   //-------------------------------------------------------------------------
   // Read the ROM optional file header
   //-------------------------------------------------------------------------
   if( seekf(dosHeader.e_lfanew) != 0 )
     return 1;
   L= readf(&romHeader, sizeof(romHeader));
   if( L <= 0 )
     return 1;

   //-------------------------------------------------------------------------
   // Display it
   //-------------------------------------------------------------------------
   if( sw_verbose )
   {
     printf("\n");
     printf("romHeader:\n");
     INTFIELD(romHeader.Magic);
     INTFIELD(romHeader.MajorLinkerVersion);
     INTFIELD(romHeader.MinorLinkerVersion);
     INTFIELD(romHeader.SizeOfCode);
     INTFIELD(romHeader.SizeOfInitializedData);
     INTFIELD(romHeader.SizeOfUninitializedData);
     INTFIELD(romHeader.AddressOfEntryPoint);
     INTFIELD(romHeader.BaseOfCode);
     INTFIELD(romHeader.BaseOfData);
     INTFIELD(romHeader.BaseOfBss);
     INTFIELD(romHeader.GprMask);
     INTFIELD(romHeader.CprMask[0]);
     INTFIELD(romHeader.CprMask[1]);
     INTFIELD(romHeader.CprMask[2]);
     INTFIELD(romHeader.CprMask[3]);
     INTFIELD(romHeader.GpValue);
   }

   //-------------------------------------------------------------------------
   // Validate it
   //-------------------------------------------------------------------------
   // TODO: Validate
#endif

//----------------------------------------------------------------------------
//
// Subroutine-
//       info
//
// Purpose-
//       Informational exit.
//
//----------------------------------------------------------------------------
static void
   info(int, char**)                // Mainline code
//   int               argc,        // Argument count (Currently unused)
//   char*             argv[])      // Argument array (Currently unused)
{
   fprintf(stderr, "%s filename ...\n", __SOURCE__);
   exit(EXIT_FAILURE);
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       parm
//
// Purpose-
//       Parameter analysis.
//
//----------------------------------------------------------------------------
static void
   parm(                            // Mainline code
     int               argc,        // Argument count
     char*             argv[])      // Argument array
{
   int                 error;       // Error switch

   //-------------------------------------------------------------------------
   // Set defaults
   //-------------------------------------------------------------------------
   sw_verbose=  TRUE;               // Default switch settings
   sw_debug=   FALSE;

   fileArgc=   (-1);                // Set fileArgc parameter index

   //-------------------------------------------------------------------------
   // Examine parameters
   //-------------------------------------------------------------------------
   error= FALSE;                    // Default, no error found
   for(int i=1; i<argc; i++)        // Examine the parameter list
   {
     if( argv[i][0] == '-' )        // If this is a switch list
     {
       if( strcmp("-help", argv[i]) == 0 )
         error= TRUE;

       else if( strcmp("-debug", argv[i]) == 0 )
         sw_debug= TRUE;

       else                         // Switch list
       {
         for(int j=1; argv[i][j] != '\0'; j++) // Examine the switch list
         {
           switch(argv[i][j])       // Examine the switch
           {
             case 'v':              // -v (verbose)
               sw_verbose= TRUE;
               break;

             default:               // If invalid switch
               error= TRUE;
               fprintf(stderr, "Invalid switch '%c'\n", (int)argv[i][j]);
               break;
           }
         }
       }
       continue;
     }

     //-----------------------------------------------------------------------
     // Process a flat (non-switch) parameter
     //-----------------------------------------------------------------------
     fileArgc= i;                   // Set the filename index
     break;
   }

   //-------------------------------------------------------------------------
   // Validate the parameters
   //-------------------------------------------------------------------------
   if( fileArgc < 0 )               // If fileName not specified
   {
     error= TRUE;
     fprintf(stderr, "Missing filename.\n");
   }

   if( error )                      // If an error was detected
   {
     info(argc, argv);              // Tell how this works
     exit(EXIT_FAILURE);            // And exit, function aborted
   }
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       seekf
//
// Purpose-
//       Seek into file.
//
//----------------------------------------------------------------------------
static int                          // Return code (0 OK)
   seekf(                           // Seek into file
     unsigned          offset)      // Seek offset
{
   unsigned            S;           // Seek offset

   S= fseek(fileHand, offset, SEEK_SET);
   if( S != 0 )
   {
     fprintf(stderr, "File(%s): ", fileName);
     perror("seek error");
   }

   return S;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       readf
//
// Purpose-
//       Read from file.
//
//----------------------------------------------------------------------------
static int                          // Number of bytes read
   readf(                           // Read from file
     void*             addr,        // Input address
     unsigned          size)        // Input length
{
   int L= fread(addr, 1, size, fileHand);
   if( L < 0 )
   {
     fprintf(stderr, "File(%s): ", fileName);
     perror("read error");
   }
   else if( unsigned(L) < size )
   {
     fprintf(stderr, "File(%s): %d= read(%u)\n", fileName, L, size);
     L= (-1);
   }

   return L;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       readHeader
//
// Purpose-
//       Dump the fileHeader.
//
//----------------------------------------------------------------------------
static int                          // Return code
   readHeader( void )               // Parse an object file
{
   int                 result = 0;  // Resultant;

   char                string[32];  // Working string
   int                 L;           // Read length
   int                 S;           // Seek offset

   //-------------------------------------------------------------------------
   // Read the file dosHeader
   //-------------------------------------------------------------------------
   L= readf(&dosHeader, sizeof(dosHeader));
   if( L != sizeof(dosHeader) )
     return 1;

   //-------------------------------------------------------------------------
   // Display it
   //-------------------------------------------------------------------------
   if( sw_verbose )
   {
     printf("\n");
     printf("dosHeader:\n");
     INTFIELD(dosHeader.e_magic);      // Magic number
     INTFIELD(dosHeader.e_cblp);       // Bytes on last page of file
     INTFIELD(dosHeader.e_cp);         // Pages in file
     INTFIELD(dosHeader.e_crlc);       // Relocations
     INTFIELD(dosHeader.e_cparhdr);    // Size of header in paragraphs
     INTFIELD(dosHeader.e_minalloc);   // Minimum extra paragraphs needed
     INTFIELD(dosHeader.e_maxalloc);   // Maximum extra paragraphs needed
     INTFIELD(dosHeader.e_ss);         // Initial (relative) SS value
     INTFIELD(dosHeader.e_sp);         // Initial SP value
     INTFIELD(dosHeader.e_csum);       // Checksum
     INTFIELD(dosHeader.e_ip);         // Initial IP value
     INTFIELD(dosHeader.e_cs);         // Initial (relative) CS value
     INTFIELD(dosHeader.e_lfarlc);     // File address of relocation table
     INTFIELD(dosHeader.e_ovno);       // Overlay number
     INTFIELD(dosHeader.e_oemid);      // OEM identifier (for e_oeminfo)
     INTFIELD(dosHeader.e_oeminfo);    // OEM information; e_oemid specific
     INTFIELD(dosHeader.e_lfanew);     // File address of new exe header
   }

   //-------------------------------------------------------------------------
   // Validate it
   //-------------------------------------------------------------------------
   if( dosHeader.e_magic != IMAGE_DOS_SIGNATURE )
     result= 1;

   //-------------------------------------------------------------------------
   // Read the NT file headers
   //-------------------------------------------------------------------------
   if( seekf(dosHeader.e_lfanew) != 0 )
     return 1;
   L= readf(&ntsHeader, sizeof(ntsHeader));
   if( L <= 0 )
     return 1;

   //-------------------------------------------------------------------------
   // Display it
   //-------------------------------------------------------------------------
   if( sw_verbose )
   {
     printf("\n");
     printf("ntsHeader:\n");
     INTFIELD(ntsHeader.Signature);
     INTFIELD(ntsHeader.FileHeader.Machine);
     INTFIELD(ntsHeader.FileHeader.NumberOfSections);
     INTFIELD(ntsHeader.FileHeader.TimeDateStamp);
     INTFIELD(ntsHeader.FileHeader.PointerToSymbolTable);
     INTFIELD(ntsHeader.FileHeader.NumberOfSymbols);
     INTFIELD(ntsHeader.FileHeader.SizeOfOptionalHeader);
     INTFIELD(ntsHeader.FileHeader.Characteristics);
     INTFIELD(ntsHeader.OptionalHeader.Magic);
     INTFIELD(ntsHeader.OptionalHeader.MajorLinkerVersion);
     INTFIELD(ntsHeader.OptionalHeader.MinorLinkerVersion);
     INTFIELD(ntsHeader.OptionalHeader.SizeOfCode);
     INTFIELD(ntsHeader.OptionalHeader.SizeOfInitializedData);
     INTFIELD(ntsHeader.OptionalHeader.SizeOfUninitializedData);
     INTFIELD(ntsHeader.OptionalHeader.AddressOfEntryPoint);
     INTFIELD(ntsHeader.OptionalHeader.BaseOfCode);
//   INTFIELD(ntsHeader.OptionalHeader.BaseOfData);
     INTFIELD(ntsHeader.OptionalHeader.ImageBase);
     INTFIELD(ntsHeader.OptionalHeader.SectionAlignment);
     INTFIELD(ntsHeader.OptionalHeader.FileAlignment);
     INTFIELD(ntsHeader.OptionalHeader.MajorOperatingSystemVersion);
     INTFIELD(ntsHeader.OptionalHeader.MinorOperatingSystemVersion);
     INTFIELD(ntsHeader.OptionalHeader.MajorImageVersion);
     INTFIELD(ntsHeader.OptionalHeader.MinorImageVersion);
     INTFIELD(ntsHeader.OptionalHeader.MajorSubsystemVersion);
     INTFIELD(ntsHeader.OptionalHeader.MinorSubsystemVersion);
     INTFIELD(ntsHeader.OptionalHeader.Win32VersionValue);
     INTFIELD(ntsHeader.OptionalHeader.SizeOfImage);
     INTFIELD(ntsHeader.OptionalHeader.SizeOfHeaders);
     INTFIELD(ntsHeader.OptionalHeader.CheckSum);
     INTFIELD(ntsHeader.OptionalHeader.Subsystem);
     INTFIELD(ntsHeader.OptionalHeader.DllCharacteristics);
     INTFIELD(ntsHeader.OptionalHeader.SizeOfStackReserve);
     INTFIELD(ntsHeader.OptionalHeader.SizeOfStackCommit);
     INTFIELD(ntsHeader.OptionalHeader.SizeOfHeapReserve);
     INTFIELD(ntsHeader.OptionalHeader.SizeOfHeapCommit);
     INTFIELD(ntsHeader.OptionalHeader.LoaderFlags);
     INTFIELD(ntsHeader.OptionalHeader.NumberOfRvaAndSizes);

     if( ntsHeader.Signature != IMAGE_NT_SIGNATURE
         || ntsHeader.OptionalHeader.NumberOfRvaAndSizes >
            IMAGE_NUMBEROF_DIRECTORY_ENTRIES )
       result= 1;

     if( result == 0 )
     {
       for(unsigned i= 0; i<ntsHeader.OptionalHeader.NumberOfRvaAndSizes; i++)
       {
         printf("                         [%2d] %.8lx.%.8lx\n", i,
          (long)ntsHeader.OptionalHeader.DataDirectory[i].VirtualAddress,
          (long)ntsHeader.OptionalHeader.DataDirectory[i].Size);
       }
     }
   }

   //-------------------------------------------------------------------------
   // Validate it
   //-------------------------------------------------------------------------
   if( ntsHeader.Signature != IMAGE_NT_SIGNATURE )
     result= 1;

   //-------------------------------------------------------------------------
   // Set control information
   //-------------------------------------------------------------------------
   sectCount= ntsHeader.FileHeader.NumberOfSections;
   if( sectCount > MAX_SECTIONS )
     result= 1;

   //-------------------------------------------------------------------------
   // Read the section information
   //-------------------------------------------------------------------------
   if( result == 0 )
   {
     S= dosHeader.e_lfanew;
     S += FIELD_OFFSET(IMAGE_NT_HEADERS, OptionalHeader);
     S += ntsHeader.FileHeader.SizeOfOptionalHeader;
     if( seekf(S) != 0 )
       return 1;

     memset(string, 0, sizeof(string));
     for(unsigned i= 0; i<sectCount; i++)
     {
       L= readf(&section[i], sizeof(section[i]));
       if( L <= 0 )
         return 1;
       if( sw_verbose )
       {
         printf("\n");
         memcpy(string, section[i].Name, sizeof(section[i].Name));
         printf("section[%d]: %s\n", i, string);
         INTFIELD(section[i].Misc.PhysicalAddress);
         INTFIELD(section[i].Misc.VirtualSize);
         INTFIELD(section[i].VirtualAddress);
         INTFIELD(section[i].SizeOfRawData);
         INTFIELD(section[i].PointerToRawData);
         INTFIELD(section[i].PointerToRelocations);
         INTFIELD(section[i].PointerToLinenumbers);
         INTFIELD(section[i].NumberOfRelocations);
         INTFIELD(section[i].NumberOfLinenumbers);
         INTFIELD(section[i].Characteristics);
       }
     }
   }

   return result;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       dumpobj
//
// Purpose-
//       Dump the object file.
//
//----------------------------------------------------------------------------
static int                          // Return code
   dumpobj( void )                  // Parse an object file
{
   //-------------------------------------------------------------------------
   // Read the file header
   //-------------------------------------------------------------------------
   if( readHeader() != 0 )
     return 1;

   //-------------------------------------------------------------------------
   // Done
   //-------------------------------------------------------------------------
   return 0;
}
#endif // defined(_OS_WIN) || defined(_OS_CYGWIN)

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
     int               argc,        // Argument count
     char*             argv[])      // Argument array
{
#if defined(_OS_WIN) || defined(_OS_CYGWIN)
   // Welcome message
   printf("%s Compiled Date(%s) Time(%s)\n", __SOURCE__, __DATE__, __TIME__);

   // Initialization
   parm(argc, argv);

   // Parse the object files
   for(int i= fileArgc; i<argc; i++)
   {
     fileName= argv[i];
     fileHand= fopen(fileName, "rb");     // Open the file
     if( fileHand == NULL )
     {
       fprintf(stderr, "File(%s): ", fileName);
       perror("Open failure: ");
       continue;
     }

     if( i != fileArgc )
       printf("\n");
     printf("File(%s)\n", fileName);

     dumpobj();

     fclose(fileHand);
   }
#else  // defined(_OS_WIN) || defined(_OS_CYGWIN)
   (void)argc; (void)argv;          // (UNUSED)
   fprintf(stderr, "WINDOWS or CYGWIN only!\n");
#endif // defined(_OS_WIN) || defined(_OS_CYGWIN)

   return 0;
}

