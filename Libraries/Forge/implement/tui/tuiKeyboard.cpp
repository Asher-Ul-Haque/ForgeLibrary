#include "../../include/tui.h"
#include <csignal>
#include <cstring>
#include <sys/ioctl.h>
#include <unistd.h>
#include <termios.h>


static FORGE_INLINE void setKey(u64* BUFFER, KeyCode KEY, bool ON)
{
  i32 idx = KEY / 64;
  i32 bit = KEY % 64;
  BIT_SET(BUFFER[idx], bit, ON);
}

static FORGE_INLINE bool getKey(u64* BUFFER, KeyCode KEY)
{
  i32 idx = KEY / 64;
  i32 bit = KEY % 64;
  return BIT(BUFFER[idx], bit);
}

const void tuiPollKeyboard()
{
  TUIContext* ctx = tuiGetContext();
  memcpy(ctx->prevDown, ctx->currentDown, sizeof(ctx->currentDown));
  memset(ctx->currentDown, 0, sizeof(ctx->currentDown));

  u8 ch;
  while (read(STDIN_FILENO, &ch, 1) == 1)
  {
    if ((ch >= 0x20 && ch <= 0x7E) || ch == '\r' || ch == '\n' || ch == '\t' || ch == 0x7F && ch != 0x1B )
    {
      setKey(ctx->currentDown, (KeyCode) ch, true);
    }
    else if (ch <= KeyCode::KEY_MAX)
    {
      setKey(ctx->currentDown, (KeyCode) ch, true);
    }
  }
}

const bool isKeyPressed(KeyCode KEY)
{ return getKey(tuiGetContext()->currentDown, KEY); }

const bool isKeyReleased(KeyCode KEY)
{
  return !getKey(tuiGetContext()->currentDown, KEY) && 
          getKey(tuiGetContext()->prevDown, KEY);
}
