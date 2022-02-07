from speechbrain.dataio.dataio import read_audio
from IPython.display import Audio



audio_file = "asr-crdnn-commonvoice-fr/example-fr.wav"

signal = read_audio(audio_file ).squeeze()

Audio(signal, rate=16000)
