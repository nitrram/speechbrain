
import torch
from speechbrain.pretrained.interfaces import Pretrained


class EncoderDecoderTransducerASR(Pretrained):
    HPARAMS_NEEDED = ["tokenizer"]
    MODULES_NEEDED = [
        "encoder",
        "decoder",
    ]
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)
        self.tokenizer = self.hparams.tokenizer

    def transcribe_file(self, path):
        waveform = self.load_audio(path)
        # Fake a batch:
        batch = waveform.unsqueeze(0)
        rel_length = torch.tensor([1.0])
        predicted_words, predicted_tokens = self.transcribe_batch(
            batch, rel_length
        )

        return predicted_words[0]

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
