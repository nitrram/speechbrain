#!/usr/bin/env python3

import sys, getopt, os
from sbinterface import EncoderDecoderTransducerASR

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
        
    
asr = EncoderDecoderTransducerASR.from_hparams(source=source, hparams_file=hparams_file, savedir=savedir)
print(asr.transcribe_file(file_to_decode))
