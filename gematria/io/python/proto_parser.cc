// Copyright 2023 Google Inc.
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

#define PYBIND11_PROTOBUF_ASSUME_FULL_ABI_COMPATIBILITY

#include "pybind11/pybind11.h"
#include "pybind11/stl.h"
#include "pybind11_protobuf/native_proto_caster.h"

namespace gematria {

namespace py = ::pybind11;

PYBIND11_MODULE(proto_parser, m) {
  pybind11_protobuf::ImportNativeProtoCasters();

  m.doc() = "A proto parser";

  m.def("parse_raw_data", &ParseRawData, py::arg("Input"), "documentation"); 
}

} // namespace gematria
