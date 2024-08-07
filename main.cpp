#define TESTS

#include "lecs.h"

#ifdef TESTS
#include "tests.cpp"
#endif
using namespace ls::lecs;





int main()
{
#ifdef TESTS
  test_suite();
  return 1;
#endif

}