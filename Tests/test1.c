#include "../Libraries/Forge/include/testManager.h"
#include <stdlib.h>

u8 testSomething()
{
  malloc(50000000000000000);
  free(0);
  return *(u8*)(NULL + 2);
}

int main(int argc, char *argv[])
{
  registerTest(testSomething, "We are testing something");
  runTests();
}
