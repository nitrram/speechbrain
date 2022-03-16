#!/usr/bin/env python3

import sys, getopt, os
import pyxhook
import torch
import time

import _thread

from torch import Tensor

from sbinterface import EncoderDecoderTransducerASR
from ctypes import *


class Looper():

    __interrupt = False
    __hook = pyxhook.HookManager()
    __shared_lib = cdll.LoadLibrary('libcbindtest.so')

    def __init__(self):
        self.__hook.KeyDown = self.__on_key_pressed
        torch.ops.load_library('libcbindtest.so')
        self.__shared_lib.init()

    def __on_key_pressed(self, event):
        print("on key pressed")
        self.__hook.cancel()
        self.__interrupt = True
        return self.__interrupt

#    def __poll_tensor_impl(self) -> torch.Tensor:
#        return self.__shared_lib.poll_tensor()

    def poll_tensor(self):
        while not self.__interrupt:
#            tensor = self.__shared_lib.poll_tensor()
            tensor = torch.ops.cbindtest.poll_tensor()
            print("[{} {}]".format(type(tensor), tensor))
            time.sleep(0.075)

        self.__shared_lib.release()

    # def poll_frames(self):
    #     start_time = time.time() * 1000
    #     while not self.__interrupt:
    #         poll_func = self.__shared_lib.poll_frames
    #         poll_func.argtypes = None
    #         poll_func.restype = POINTER(c_uint8)
    #         res = poll_func()

    def start(self):
        self.__interrupt = False
        self.__hook.HookKeyboard()
        self.__hook.start()
        self.poll_tensor()



def main():

    #source = "exp/crdnn_gru.cs.cv7.outneu1000.char/1234"
    #source = "exp/crdnn_gelu.cs.cv8.outneu80.bpe"
    source = "."
    hparams_file = "eval.yaml"
    savedir = "pretrained_model"

    opts, args = getopt.getopt(sys.argv[1:], "he:p:", ["eval=", "pcm="])

    file_to_decode = ""

    for opt , arg in opts:
        if opt == "-h":
            print("./decode.py -e <eval.yaml> -p <pcm_file_to_decode>")
            sys.exit()
        elif opt in ("-e", "--eval"):
            hparams_file = os.path.basename(arg)
            source = os.path.dirname(arg) ## directory of file
        elif opt in ("-p", "--pcm"):
            file_to_decode = arg


    if ((not (hparams_file and hparams_file.strip())) and os.path.exists(hparams_file)):
        print("hyperparameter setup file is needed")
        sys.exit()

    if ((not (file_to_decode and file_to_decode.strip())) and os.path.exists(file_to_decode)):
        print("there's nothing to decode")
        sys.exit()

    print("setup: \n" +
          "source {}\n ".format(source) +
          "eval {}\n".format(hparams_file) +
          "=============================")


    print("Running...")

    looper = Looper()
    looper.start()

    print("\033[0;32mDone\033[0;0m")


if __name__ == "__main__":
    main()
