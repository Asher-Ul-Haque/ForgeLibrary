#pragma once
#include "defines.h"
#include <termios.h>
#ifdef __cplusplus
extern "C" {
#endif

// - - - framebuffer
#define MAX_WIDTH  255
#define MAX_HEIGHT 127

typedef enum KeyCode 
{
  // - - - ASCII control
  KEY_NULL       = 0,
  KEY_ESC        = 27,
  KEY_SPACE      = 32,
  KEY_ENTER      = 13,
  KEY_TAB        = 9,
  KEY_BACKSPACE  = 127,

  // - - - Page / navigation
  KEY_NAV_UP     = 128,
  KEY_NAV_DOWN,
  KEY_NAV_LEFT,
  KEY_NAV_RIGHT,
  KEY_NAV_HOME,
  KEY_NAV_END,
  KEY_NAV_PAGE_UP,
  KEY_NAV_PAGE_DOWN,
  KEY_NAV_INSERT,
  KEY_NAV_DELETE,

  // - - - Function keys
  KEY_FUNC_F1, KEY_FUNC_F2, KEY_FUNC_F3, KEY_FUNC_F4,
  KEY_FUNC_F5, KEY_FUNC_F6, KEY_FUNC_F7, KEY_FUNC_F8,
  KEY_FUNC_F9, KEY_FUNC_F10, KEY_FUNC_F11, KEY_FUNC_F12,

  // - - - Letters
  KEY_ALPHA_A = 'a', KEY_ALPHA_B = 'b', KEY_ALPHA_C = 'c', KEY_ALPHA_D = 'd',
  KEY_ALPHA_E = 'e', KEY_ALPHA_F = 'f', KEY_ALPHA_G = 'g', KEY_ALPHA_H = 'h',
  KEY_ALPHA_I = 'i', KEY_ALPHA_J = 'j', KEY_ALPHA_K = 'k', KEY_ALPHA_L = 'l',
  KEY_ALPHA_M = 'm', KEY_ALPHA_N = 'n', KEY_ALPHA_O = 'o', KEY_ALPHA_P = 'p',
  KEY_ALPHA_Q = 'q', KEY_ALPHA_R = 'r', KEY_ALPHA_S = 's', KEY_ALPHA_T = 't',
  KEY_ALPHA_U = 'u', KEY_ALPHA_V = 'v', KEY_ALPHA_W = 'w', KEY_ALPHA_X = 'x',
  KEY_ALPHA_Y = 'y', KEY_ALPHA_Z = 'z',

  // - - - Numbers
  KEY_NUM_0 = '0', KEY_NUM_1 = '1', KEY_NUM_2 = '2', KEY_NUM_3 = '3',
  KEY_NUM_4 = '4', KEY_NUM_5 = '5', KEY_NUM_6 = '6', KEY_NUM_7 = '7',
  KEY_NUM_8 = '8', KEY_NUM_9 = '9',

  KEY_MAX
} KeyCode;


typedef struct 
{
  u8 width;
  u8 height;

  struct termios origTermios;

  u64 prevDown[4];
  u64 currentDown[4];

  i8 frameBuffer[MAX_HEIGHT][MAX_WIDTH];

  clock_t lastTime;
  clock_t detlaTime;
  u32     frames;
  f64     frameRate;
  f64     averageFrameTimeMilli;
} TUIContext;

TUIContext* tuiGetContext();

// - - - Usage 
FORGE_API void tuiInit(u8 WIDTH, u8 HEIGHT);
FORGE_API void tuiUpdate();
FORGE_API void tuiShutdown();

// - - - Getters and Setters
FORGE_API static const u8 tuiGetWidth();
FORGE_API static const u8 tuiGetHeight();

// - - - keyboard 
FORGE_API const void tuiPollKeyboard();
FORGE_API const bool isKeyPressed (KeyCode KEY);
FORGE_API const bool isKeyReleased(KeyCode KEY);

#ifdef __cplusplus
}
#endif
