import serial
import time
import struct

from enum import Enum, auto

class cmd(Enum):
    clear = 1
    color = 2
    bgcolor = 3
    box = 4
    line = 5
    rect = 6
    circle = 7
    fillcircle = 8
    vtext = 9
    filltrianvle = 10

    sync = 0xfe

class DisplayController:

    # for printf x
    LEFT = 0
    RIGHT = 9999
    RIGHT = 9999
    CENTER = 9998

    # for font
    SMALLFONT = 0
    BIGFONT = 1
    SEVENSEGNUMFONT = 2



    def __init__(self, port, baudrate=115200, timeout=1):
        self.port = port
        self.baudrate = baudrate
        self.timeout = timeout
        self.s = None

    def __enter__(self):
        self.s = serial.Serial(self.port, self.baudrate, timeout=self.timeout)
        self.sync()
        return self

    def __exit__(self, exc_type, exc_value, traceback):
        if self.s is not None:
            self.s.close()
            self.s = None

    def _send(self, p):
        if len(p) > 255:
            raise ValueError("Payload is too large")
        
        payload = struct.pack('B', len(p)) + p
        #print(payload)
        self.s.write(payload)
        ret = self.s.read(1)
        #print(ret)

    def sync(self):
        sret = b''
        while sret != struct.pack('B', cmd.sync.value):
            self.s.write(struct.pack('BB', 1, cmd.sync.value))
            self.s.flush()
            sret = self.s.read(1)

    def clear(self):
        self._send(struct.pack('B', cmd.clear.value))

    def color(self, r, g, b):
        p = struct.pack('BBBB', cmd.color.value, r, g, b)
        self._send(p)

    def bgcolor(self, r, g, b):
        p = struct.pack('BBBB', cmd.bgcolor.value, r, g, b)
        self._send(p)

    def box(self, x0, y0, x1, y1):
        p = struct.pack('>Bhhhh', cmd.box.value, x0, y0, x1, y1)
        self._send(p)

    def line(self, x0, y0, x1, y1):
        p = struct.pack('>Bhhhh', cmd.line.value, x0, y0, x1, y1)
        self._send(p)

    def rect(self, x0, y0, x1, y1):
        p = struct.pack('>Bhhhh', cmd.rect.value, x0, y0, x1, y1)
        self._send(p)

    def circle(self, x, y, r):
        p = struct.pack('>Bhhh', cmd.circle.value, x, y, r)
        self._send(p)

    def fillcircle(self, x, y, r):
        p = struct.pack('>Bhhh', cmd.fillcircle.value, x, y, r)
        self._send(p)
 
    def print(self, text, x, y):
        maxlen = 200
        s = str(text)
        if len(s) > maxlen:
            s = s[:maxlen]
        p = struct.pack('>Bhh', cmd.print.value, x, y) + s.encode('utf-8') + b'\0'
        self._send(p)

    def vtext(self, text, x, y, scale):
        maxlen = 200
        s = str(text)
        if len(s) > maxlen:
            s = s[:maxlen]
        p = struct.pack('>Bhhh', cmd.vtext.value, x, y, scale) + s.encode('utf-8') + b'\0'
        self._send(p)

    def font(self, f):
        p = struct.pack('BB', cmd.font.value, f)
        self._send(p)


if __name__ == '__main__':
    with DisplayController('COM7') as display:
        display.sync()
        display.bgcolor(0x20,0x60,0x70)
        display.clear()
        display.color(0xff,0xff,0xff)

        display.vtext('.-+*BAUHAUS 93 FONT*+-.', 0, 30, 30)
        display.vtext('0123456789.+-:=/#*$', 0,100,60)
        display.vtext('ABCDEFGHIJKLMN', 0,160,60)
        display.vtext('OPQRSTUVWXYZ', 0,220,60)

        display.color(0xff, 0xff, 0xff)
        display.box(0, 300, 479, 319)
        #display.color(0x10,0x80,0xff)
        #display.fillcircle(240,160,100)
        display.sync()
        time.sleep(5)
