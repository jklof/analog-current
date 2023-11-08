#include <limits.h>
#include <MCUFRIEND_kbv.h>
MCUFRIEND_kbv tft;
uint16_t tft_color = 0xffff;
uint16_t tft_bgcolor = 0x0000;

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
    if (font_map[i].chr == chr) {
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
    tft.drawLine(ax+x,ay+y, bx+x, by+y, tft_color);
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
    rect = 6,
    circle = 7,
    fillcircle = 8,
    vtext = 9,
    filltriangle = 10,

    sync = 0xfe,

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
          tft.fillScreen(tft_bgcolor);
        }
        break;

      case color:
        if (size ==  1 + 3) {
          tft_color =  ((p[0]&0xF8) << 8) | ((p[1]&0xFC) << 3) | (p[2]>>3);
        }
        break;

      case bgcolor:
        if (size == 1 + 3) {
          tft_bgcolor =  ((p[0]&0xF8) << 8) | ((p[1]&0xFC) << 3) | (p[2]>>3);
        }
        break;

      case box:
        if (size == 1 + 4*2) {
          uint16_t x0 = (uint16_t(p[0])<<8) | uint16_t(p[1]);
          uint16_t y0 = (uint16_t(p[2])<<8) | uint16_t(p[3]);
          uint16_t x1 = (uint16_t(p[4])<<8) | uint16_t(p[5]);
          uint16_t y1 = (uint16_t(p[6])<<8) | uint16_t(p[7]);
          int w = int(x1) - int(x0) + 1, h = int(y1) - int(y0) + 1;
          if (w < 0) { x0 = x1; w = -w; }
          if (h < 0) { y0 = y1; h = -h; }
          tft.fillRect(x0, y0, w, h, tft_color);
        }
        break;

      case line:
        if (size == 1 + 4*2) {
          uint16_t x0 = (uint16_t(p[0])<<8) | uint16_t(p[1]);
          uint16_t y0 = (uint16_t(p[2])<<8) | uint16_t(p[3]);
          uint16_t x1 = (uint16_t(p[4])<<8) | uint16_t(p[5]);
          uint16_t y1 = (uint16_t(p[6])<<8) | uint16_t(p[7]);
          tft.drawLine(x0,y0,x1,y1, tft_color);
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

      case rect:
        if (size == 1 + 4*2) {
          uint16_t x0 = (uint16_t(p[0])<<8) | uint16_t(p[1]);
          uint16_t y0 = (uint16_t(p[2])<<8) | uint16_t(p[3]);
          uint16_t x1 = (uint16_t(p[4])<<8) | uint16_t(p[5]);
          uint16_t y1 = (uint16_t(p[6])<<8) | uint16_t(p[7]);
          int w = int(x1) - int(x0) + 1, h = int(y1) - int(y0) + 1;
          if (w < 0) { x0 = x1; w = -w; }
          if (h < 0) { y0 = y1; h = -h; }
          tft.drawRect(x0, y0, w, h, tft_color);
        }
        break;

      case circle:
        if (size == 1 + 3*2) {
          uint16_t x = (uint16_t(p[0])<<8) | uint16_t(p[1]);
          uint16_t y = (uint16_t(p[2])<<8) | uint16_t(p[3]);
          uint16_t r = (uint16_t(p[4])<<8) | uint16_t(p[5]);
          tft.drawCircle(x,y,r, tft_color);
        }
        break;

      case fillcircle:
        if (size == 1 + 3*2) {
          uint16_t x = (uint16_t(p[0])<<8) | uint16_t(p[1]);
          uint16_t y = (uint16_t(p[2])<<8) | uint16_t(p[3]);
          uint16_t r = (uint16_t(p[4])<<8) | uint16_t(p[5]);
          tft.fillCircle(x,y,r, tft_color);
        }
        break;
      
      case filltriangle:
        if (size == 1 + 4*2) {
          uint16_t x0 = (uint16_t(p[0])<<8) | uint16_t(p[1]);
          uint16_t y0 = (uint16_t(p[2])<<8) | uint16_t(p[3]);
          uint16_t x1 = (uint16_t(p[4])<<8) | uint16_t(p[5]);
          uint16_t y1 = (uint16_t(p[6])<<8) | uint16_t(p[7]);
          uint16_t x2 = (uint16_t(p[8])<<8) | uint16_t(p[9]);
          uint16_t y2 = (uint16_t(p[10])<<8) | uint16_t(p[11]);
          tft.fillTriangle(x0,y0,x1,y1,x2,y2, tft_color);
        }      

      default:
        break;

    } // switch
    Serial.write(cmd);
    //Serial.flush();
  } // if
}



uint32_t previousMillis = 0l;
const uint32_t interval = 2*60*1000l;  //

uint32_t current_secs = 0;

void disconnected()
{
  // draw disconnected message
  tft.fillScreen(0x0000);
  tft_color = 0xffff;
  drawtext("DISCONNECTED..", 0, 100, 100);
}


void hex(char *s, uint16_t u) {
  const char h[] = "0123456789ABCDEF";
  s[0] = h[(u>>12)&0xf];
  s[1] = h[(u>>8)&0xf];
  s[2] = h[(u>>4)&0xf];
  s[3] = h[(u>>0)&0xf];
  s[4] = 0;
  return s;
}

void setup() {


  uint16_t ID = tft.readID();
  tft.begin(ID);
  tft.setRotation(1); //Landscape

  // set up serial
  Serial.begin(115200);

  // draw ready message
  tft.fillScreen(0x0000);
  tft_color = 0xffff;

  char s[7];
  s[0] = 'I'; s[1] = 'D';
  hex(s+2,ID);
  drawtext(s, 0, 50, 60);
  drawtext("READY..", 0, 100, 100);

  previousMillis = millis();
}

void loop() {
  uint32_t currentMillis = millis();

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