// Copyright 2024 Google Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//    http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "gematria/io/proto_parser.h"

#include <vector>

#include "gematria/basic_block/basic_block_protos.h"
#include "gematria/proto/basic_block.pb.h"
#include "gematria/proto/throughput.pb.h"

namespace gematria {

std::vector<BasicBlockWithThroughputProto> ParseRawData(const std::vector<std::string> &Input) {
  std::vector<BasicBlockWithThroughputProto> Output;
  Output.reserve(Input.size());
  for (const std::string &InputString : Input) {
    BasicBlockWithThroughputProto ToAdd;
    ToAdd.ParseFromString(InputString);
    Output.push_back(std::move(ToAdd));
  }
  return Output;
}

} // namespace gematria
