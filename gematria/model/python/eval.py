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

_PREDICTED = flags.DEFINE_string('predicted',
        None,
        'The path to the TFRecord with the predictions',
        required=True)

def main(_):
  predicted_protos = tfrecord.read_protos(_PREDICTED.value,
          throughput_pb2.BasicBlockWithThroughputProto)
  number_processed = 0
  error_sum = 0

  for proto in predicted_protos:
    if number_processed % 10000 == 0:
      logging.info(f'Have processed {number_processed}')
    throughput_a = proto.inverse_throughputs[0].inverse_throughput_cycles[0]
    throughput_b = proto.inverse_throughputs[1].inverse_throughput_cycles[0]
    error = abs(throughput_b - throughput_a) / throughput_a
    error_sum += error
    number_processed += 1

  average_error = error_sum / number_processed
  logging.info(f'Average error: {average_error}')

if __name__ == '__main__':
  app.run(main)
