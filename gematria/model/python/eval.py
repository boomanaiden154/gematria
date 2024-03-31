# Copyright 2024 Google Inc.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#    http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
"""Compares values for two different datasets"""

from absl import app
from absl import flags
from absl import logging

from gematria.io.python import tfrecord
from gematria.proto import throughput_pb2
from gematria.basic_block.python import throughput_protos
from gematria.basic_block.python import basic_block

_GROUND_TRUTH = flags.DEFINE_string('ground_truth', None, 'The path to the ground truth TFRecord', required=True)
_PREDICTED = flags.DEFINE_string('predicted', None, 'The path to the TFRecord with the predictions', required=True)

def main(_):
  ground_truth_protos = tfrecord.read_protos(_GROUND_TRUTH.value, throughput_pb2.BasicBlockWithThroughputProto)
  predicted_protos = tfrecord.read_protos(_PREDICTED.value, throughput_pb2.BasicBlockWithThroughputProto)
  number_processed = 0
  error_sum = 0

  for proto_a, proto_b in zip(ground_truth_protos, predicted_protos):
    if number_processed % 1000 == 0:
      logging.info(f'Have processed {number_processed}')
    basic_block_proto_a = throughput_protos.block_with_throughput_from_proto(proto_a)
    basic_block_proto_b = throughput_protos.block_with_throughput_from_proto(proto_b)
    throughput_a = basic_block_proto_a.throughputs[0].inverse_throughput_cycles[0]
    throughput_b = basic_block_proto_b.throughputs[1].inverse_throughput_cycles[0]
    error = abs(throughput_b - throughput_a) / throughput_a
    error_sum += error
    number_processed += 1

  average_error = error_sum / number_processed
  logging.info(f'Average error: {average_error}')

if __name__ == '__main__':
  app.run(main)
