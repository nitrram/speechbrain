#!/usr/bin/env python3

import sys, getopt, os, datetime
import signal
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

        signal.signal(signal.SIGINT, self.__on_exit)
        signal.signal(signal.SIGTERM, self.__on_exit)
                          
        self._interrupt = False
        self._asr = asr

        torch.ops.load_library('libcpybinding.so')
        torch.ops.sprbind.init()

    def __on_exit(self, *args):
        print("\nPYTHON... exitting")
        self._interrupt = True

    def poll_tensor(self):
        while not self._interrupt:
            tensor, sr = torch.ops.sprbind.poll_tensor()
#            print("tensor size: {}".format(tensor.numel()))
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
#            time.sleep(0.25) # recognize 4x in a second

        start_time = datetime.datetime.now()
        log_file_name = "log.wav"
        saving = torch.unsqueeze(torch.flatten(self._tensor_rec), 0)
        print("PYTHON saving audio: {} - {} [size]".format(log_file_name, saving.size(dim=1)))
        torchaudio.save(log_file_name, saving, self._sr,
                        encoding="PCM_S", bits_per_sample=16)
        torch.ops.sprbind.release()
        print("PYTHON audio save took: {}ms".format((datetime.datetime.now() - start_time).total_seconds()*1000));


        print("PYTHON transcribing... [{} elms]".format(self._tensor_rec.numel()))
        start_time = datetime.datetime.now()
        norm = self._audioNormalizer(self._tensor_rec, self._sr)
        print("PYTHON audio normalizer took: {}ms".format((datetime.datetime.now() - start_time).total_seconds()*1000));
        start_time = datetime.datetime.now()
        if(norm.size(dim=0) > 0):
            trans = self._asr.transcribe_tensor(norm)
            print("PYTHON final hypothesis: \" \033[0;33m {} \033[0;0m \"".format(trans))
        print("PYTHON transcription took: {}ms".format((datetime.datetime.now() - start_time).total_seconds()*1000));

    def start(self):
        self._interrupt = False
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
