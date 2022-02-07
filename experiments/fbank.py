import torch
from speechbrain.lobes.features import Fbank

signal = torch.rand([4, 16000])
print(signal.shape)

fbank_maker = Fbank()
fbanks = fbank_maker(signal)
print(fbanks.shape)
