#include "../Libraries/Forge/include/testManager.h"
#include "../Libraries/Forge/include/logger.h"
#include "../Libraries/Forge/include/asserts.h"
#include <stdlib.h>

u8 test1()
{
  int arr[3];
  for (int i = 0; i < 3; ++i)
  {
    arr[i] = rand();
    FORGE_LOG_INFO("Generated random value : %d", arr[i]);
  }

  return 1; // - - - test fail
}

u8 test2()
{
  int arr[3];
  for (int i = 0; i < 30; ++i)
  {
    arr[i] = rand();
    FORGE_LOG_INFO("Generated random value : %d", arr[i]);
  }

  FORGE_ASSERT_MESSAGE(1 + 1 == 3, "zabardasti");

  return 2; // - - - test pass 
}

int main(int argc, char *argv[])
{
  registerTest(test1, "Aniket bhai ka test 1");
  registerTest(test2, "Aniket bhai ka test 2");
  runTests();
}
