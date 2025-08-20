#include "Libraries/Forge/include/tui.h"
#include <chrono>
#include <thread>

int main (int argc, char *argv[]) 
{
  init(0, 0);

  for (int i = 0; i < 50; ++i)
  {
    update();
    std::this_thread::sleep_for(std::chrono::milliseconds(16));    
  }

  shutdown();
}
