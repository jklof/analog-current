#include <limits.h>
#include <UTFTGLUE.h>              //use GLUE class and constructor
UTFTGLUE myGLCD(0,A2,A1,A3,A4,A0); //all dummy args

#include "vfont.h"

const struct {
  const char chr;
  const int16_t *lines;
  const size_t size;
} font_map[] = {
  {'.', Number_dot_Lines, sizeof(Number_dot_Lines) / sizeof(Number_dot_Lines[0])},
  {'+', Number_plus_Lines, sizeof(Number_plus_Lines) / sizeof(Number_plus_Lines[0])},
  {'-', Number_minus_Lines, sizeof(Number_minus_Lines) / sizeof(Number_minus_Lines[0])},
  {':', Number_dot2_Lines, sizeof(Number_dot2_Lines) / sizeof(Number_dot2_Lines[0])},

  {'0', Number_0_Lines, sizeof(Number_0_Lines) / sizeof(Number_0_Lines[0])},
  {'1', Number_1_Lines, sizeof(Number_1_Lines) / sizeof(Number_1_Lines[0])},
  {'2', Number_2_Lines, sizeof(Number_2_Lines) / sizeof(Number_2_Lines[0])},
  {'3', Number_3_Lines, sizeof(Number_3_Lines) / sizeof(Number_3_Lines[0])},
  {'4', Number_4_Lines, sizeof(Number_4_Lines) / sizeof(Number_4_Lines[0])},
  {'5', Number_5_Lines, sizeof(Number_5_Lines) / sizeof(Number_5_Lines[0])},
  {'6', Number_6_Lines, sizeof(Number_6_Lines) / sizeof(Number_6_Lines[0])},
  {'7', Number_7_Lines, sizeof(Number_7_Lines) / sizeof(Number_7_Lines[0])},
  {'8', Number_8_Lines, sizeof(Number_8_Lines) / sizeof(Number_8_Lines[0])},
  {'9', Number_9_Lines, sizeof(Number_9_Lines) / sizeof(Number_9_Lines[0])},
  {'A', Letter_A_Lines, sizeof(Letter_A_Lines) / sizeof(Letter_A_Lines[0])},
  {'B', Letter_B_Lines, sizeof(Letter_B_Lines) / sizeof(Letter_B_Lines[0])},
  {'C', Letter_C_Lines, sizeof(Letter_C_Lines) / sizeof(Letter_C_Lines[0])},
  {'D', Letter_D_Lines, sizeof(Letter_D_Lines) / sizeof(Letter_D_Lines[0])},
  {'E', Letter_E_Lines, sizeof(Letter_E_Lines) / sizeof(Letter_E_Lines[0])},
  {'F', Letter_F_Lines, sizeof(Letter_F_Lines) / sizeof(Letter_F_Lines[0])},
  {'G', Letter_G_Lines, sizeof(Letter_G_Lines) / sizeof(Letter_G_Lines[0])},
  {'H', Letter_H_Lines, sizeof(Letter_H_Lines) / sizeof(Letter_H_Lines[0])},
  {'I', Letter_I_Lines, sizeof(Letter_I_Lines) / sizeof(Letter_I_Lines[0])},
  {'J', Letter_J_Lines, sizeof(Letter_J_Lines) / sizeof(Letter_J_Lines[0])},
  {'K', Letter_K_Lines, sizeof(Letter_K_Lines) / sizeof(Letter_K_Lines[0])},
  {'L', Letter_L_Lines, sizeof(Letter_L_Lines) / sizeof(Letter_L_Lines[0])},
  {'M', Letter_M_Lines, sizeof(Letter_M_Lines) / sizeof(Letter_M_Lines[0])},
  {'N', Letter_N_Lines, sizeof(Letter_N_Lines) / sizeof(Letter_N_Lines[0])},
  {'O', Letter_O_Lines, sizeof(Letter_O_Lines) / sizeof(Letter_O_Lines[0])},
  {'P', Letter_P_Lines, sizeof(Letter_P_Lines) / sizeof(Letter_P_Lines[0])},
  {'Q', Letter_Q_Lines, sizeof(Letter_Q_Lines) / sizeof(Letter_Q_Lines[0])},
  {'R', Letter_R_Lines, sizeof(Letter_R_Lines) / sizeof(Letter_R_Lines[0])},
  {'S', Letter_S_Lines, sizeof(Letter_S_Lines) / sizeof(Letter_S_Lines[0])},
  {'T', Letter_T_Lines, sizeof(Letter_T_Lines) / sizeof(Letter_T_Lines[0])},
  {'U', Letter_U_Lines, sizeof(Letter_U_Lines) / sizeof(Letter_U_Lines[0])},
  {'V', Letter_V_Lines, sizeof(Letter_V_Lines) / sizeof(Letter_V_Lines[0])},
  {'W', Letter_W_Lines, sizeof(Letter_W_Lines) / sizeof(Letter_W_Lines[0])},
  {'X', Letter_X_Lines, sizeof(Letter_X_Lines) / sizeof(Letter_X_Lines[0])},
  {'Y', Letter_Y_Lines, sizeof(Letter_Y_Lines) / sizeof(Letter_Y_Lines[0])},
  {'Z', Letter_Z_Lines, sizeof(Letter_Z_Lines) / sizeof(Letter_Z_Lines[0])},
  {'$', Symbol_ce_kwh_Lines, sizeof(Symbol_ce_kwh_Lines) / sizeof(Symbol_ce_kwh_Lines[0])},
};
int drawletter(char chr, int x, int y, int scale) {

  if (chr == ' ')
    return (pgm_read_word(&Letter_X_Lines[0]) * scale) / 127;

  const int16_t *lines = font_map[0].lines;
  const int16_t *end  = lines + font_map[0].size;
  for (unsigned i=0; i< sizeof(font_map)/sizeof(font_map[0]); i++) {
    if ( font_map[i].chr == chr) {
      lines = font_map[i].lines;
      end = lines + font_map[i].size;
      break;
    }
  }
  uint16_t horiz_advance = (pgm_read_word(lines++) * scale) / 127;
  while (lines < end) {
    uint16_t v1 = pgm_read_word(lines + 0)*2;
    uint16_t v2 = pgm_read_word(lines + 1)*2;
    const int ax = (int(int8_t(pgm_read_byte(vertices+v1+0))) * scale) / 127;
    const int ay = (int(int8_t(pgm_read_byte(vertices+v1+1))) * scale) / 127;
    const int bx = (int(int8_t(pgm_read_byte(vertices+v2+0))) * scale) / 127;
    const int by = (int(int8_t(pgm_read_byte(vertices+v2+1))) * scale) / 127;
    myGLCD.drawLine(ax+x,ay+y, bx+x, by+y);
    lines += 2;
  }
  return horiz_advance;
}
int drawtext(const char *text, int x, int y, int scale)
{
  int hz_advance = 0;
  while (*text) {
    hz_advance += drawletter(*text, x+hz_advance, y, scale);
    text++;
  }
  return hz_advance;
}




struct Console {
  bool enabled;
  int yline;
  Console() {
    enabled = true;
    yline = 0;
  }
  void print(char *text) {
    if (!enabled)
      return;
    myGLCD.print(text, 0, yline);
    yline += 10;
      if (yline >= 320)
        yline = 0;
  }
  void hex(uint16_t u) {
    if (!enabled)
      return;
    const char h[] = "0123456789abcdef";
    char s[5];
    s[0] = h[(u>>12)&0xf];
    s[1] = h[(u>>8)&0xf];
    s[2] = h[(u>>4)&0xf];
    s[3] = h[(u>>0)&0xf];
    s[4] = 0;
    print(s);
  }
  void debug(bool en) {
    enabled = en;
    yline = 0;
  }
} console;


struct cmdbuf {
  int read;
  uint8_t size;
  uint8_t data[256];
  cmdbuf() {
    size = 0;
    read = -1;
  }
  // add byte to buffer, return true if full message was received
  bool add(uint8_t byte) {
    // begin new cmd
    if (read < 0) {
      size = unsigned(byte);
      read = 0;
    } else {
      // add to buffer
      data[read++] = byte;
    }
    // got full message or full buffer minus 1 byte for nul
    if ( (read >= size) || (read >= sizeof(data)-1 ) ) {
      read = -1; // next time start a new msg
      return true; // received full msg
    }
    return false; // still waiting for bytes
  }
};

struct cmd : cmdbuf {

  enum {
    clear = 1,
    color = 2,
    bgcolor = 3,
    box = 4,
    line = 5,
    print = 6,
    font = 7,
    rect = 8,
    circle = 9,
    fillcircle = 10,
    vtext = 11,

    sync = 0xfe,
    console = 0xff

  };

  // read all available bytes from serial
  // stop and return true if full message was
  // received
  bool readFromSerial();

  void dispatch();

} command;


bool cmd::readFromSerial() {
  int byte;
  while ( (byte = Serial.read()) >= 0) {
    ::console.hex(byte);
    if ( add(byte) ) {
      return true;
    }
  }
  return false;
}


void cmd::dispatch() {
  uint8_t *p = data;
  if (size >= 1) {
    uint8_t cmd = *p++;
    switch (cmd) {

      case sync:
        break;

      case clear:
        if (size == 1) {
          myGLCD.clrScr();
        }
        break;

      case color:
        if (size ==  1 + 3) {
          myGLCD.setColor(p[0],p[1],p[2]);
        }
        break;

      case bgcolor:
        if (size == 1 + 3) {
          myGLCD.setBackColor(p[0],p[1],p[2]);
        }
        break;

      case box:
        if (size == 1 + 4*2) {
          uint16_t x0 = (uint16_t(p[0])<<8) | uint16_t(p[1]);
          uint16_t y0 = (uint16_t(p[2])<<8) | uint16_t(p[3]);
          uint16_t x1 = (uint16_t(p[4])<<8) | uint16_t(p[5]);
          uint16_t y1 = (uint16_t(p[6])<<8) | uint16_t(p[7]);
          myGLCD.fillRect(x0,y0,x1,y1);
        }
        break;

      case line:
        if (size == 1 + 4*2) {
          uint16_t x0 = (uint16_t(p[0])<<8) | uint16_t(p[1]);
          uint16_t y0 = (uint16_t(p[2])<<8) | uint16_t(p[3]);
          uint16_t x1 = (uint16_t(p[4])<<8) | uint16_t(p[5]);
          uint16_t y1 = (uint16_t(p[6])<<8) | uint16_t(p[7]);
          myGLCD.drawLine(x0,y0,x1,y1);
        }
        break;

      case print:
        if (size > 1 + 2*2) {
          uint16_t x0 = (uint16_t(p[0])<<8) | uint16_t(p[1]);
          uint16_t y0 = (uint16_t(p[2])<<8) | uint16_t(p[3]);
          char* text = (char*)(p+4);
          data[size-1] = 0; // enforce null
          myGLCD.print(text,x0,y0);
        }
        break;

      case vtext:
        if (size > 1 + 3*2) {
          uint16_t x0 = (uint16_t(p[0])<<8) | uint16_t(p[1]);
          uint16_t y0 = (uint16_t(p[2])<<8) | uint16_t(p[3]);
          uint16_t sc = (uint16_t(p[4])<<8) | uint16_t(p[5]); // scale
          char* text = (char*)(p+6);
          data[size-1] = 0; // enforce null
          drawtext(text,x0,y0, sc);
        }
        break;

#if 0 // avoid pulling fonts code to ROM
      case font:
        if (size ==  1+1) {
          switch(p[0]) {
            case 0:
              myGLCD.setFont(SmallFont);
              break;
            case 1:
              myGLCD.setFont(BigFont);
              break;
            case 2:
              myGLCD.setFont(SevenSegNumFont);
              break;
          }
        }
        break;
#endif

      case rect:
        if (size == 1 + 4*2) {
          uint16_t x0 = (uint16_t(p[0])<<8) | uint16_t(p[1]);
          uint16_t y0 = (uint16_t(p[2])<<8) | uint16_t(p[3]);
          uint16_t x1 = (uint16_t(p[4])<<8) | uint16_t(p[5]);
          uint16_t y1 = (uint16_t(p[6])<<8) | uint16_t(p[7]);
          myGLCD.drawRect(x0,y0,x1,y1);
        }
        break;

      case circle:
        if (size == 1 + 3*2) {
          uint16_t x = (uint16_t(p[0])<<8) | uint16_t(p[1]);
          uint16_t y = (uint16_t(p[2])<<8) | uint16_t(p[3]);
          uint16_t r = (uint16_t(p[4])<<8) | uint16_t(p[5]);
          myGLCD.drawCircle(x,y,r);
        }
        break;

      case fillcircle:
        if (size == 1 + 3*2) {
          uint16_t x = (uint16_t(p[0])<<8) | uint16_t(p[1]);
          uint16_t y = (uint16_t(p[2])<<8) | uint16_t(p[3]);
          uint16_t r = (uint16_t(p[4])<<8) | uint16_t(p[5]);
          myGLCD.fillCircle(x,y,r);
        }
        break;


      case console:
        if (size ==  1+1) {
          ::console.debug(p[0]);
        }
        break;

      default:
        break;

    } // switch
    Serial.write(cmd);
    //Serial.flush();
  } // if
}



unsigned long previousMillis = 0;
const long interval = 65*60*1000;  // 1 hour 5 min

void disconnected()
{
  // draw disconnected message
  myGLCD.setColor(255,255,255);
  myGLCD.clrScr();
  myGLCD.setColor(0xff,0xff,0xff);
  drawtext("DISCONNECTED..", 0, 100, 100);
}

void setup() {
  // put your setup code here, to run once:
  myGLCD.InitLCD();
  myGLCD.setFont(SmallFont);
  // set up serial
  Serial.begin(115200);

  // draw ready message
  myGLCD.clrScr();
  myGLCD.setColor(0xff,0xff,0xff);
  drawtext("READY..", 0, 100, 100);

  previousMillis = millis();
}

void loop() {
  unsigned long currentMillis = millis();
  if (command.readFromSerial()) {
    command.dispatch();
    previousMillis = currentMillis;
  } else {

    // calculate time since previous command, handle rollover
    unsigned long intervalSincePrevious =  (currentMillis < previousMillis) ? 
        (ULONG_MAX - previousMillis) + currentMillis + 1 : // rollover
        (currentMillis - previousMillis); // no rollover

    if (intervalSincePrevious >= interval) {
      disconnected();
      previousMillis = currentMillis;
    }
  }
}