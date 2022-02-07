from speechbrain.pretrained import EncoderDecoderASR

asr_model = EncoderDecoderASR.from_hparams(source="asr-crdnn-commonvoice-fr", savedir="pretrained_model/asr-crdnn-commonvoice-fr")

audio_file = "asr-crdnn-commonvoice-fr/example-fr.wav"

print(asr_model.transcribe_file(audio_file))
