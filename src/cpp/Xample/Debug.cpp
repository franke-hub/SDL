#include <pub/Debug.h>
#define PUB _LIBPUB_NAMESPACE
using namespace PUB::debugging;

int main() {
   debugf("This appears in %s and %s\n", "TRACE", "STDOUT");
   errorf("This appears in %s and %s\n", "TRACE", "STDERR");
   tracef("This ONLY appears in %s\n",   "TRACE");
   debugh("This appears in %s and %s\n", "TRACE", "STDOUT");
   errorh("This appears in %s and %s\n", "TRACE", "STDERR");
   traceh("This ONLY appears in %s\n",   "TRACE");
}
