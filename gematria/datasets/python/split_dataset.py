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

import os

from absl import app
from absl import flags
from absl import logging

import tensorflow

_INPUT_TFRECORD_FILE = flags.DEFINE_string(
    'input_tfrecord',
    None,
    'The path to the input tfrecord file to process',
    required=True)

_OUTPUT_DIR = flags.DEFINE_string(
    'output_dir',
    None,
    'The path to the output directory',
    required=True)

_NUM_SHARDS = flags.DEFINE_integer(
    'shards',
    10,
    'The number of shards to create')

def main(_):
  raw_dataset = tensorflow.data.TFRecordDataset(_INPUT_TFRECORD_FILE.value)

  for i in range(_NUM_SHARDS.value):
    output_path = os.path.join(_OUTPUT_DIR.value, f'dataset-{i}.tfrecord')
    writer = tensorflow.data.experimental.TFRecordWriter(output_path)
    writer.write(raw_dataset.shard(_NUM_SHARDS.value, i))
    logging.info(f'Finished writing shard to {output_path}')

if __name__ == '__main__':
  app.run(main)
