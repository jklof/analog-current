
#include <UTFTGLUE.h>              //use GLUE class and constructor
UTFTGLUE myGLCD(0,A2,A1,A3,A4,A0); //all dummy args

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


struct cmd {
    int read;
    uint8_t size;
    uint8_t data[256];

public:
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

      sync = 0xfe,
      console = 0xff

    };

    cmd() {
      size = 0;
      read = -1;
    }
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
} cmd_buf;



enum State {
  s_init = 0,
  s_wait,
  s_update,
  s_read,
};


enum State fn_init() {
// Clear the screen and draw the frame
  myGLCD.clrScr();
  myGLCD.setBackColor(255, 64, 64);
  myGLCD.setColor(255,255,255);

  myGLCD.setColor(0, 255, 0);
  myGLCD.drawRect(0, 0, 479, 319);

  myGLCD.setColor(255,255,255);
  myGLCD.print("* MONITOR *", CENTER, 1);
  return s_wait;
}

enum State fn_wait()
{
  myGLCD.print("* WAIT *", CENTER, 20);
  if (Serial) {
    myGLCD.print("* READY *", CENTER, 36);
    return s_read;
  }
  return s_wait;
}

State fn_update() {

  uint8_t *p = cmd_buf.data;

  if (cmd_buf.size >= 1) {
    uint8_t cmd = *p++;

    switch (cmd) {
      case cmd::sync:
        break;

      case cmd::clear:
        if (cmd_buf.size == 1) {
          myGLCD.clrScr();
        }
        break;

      case cmd::color:
        if (cmd_buf.size ==  1 + 3) {
          myGLCD.setColor(p[0],p[1],p[2]);
        }
        break;

      case cmd::bgcolor:
        if (cmd_buf.size == 1 + 3) {
          myGLCD.setBackColor(p[0],p[1],p[2]);
        }
        break;

      case cmd::box:
        if (cmd_buf.size == 1 + 4*2) {
          uint16_t x0 = (uint16_t(p[0])<<8) | uint16_t(p[1]);
          uint16_t y0 = (uint16_t(p[2])<<8) | uint16_t(p[3]);
          uint16_t x1 = (uint16_t(p[4])<<8) | uint16_t(p[5]);
          uint16_t y1 = (uint16_t(p[6])<<8) | uint16_t(p[7]);
          myGLCD.fillRect(x0,y0,x1,y1);
        }
        break;        

      case cmd::line:
        if (cmd_buf.size == 1 + 4*2) {
          uint16_t x0 = (uint16_t(p[0])<<8) | uint16_t(p[1]);
          uint16_t y0 = (uint16_t(p[2])<<8) | uint16_t(p[3]);
          uint16_t x1 = (uint16_t(p[4])<<8) | uint16_t(p[5]);
          uint16_t y1 = (uint16_t(p[6])<<8) | uint16_t(p[7]);
          myGLCD.drawLine(x0,y0,x1,y1);
        }
        break;

      case cmd::print:
        if (cmd_buf.size > 1 + 2*2) {
          uint16_t x0 = (uint16_t(p[0])<<8) | uint16_t(p[1]);
          uint16_t y0 = (uint16_t(p[2])<<8) | uint16_t(p[3]);
          char* text = (char*)(p+4);
          cmd_buf.data[cmd_buf.size-1] = 0; // enforce null
          myGLCD.print(text,x0,y0);
        }
        break;

      case cmd::font:
        if (cmd_buf.size ==  1+1) {
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

      case cmd::rect:
        if (cmd_buf.size == 1 + 4*2) {
          uint16_t x0 = (uint16_t(p[0])<<8) | uint16_t(p[1]);
          uint16_t y0 = (uint16_t(p[2])<<8) | uint16_t(p[3]);
          uint16_t x1 = (uint16_t(p[4])<<8) | uint16_t(p[5]);
          uint16_t y1 = (uint16_t(p[6])<<8) | uint16_t(p[7]);
          myGLCD.drawRect(x0,y0,x1,y1);
        }
        break;

      case cmd::circle:
        if (cmd_buf.size == 1 + 3*2) {
          uint16_t x = (uint16_t(p[0])<<8) | uint16_t(p[1]);
          uint16_t y = (uint16_t(p[2])<<8) | uint16_t(p[3]);
          uint16_t r = (uint16_t(p[4])<<8) | uint16_t(p[5]);
          myGLCD.drawCircle(x,y,r);
        }
        break;

      case cmd::fillcircle:
        if (cmd_buf.size == 1 + 3*2) {
          uint16_t x = (uint16_t(p[0])<<8) | uint16_t(p[1]);
          uint16_t y = (uint16_t(p[2])<<8) | uint16_t(p[3]);
          uint16_t r = (uint16_t(p[4])<<8) | uint16_t(p[5]);
          myGLCD.fillCircle(x,y,r);
        }
        break;

      case cmd::console:
        if (cmd_buf.size ==  1+1) {
          console.debug(p[0]);
        }
        break;

      default:
        break;

    } // switch
    Serial.write(cmd);
    //Serial.flush();
  } // if
  return s_read;
}

State fn_read() {
  int byte;
  while ( (byte = Serial.read()) >= 0) {
    console.hex(byte);
    if ( cmd_buf.add(byte) ) {
      return s_update;
    }
  }
  return s_read;
}

// initial state
State state = s_init;
// table for functions to call in each state
State (*const fn_table[])() = {
  &fn_init,
  &fn_wait,
  &fn_update,
  &fn_read
};


void setup() {
  // put your setup code here, to run once:
  myGLCD.InitLCD();
  myGLCD.setFont(SmallFont);
  // set up serial
  Serial.begin(115200);

}

void loop() {
  // put your main code here, to run repeatedly:
  state = fn_table[state]();
}
