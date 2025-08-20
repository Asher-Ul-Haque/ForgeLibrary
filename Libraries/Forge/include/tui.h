#pragma once
#include "defines.h"
#ifdef __cplusplus
extern "C" {
#endif

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
  KEY_NAV_UP     = 256,
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
  KEY_FUNCTION_F1, KEY_FUNCTION_F2, KEY_FUNCTION_F3, KEY_FUNCTION_F4,
  KEY_FUNCTION_F5, KEY_FUNCTION_F6, KEY_FUNCTION_F7, KEY_FUNCTION_F8,
  KEY_FUNCTION_F9, KEY_FUNCTION_F10, KEY_FUNCTION_F11, KEY_FUNCTION_F12,

  // - - - Letters
  KEY_ALPHA_A = 'A', KEY_ALPHA_B = 'B', KEY_ALPHA_C = 'C', KEY_ALPHA_D = 'D',
  KEY_ALPHA_E = 'E', KEY_ALPHA_F = 'F', KEY_ALPHA_G = 'G', KEY_ALPHA_H = 'H',
  KEY_ALPHA_I = 'I', KEY_ALPHA_J = 'J', KEY_ALPHA_K = 'K', KEY_ALPHA_L = 'L',
  KEY_ALPHA_M = 'M', KEY_ALPHA_N = 'N', KEY_ALPHA_O = 'O', KEY_ALPHA_P = 'P',
  KEY_ALPHA_Q = 'Q', KEY_ALPHA_R = 'R', KEY_ALPHA_S = 'S', KEY_ALPHA_T = 'T',
  KEY_ALPHA_U = 'U', KEY_ALPHA_V = 'V', KEY_ALPHA_W = 'W', KEY_ALPHA_X = 'X',
  KEY_ALPHA_Y = 'Y', KEY_ALPHA_Z = 'Z',

  // - - - Numbers
  KEY_NUM_0 = '0', KEY_NUM_1 = '1', KEY_NUM_2 = '2', KEY_NUM_3 = '3',
  KEY_NUM_4 = '4', KEY_NUM_5 = '5', KEY_NUM_6 = '6', KEY_NUM_7 = '7',
  KEY_NUM_8 = '8', KEY_NUM_9 = '9'
} KeyCode;

// - - - Usage 
void tuiInit(i16 WIDTH, i16 HEIGHT);
void tuiUpdate();
void tuiShutdown();

// - - - Getters and Setters
FORGE_API static const i16 tuiGetWidth();
FORGE_API static const i16 tuiGetHeight();

// - - - keyboard 
FORGE_API static const bool isKeyPressed (KeyCode KEY);
FORGE_API static const bool isKeyReleased(KeyCode KEY);

#ifdef __cplusplus
}
#endif
