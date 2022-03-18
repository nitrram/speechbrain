
import torch
from speechbrain.pretrained.interfaces import Pretrained

import _thread
import time
import os
import pyxhook

# class Looper():

#      def __init__(self):
#          __interrupt = False
#          __hook = pyxhook.HookManager()
#          __hook.KeyDown = __on_key_pressed
        

#      def __on_key_pressed(event):
#          _interrupt = True
    
#      # Define a function for the thread
#      def poll_frames(self, delay):
#          while not __interrupt:
#              time.sleep(delay)
#              print("%s: %s" % ( threadName, time.ctime(time.time()) ))


#      def start(self):
#          __interrupt = False
#          try:
#              thread.start_new_thread( poll_frames, (self, 2, ) )
#          except:
#              print("Error: unable to start thread")
               
#          while 1:
#              pass



class EncoderDecoderTransducerASR(Pretrained):
    HPARAMS_NEEDED = ["tokenizer"]
    MODULES_NEEDED = [
        "encoder",
        "decoder",
    ]
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)
        self.tokenizer = self.hparams.tokenizer

    # def transcribe_mic(self):
    #     return

    def transcribe_tensor(self, waveform):
        batch = waveform.unsqueeze(0)
        rel_length = torch.tensor([1.0])
        predicted_words, predicted_tokens = self.transcribe_batch(
            batch, rel_length
        )

        return predicted_words[0]
        

    def transcribe_file(self, path):
        return self.transcribe_tensor(self.load_audio(path))
        

    def transcribe_batch(self, wavs, wav_lens):
        with torch.no_grad():
            wavs = wavs.float()
            wavs, wav_lens = wavs.to(self.device), wav_lens.to(self.device)

            encoder_out = self.mods.encoder(wavs, wav_lens)

            (
                best_hyps,
                best_scores,
                nbest_hyps,
                nbest_scores,
            ) = self.hparams.decoder(encoder_out)

            predicted_words = [
                self.tokenizer.decode_ids(token_seq)
                for token_seq in best_hyps
            ]

            return predicted_words, best_hyps

    def forward(self, wavs, wav_lens):
        """Runs full transcription - note: no gradients through decoding"""
        return self.transcribe_batch(wavs, wav_lens)
