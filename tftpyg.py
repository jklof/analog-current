
import pygame
import time
import struct
import pygame
import objloader
import os
import threading
import signal

from enum import Enum, auto


class DisplayController:

    # for printf x
    LEFT = 0
    RIGHT = 9999
    RIGHT = 9999
    CENTER = 9998

    def __init__(self, port, baudrate=115200, timeout=1):
        pygame.init()
        self.width = 480
        self.height = 320
        self.screen = pygame.display.set_mode((self.width, self.height))
        self.fgcolor = (0xff,0xff,0xff)
        self.bgcolor = (0,0,0)
        self.vfont = objloader.OBJLoader("./vfont.obj")

    def __enter__(self):
        return self

    def __exit__(self, exc_type, exc_value, traceback):
        pygame.quit()

    def sync(self):
        pygame.display.flip()

    def clear(self):
        self.screen.fill((0,0,0))

    def color(self, r, g, b):
        self.fgcolor = (r,g,b)

    def bgcolor(self, r, g, b):
        self.bgcolor = (r,g,b)

    def box(self, x0, y0, x1, y1):
        pygame.draw.rect(self.screen, self.fgcolor, (x0, y0, x1 - x0, y1 - y0), 0)

    def line(self, x0, y0, x1, y1):
        pygame.draw.line(self.screen , self.fgcolor, (x0, y0), (x1, y1))

    def rect(self, x0, y0, x1, y1):
        pass

    def circle(self, x, y, r):
        pass

    def fillcircle(self, x, y, r):
        pass
    def print(self, text, x, y):
        pass

    def vtext(self, text, x, y, scale):
        self.vfont.print(self, text, x,y,scale)

    def font(self, f):
        pass

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

        display.sync()
        print("hello")
        time.sleep(500)
