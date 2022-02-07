from speechbrain.nnet.transducer.transducer_joint import Transducer_joint
from speechbrain.decoders.transducer import TransducerBeamSearcher
import speechbrain as sb
import torch


emb = sb.nnet.embedding.Embedding(
    num_embeddings=35,
    embedding_dim=3,
    consider_as_one_hot=True,
    blank_id=0
)
dec = sb.nnet.RNN.GRU(
    hidden_size=10, input_shape=(1, 40, 34), bidirectional=False
)
lin = sb.nnet.linear.Linear(input_shape=(1, 40, 10), n_neurons=35)
joint_network= sb.nnet.linear.Linear(input_shape=(1, 1, 40, 35), n_neurons=35)
tjoint = Transducer_joint(joint_network, joint="sum")
searcher = TransducerBeamSearcher(
    decode_network_lst=[emb, dec],
    tjoint=tjoint,
    classifier_network=[lin],
    blank_id=0,
    beam_size=1,
    nbest=1,
    lm_module=None,
    lm_weight=0.0,
)


# bind encoder with existing model
enc = torch.rand([1, 20, 10])



hyps, scores, bb, bbs = searcher(enc)

print("best batch: " + str(bb))
print("best batch score: " + str(bbs))
print("hyps: " + str(hyps))
print("scores: " + str(scores))

