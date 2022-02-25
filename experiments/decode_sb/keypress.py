import _thread
import time
import os
import pyxhook

from ctypes import *



class Looper():

    __interrupt = False
    __hook = pyxhook.HookManager()
    __shared_lib = cdll.LoadLibrary('libcbindtest.so')

    def __init__(self):
        self.__hook.KeyDown = self.__on_key_pressed
        print("Looper initialized")
        

    def __on_key_pressed(self, event):
        print("on key pressed")
        self.__hook.cancel()
        self.__interrupt = True
        return self.__interrupt
    
    def poll_frames(self, delay):
        start_time = time.time() * 1000
        while not self.__interrupt:
#            time.sleep(delay)
            poll_func = self.__shared_lib.poll_frames
            poll_func.argtypes = None
            poll_func.restype = POINTER(c_uint8)

            res = poll_func()
#            print(bool(res))
            if bool(res):
                print("res; [%s,%s] in %s[ms]" % (res[0], res[1], ( round(time.time() * 1000 - start_time) )))
#            else:
#                print("res; \033[0;31mNULL\033[0;0m in %s[ms]" % ( round(time.time() * 1000 - start_time) ))

    def start(self):
        self.__interrupt = False
        self.__hook.HookKeyboard()
        self.__hook.start()
        self.poll_frames(0.1)
        


def main():
    print("Hello World!")

    shared_lib = cdll.LoadLibrary("libcbindtest.so")

    print(vars(shared_lib))

    shared_lib.init()
    
    looper = Looper()
    looper.start()

    shared_lib.release()
    

if __name__ == "__main__":
    main()


print("\033[0;32mDone!\033[0;0m")
