from speechbrain.pretrained import EncoderDecoderASR

asr_model = EncoderDecoderASR.from_hparams(source="asr-rnnt-commonvoice-cs", savedir="pretrained_model/asr-rnnt-commonvoice-cs")

audio_file = "asr-rnnt-commonvoice-cs/example-cs.wav"

print(asr_model.transcribe_file(audio_file))
