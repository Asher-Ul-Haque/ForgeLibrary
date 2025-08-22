#include "../Libraries/Forge/include/testManager.h"
#include "../Libraries/Forge/include/logger.h"
#include "../Libraries/Forge/include/asserts.h"
#include "../Libraries/Forge/include/tui.h"
#include <unistd.h>

u8 testTuiKey()
{
  tuiInit(0, 0);

  for (int i = 0 ; i < 50 * 50; ++i)
  {
    tuiPollKeyboard();

    if (isKeyPressed(KEY_ALPHA_A))      FORGE_LOG_INFO("Key A is presed");
    if (isKeyReleased(KEY_ALPHA_A))     FORGE_LOG_INFO("Key A is released");
    
    if (isKeyPressed(KEY_NUM_1))        FORGE_LOG_INFO("Key 1 is presed");
    if (isKeyReleased(KEY_NUM_1))       FORGE_LOG_INFO("Key 1 is released");
    
    if (isKeyPressed(KEY_NAV_UP))       { FORGE_LOG_INFO("Key UP is presed"); }
    if (isKeyReleased(KEY_NAV_UP))      FORGE_LOG_INFO("Key UP is released");
    
    if (isKeyPressed(KEY_ENTER))        FORGE_LOG_INFO("Key Enter is presed"); 
    if (isKeyReleased(KEY_ENTER))       FORGE_LOG_INFO("Key Enter is released");

    usleep(2000);
  }

  tuiShutdown();
  return 2;
}

int main(int argc, char *argv[])
{
  registerTest(testTuiKey, "Test keyboard");
  runTests();
}
