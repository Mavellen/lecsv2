#define TESTS

#include "lecs.h"

#ifdef TESTS
#include "tests.cpp"
#endif
using namespace ls::lecs;



// TODO Relationships
// TODO INVALIDATION MECHANISMS
// TODO EVENTS LIKE ADDED COMPONENT

int main()
{
#ifdef TESTS
  test_suite();
  return 1;
#endif

}