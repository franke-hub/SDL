#include <cstdio>                   // For EOF
#include <exception>                // For std::runtime_error
#include "pub/Utf.i"                // Import Utf.h symbols into namespace

enum { DIM= 512 };                  // Input/output buffer size

static void output(                 // Convert and output UTF-8 string
   const char*         text,        // The UTF-8 string
   int                 size)        // Of this BYTE length
{
   utf16_t             out[DIM];    // UTF-16 output buffer
   utf16_encoder       encoder(out, DIM, MODE_BE);

   encoder= utf8_decoder(text, size);
   if( encoder.encode('\n') == 0 )
     throw pub::utf_overflow_error("Output buffer too small");

   int M= (int)(encoder.get_offset() * sizeof(utf16_t));
   for(int i= 0; i < M; ++i) {
     int C= putchar(((char*)out)[i]);
     if( C < 0 )                    // If error (not (unsigned char)text[i])
       throw std::runtime_error("putchar EOF");
   }
}

int main() {
   char                inp[DIM];    // UTF-8 input buffer

   putchar(0xFE);                   // Write big-endian Byte Order Mark
   putchar(0xFF);                   // "

   for(;;) {                        // Read and convert lines
     for(int inp_ix= 0; ;) {
       int C= getchar();
       if( C == EOF ) {
         if( inp_ix )
           output(inp, inp_ix);
         return 0;
       }

       if( C == '\r' )              // Ignore '\r'
         continue;
       if( C == '\n' ) {            // If end of line
         output(inp, inp_ix);
         break;
       }

       if( inp_ix >= DIM )
         throw pub::utf_overflow_error("Input buffer too small");
       inp[inp_ix++]= C;
     }
   }
}
