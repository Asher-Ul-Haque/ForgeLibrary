#include "../../include/tui.h"
#include <iostream>
#include <chrono>
#include <csignal>
#include "../../include/asserts.h"
#include "../../include/logger.h"
#include <time.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <termios.h>


// - - - Global State - - -

// - - - width and height 
static i16 width  = 0;
static i16 height = 0;

// - - - framebuffer
static const i16 MAX_WIDTH  = 256;
static const i16 MAX_HEIGHT = 128;
static i8 frameBuffer[MAX_HEIGHT][MAX_WIDTH];

// - - - fps 
clock_t detlaTime               = 0;
u32     frames                  = 0;
f64     frameRate               = 30;
f64     averageFrameTimeMilli   = 33.333;


// - - - keyboard
static u8 prevDown[32];
static u8 currentDown[32];

// - - - termios 
static struct termios origTermios;


// - - - | Functions | - - - 


// - - - Sizes - - -

static const i16 tuiGetWidth()  { return width;  }
static const i16 tuiGetHeight() { return height; }


// - - - Keyboard - - - 

static FORGE_INLINE bool getKey(u8 CACHE[32], KeyCode KEY)
{
  i32 index = KEY >> 3;
  i32 bit   = KEY & 7;
  return BIT(CACHE[index], bit);
}

static const bool isKeyPressed(KeyCode KEY)
{
  return getKey(currDown, KEY);
}



// - - - Forward Declarations 
void handleResize(int SIGNAL);

// - - - Init and Quit
void tuiInit(i16 WIDTH, i16 HEIGHT)
{
  FORGE_ASSERT_MESSAGE(width + height == 0, "[TUI] : Already has been initailized");  
  FORGE_LOG_INFO("[TUI] : Intializing Terminal User Interface");
    
  // - - - hide cursor 
  std::cout << "\033[?1049h\033[?25l"; 
  std::cout.flush();

  // - - - raw input mode 
  struct termios raw;
  tcgetattr(STDIN_FILENO, &origTermios);
  raw = origTermios;

  raw.c_lflag    &= ~(ICANON | ECHO);   // - - - no line buffering, no echo
  raw.c_cc[VMIN]  = 0;                  // - - - non-blocking read
  raw.c_cc[VTIME] = 0;

  tcsetattr(STDIN_FILENO, TCSANOW, &raw);

  if (WIDTH == 0 || HEIGHT == 0) handleResize(0);
  else 
  {
    FORGE_LOG_WARNING("[TUI] : Height or Width specified as 0, going for terminal size");
    width  = (WIDTH < MAX_WIDTH)   ? WIDTH  : MAX_WIDTH;
    height = (HEIGHT < MAX_HEIGHT) ? HEIGHT : MAX_HEIGHT;  
  }
  std::signal(SIGWINCH, handleResize);
  FORGE_LOG_INFO("[TUI] : Graceful initialization")  
}

void tuiShutdown()
{
  FORGE_LOG_WARNING("[TUI] : Shutting Down");
  // - - - restore normal input mode   
  tcsetattr(STDIN_FILENO, TCSANOW, &origTermios);

  // - - - restore screen + cursor  
  std::cout << "\033[?25h\033[?1049l"; 
  std::cout.flush();  
  FORGE_LOG_INFO("[TUI] : Graceful shutdown complete");
}

void tuiUpdate()
{}
