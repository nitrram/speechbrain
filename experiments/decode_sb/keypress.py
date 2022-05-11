#!/usr/bin/env python3

import sys, getopt, os
import pyxhook
import torch
import time

import _thread

from torch import Tensor

import torchaudio

from speechbrain.dataio.preprocess import AudioNormalizer
from sbinterface import EncoderDecoderTransducerASR
from ctypes import *


class Looper():

    _hyp = ""
    _tensor_rec = None
    _sr = 0

    def __init__(self, asr):
        self._audioNormalizer = AudioNormalizer()
        self._hook = pyxhook.HookManager()

        self._hook.KeyDown = self.__on_key_pressed
        self._interrupt = False
        self._asr = asr

        torch.ops.load_library('libcbindtest.so')
        torch.ops.sprbind.init()

    def __on_key_pressed(self, event):
        print("\nPYTHON on key pressed")
        self._hook.cancel()
        self._interrupt = True
        return self._interrupt

    def poll_tensor(self):
        while not self._interrupt:
            tensor, sr = torch.ops.sprbind.poll_tensor()

            self._sr = sr

            norm = self._audioNormalizer(tensor, sr)

            #TODO: make asr translate that tensor (perhaps normalize ahead of time)
            if(norm.size(dim=0) > 0):

                if(self._tensor_rec == None):
                    self._tensor_rec = norm
                else:
                    self._tensor_rec = torch.cat([self._tensor_rec, norm])

                trans = self._asr.transcribe_tensor(norm)
                self._hyp += ". " if not trans else trans
                print(" hypothesis: \" {} \" [{}]".format( self._hyp, norm.size()), end='\r')

            time.sleep(0.25) # recognize 4x in a second

        log_file_name = "log.wav"

        saving = torch.unsqueeze(torch.flatten(self._tensor_rec), 0)
        print(" saving audio: {} - {} [size]".format(log_file_name, saving.size(dim=1)))
        torchaudio.save(log_file_name, saving, self._sr,
                        encoding="PCM_S", bits_per_sample=16)

        torch.ops.sprbind.release()

    # def poll_frames(self):
    #     start_time = time.time() * 1000
    #     while not self.__interrupt:
    #         poll_func = self.__shared_lib.poll_frames
    #         poll_func.argtypes = None
    #         poll_func.restype = POINTER(c_uint8)
    #         res = poll_func()

    def start(self):
        self._interrupt = False
        self._hook.HookKeyboard()
        self._hook.start()
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


    print("PYTHON ASR initialize")
    asr = EncoderDecoderTransducerASR.from_hparams(source = source, hparams_file=hparams_file, savedir=savedir)


    print("PYTHON Running...")

    looper = Looper(asr)
    looper.start()

    print("PYTHON \033[0;32mDone\033[0;0m")


if __name__ == "__main__":
    main()
