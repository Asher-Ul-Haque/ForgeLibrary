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

// - - - | Functions | - - - 


// - - - Sizes - - -

static const u8 tuiGetWidth()  { return tuiGetContext()->width;  }
static const u8 tuiGetHeight() { return tuiGetContext()->height; }



// - - - Forward Declarations 
void handleResize(int SIGNAL)
{
  struct winsize ws;
  TUIContext* ctx = tuiGetContext();
  if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == 0) 
  {
    ctx->width  = (ws.ws_col < MAX_WIDTH) ? ws.ws_col : MAX_WIDTH;
    ctx->height = (ws.ws_row < MAX_HEIGHT) ? ws.ws_row : MAX_HEIGHT;
  }
}

// - - - Init and Quit
void tuiInit(u8 WIDTH, u8 HEIGHT)
{
  TUIContext* ctx = tuiGetContext();
  FORGE_ASSERT_MESSAGE(ctx->width + ctx->height == 0, "[TUI] : Already initialized");
  FORGE_LOG_INFO("[TUI] : Initializing Terminal User Interface");

  std::cout << "\033[?1049h\033[?25l";
  std::cout.flush();

  tcgetattr(STDIN_FILENO, &ctx->origTermios);

  struct termios raw = ctx->origTermios;
  raw.c_iflag    &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
  raw.c_oflag    &= ~(OPOST);
  raw.c_cflag    |=  (CS8);
  raw.c_lflag    &= ~(ECHO | ICANON | IEXTEN | ISIG);
  raw.c_cc[VMIN]  = 0;  
  raw.c_cc[VTIME] = 0;

  tcsetattr(STDIN_FILENO, TCSANOW, &raw);

  // size
  if (WIDTH == 0 || HEIGHT == 0)
  {
    FORGE_LOG_WARNING("[TUI] : Width/Height is 0; using terminal size");
    handleResize(0);
  }
  else
  {
    ctx->width  = (WIDTH  < MAX_WIDTH)  ? WIDTH  : MAX_WIDTH;
    ctx->height = (HEIGHT < MAX_HEIGHT) ? HEIGHT : MAX_HEIGHT;
  }
  std::signal(SIGWINCH, handleResize);

  ctx->lastTime              = clock();
  ctx->detlaTime             = 0;
  ctx->frames                = 0;
  ctx->frameRate             = 0.0;
  ctx->averageFrameTimeMilli = 0.0;

  std::atexit(tuiShutdown);

  FORGE_LOG_INFO("[TUI] : Graceful initialization");
}

void tuiShutdown()
{
  TUIContext* ctx = tuiGetContext();
  FORGE_LOG_WARNING("[TUI] : Shutting Down");

  tcsetattr(STDIN_FILENO, TCSANOW, &ctx->origTermios);

  std::cout << "\033[?25h\033[?1049l";
  std::cout.flush();

  FORGE_LOG_INFO("[TUI] : Graceful shutdown complete");
}

void clear() 
{  std::cout << "\033[H"; }

void drawBorder()
{
  TUIContext* ctx = tuiGetContext();
  std::cout << "+" << std::string(ctx->width - 2, '-') << "+\n";

  for (int i = 0; i < ctx->height - 2; i++)
    std::cout << "|" << std::string(ctx->width - 2, ' ') << "|\n";

  std::cout << "+" << std::string(ctx->width - 2, '-') << "+\n";
}

static void updateStats()
{
  TUIContext* ctx = tuiGetContext();
  clock_t     currentTime = clock();
  ctx->detlaTime          = currentTime - ctx->lastTime;
  ctx->lastTime           = currentTime;

  f64 deltaSeconds = static_cast<f64>(ctx->detlaTime) / CLOCKS_PER_SEC;

  ctx->frames++;
  if (deltaSeconds > 0.0) 
  {
    ctx->frameRate = 1.0 / deltaSeconds;
    ctx->averageFrameTimeMilli = (deltaSeconds * 1000.0);
  }
}


static std::string keyMessage = "";
static void updateKeyMessage()
{
  tuiPollKeyboard();

  if (isKeyPressed(KeyCode::KEY_ALPHA_A))       keyMessage = "A pressed";
  else if (isKeyPressed(KeyCode::KEY_SPACE))    keyMessage = "SPACE pressed";
  else if (isKeyPressed(KeyCode::KEY_ESC))      keyMessage = "ESC pressed (exit)";
  else if (isKeyPressed(KeyCode::KEY_NAV_HOME)) keyMessage = "HOME pressed";
  else if (isKeyPressed(KeyCode::KEY_FUNC_F11)) keyMessage = "Fn 11 pressed";
  else if (isKeyPressed(KeyCode::KEY_NUM_2))    keyMessage = "2 pressed";

  if (isKeyPressed(KeyCode::KEY_ESC)) 
  {
    std::cout << "\033[0m\033[?25h"; 
    exit(0);
  }
}
void tuiUpdate()
{
  clear();
  updateStats();
  updateKeyMessage();

  drawBorder();

  TUIContext* ctx = tuiGetContext();
  std::cout << "Updates: " << ctx->frames << "\n";
  std::cout << "FPS: " << ctx->frameRate << "\n";
  std::cout << "Keyboard: " << keyMessage << "\n";

  std::cout.flush();
}

static TUIContext ctx;
TUIContext* tuiGetContext() { return &ctx; }
