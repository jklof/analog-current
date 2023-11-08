#include <limits.h>
#include <MCUFRIEND_kbv.h>
MCUFRIEND_kbv tft;
uint16_t tft_color = 0xffff;
uint16_t tft_bgcolor = 0x0000;

#include "bauhaus_lines.h"
#include "bauhaus_tris.h"


int draw_tri_letter(char chr, int x, int y, int scale)
{
  if (chr == ' ')
    return (font_tri_geometry[0].advance * scale) / TRI_FONT_SCALE;

  // default to first charater
  const int16_t *tris = font_tri_geometry[0].tris;
  const int16_t *end  = tris + font_tri_geometry[0].size;
  uint16_t horiz_advance = font_tri_geometry[0].advance;

  for (unsigned i=0; i< sizeof(font_tri_geometry)/sizeof(font_tri_geometry[0]); i++) {
    if (font_tri_geometry[i].chr == chr) {
      tris = font_tri_geometry[i].tris;
      end = tris + font_tri_geometry[i].size;
      horiz_advance = font_tri_geometry[i].advance;
      break;
    }
  }
  while (tris < end) {
    uint16_t v1 = pgm_read_word(tris++)*2;
    uint16_t v2 = pgm_read_word(tris++)*2;
    uint16_t v3 = pgm_read_word(tris++)*2;
    const int ax = int32_t((int16_t(pgm_read_word(tri_font_vertices+v1+0))) * scale) / LINE_FONT_SCALE;
    const int ay = int32_t((int16_t(pgm_read_word(tri_font_vertices+v1+1))) * scale) / LINE_FONT_SCALE;
    const int bx = int32_t((int16_t(pgm_read_word(tri_font_vertices+v2+0))) * scale) / LINE_FONT_SCALE;
    const int by = int32_t((int16_t(pgm_read_word(tri_font_vertices+v2+1))) * scale) / LINE_FONT_SCALE;
    const int cx = int32_t((int16_t(pgm_read_word(tri_font_vertices+v3+0))) * scale) / LINE_FONT_SCALE;
    const int cy = int32_t((int16_t(pgm_read_word(tri_font_vertices+v3+1))) * scale) / LINE_FONT_SCALE;
    tft.fillTriangle(x+ax,y-ay, x+bx, y-by, x+cx, y-cy, tft_color);
  }
  return (int32_t(horiz_advance) * scale) / LINE_FONT_SCALE;
}

int draw_line_letter(char chr, int x, int y, int scale) {

  if (chr == ' ')
    return (font_line_geometry[0].advance * scale) / LINE_FONT_SCALE;

  // default to first charater
  const int16_t *lines = font_line_geometry[0].lines;
  const int16_t *end  = lines + font_line_geometry[0].size;
  uint16_t horiz_advance = font_line_geometry[0].advance;

  for (unsigned i=0; i< sizeof(font_line_geometry)/sizeof(font_line_geometry[0]); i++) {
    if (font_line_geometry[i].chr == chr) {
      lines = font_line_geometry[i].lines;
      end = lines + font_line_geometry[i].size;
      horiz_advance = font_line_geometry[i].advance;
      break;
    }
  }
  while (lines < end) {
    uint16_t v1 = pgm_read_word(lines++)*2;
    uint16_t v2 = pgm_read_word(lines++)*2;
    const int ax = (int32_t(int16_t(pgm_read_word(line_font_vertices+v1+0))) * scale) / LINE_FONT_SCALE;
    const int ay = (int32_t(int16_t(pgm_read_word(line_font_vertices+v1+1))) * scale) / LINE_FONT_SCALE;
    const int bx = (int32_t(int16_t(pgm_read_word(line_font_vertices+v2+0))) * scale) / LINE_FONT_SCALE;
    const int by = (int32_t(int16_t(pgm_read_word(line_font_vertices+v2+1))) * scale) / LINE_FONT_SCALE;
    tft.drawLine(x+ax,y-ay, x+bx, y-by, tft_color);
  }
  return (int32_t(horiz_advance) * scale) / LINE_FONT_SCALE;
}
int drawtext(const char *text, int x, int y, int scale)
{
  int hz_advance = 0;
  while (*text) {
    hz_advance += draw_line_letter(*text, x+hz_advance, y, scale);
    //hz_advance += draw_tri_letter(*text, x+hz_advance, y, scale);
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