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

#include <fstream>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <string_view>

#include "absl/flags/flag.h"
#include "absl/flags/parse.h"
#include "gematria/datasets/bhive_importer.h"
#include "gematria/datasets/find_accessed_addrs.h"
#include "gematria/datasets/find_accessed_addrs_exegesis.h"
#include "gematria/llvm/canonicalizer.h"
#include "gematria/llvm/llvm_architecture_support.h"
#include "gematria/utils/string.h"
#include "X86RegisterInfo.h"
#include "X86Subtarget.h"

#include "llvm/Support/JSON.h"
#include "llvm/Support/raw_ostream.h"

#include "TargetSelect.h"

constexpr uint64_t kInitialRegVal = 0x12345600;
constexpr uint64_t kInitialMemVal = 0x12345600;
constexpr std::string_view kRegDefPrefix = "# LLVM-EXEGESIS-DEFREG ";
constexpr std::string_view kMemDefPrefix = "# LLVM-EXEGESIS-MEM-DEF ";
constexpr std::string_view kMemMapPrefix = "# LLVM-EXEGESIS-MEM-MAP ";
constexpr std::string_view kMemNamePrefix = "MEM";

namespace {
unsigned int file_counter = 0;
}

ABSL_FLAG(std::string, bhive_csv, "", "Filename of the input BHive CSV file");
ABSL_FLAG(
    std::string, output_dir, "",
    "Directory containing output files that can be executed by llvm-exegesis");

int main(int argc, char* argv[]) {
  absl::ParseCommandLine(argc, argv);

  const std::string bhive_filename = absl::GetFlag(FLAGS_bhive_csv);
  if (bhive_filename.empty()) {
    std::cerr << "Error: --bhive_csv is required\n";
    return 1;
  }

  const std::string output_dir = absl::GetFlag(FLAGS_output_dir);
  if (output_dir.empty()) {
    std::cerr << "Error: --output_dir is required\n";
    return 1;
  }

  std::string register_defs_lines;
  const std::unique_ptr<gematria::LlvmArchitectureSupport> llvm_support =
      gematria::LlvmArchitectureSupport::X86_64();
  const llvm::MCRegisterInfo& MRI = llvm_support->mc_register_info();

  // Iterate through all general purpose registers and vector registers
  // and add them to the register definitions.
  for (unsigned i = 0;
       i < MRI.getRegClass(llvm::X86::GR64_NOREX2RegClassID).getNumRegs(); ++i) {
    llvm::StringRef reg_name =
        MRI.getName(MRI.getRegClass(llvm::X86::GR64_NOREX2RegClassID).getRegister(i));
    std::stringstream register_line_stream;
    register_line_stream << kRegDefPrefix << reg_name.str() << " " << std::hex << kInitialRegVal << "\n";
    register_defs_lines += register_line_stream.str();
  }
  for (unsigned i = 0; i < MRI.getRegClass(llvm::X86::VR128RegClassID).getNumRegs();
       ++i) {
    llvm::StringRef reg_name =
        MRI.getName(MRI.getRegClass(llvm::X86::VR128RegClassID).getRegister(i));
    std::stringstream register_line_stream;
    register_line_stream << kRegDefPrefix << reg_name.str() << " " << std::hex << kInitialRegVal << "\n";
    register_defs_lines += register_line_stream.str();
  }

  gematria::X86Canonicalizer canonicalizer(&llvm_support->target_machine());
  gematria::BHiveImporter bhive_importer(&canonicalizer);

  llvm::exegesis::InitializeX86ExegesisTarget();

  llvm::exegesis::LLVMState State(cantFail(llvm::exegesis::LLVMState::Create("", "native")));

  auto Annotator = cantFail(gematria::ExegesisAnnotator::Create(*llvm_support, State));

  std::string output_file_path = output_dir + "/benchmarks.json";

  std::ifstream bhive_csv_file(bhive_filename);
  llvm::json::Array ProcessedSnippets;
  for (std::string line; std::getline(bhive_csv_file, line);) {
    std::cout << "Working on file " << std::dec << file_counter << "\n";
    auto comma_index = line.find(',');
    if (comma_index == std::string::npos) {
      std::cerr << "Invalid CSV file: no comma in line '" << line << "'\n";
      return 2;
    }

    std::string_view hex = std::string_view(line).substr(0, comma_index);
    // For each line, find the accessed addresses & disassemble instructions.
    auto bytes = gematria::ParseHexString(hex);
    if (!bytes.has_value()) {
      std::cerr << "could not parse: " << hex << "\n";
      return 3;
    }

    // This will only get the first segfault address.
    auto addrs = Annotator->FindAccessedAddrs(*bytes);
    auto proto = bhive_importer.BasicBlockProtoFromMachineCode(*bytes);

    // Check for errors.
    if (!proto.ok()) {
      std::cerr << "Failed to disassemble block '" << hex << ": "
                << proto.status() << "\n";
      continue;
    }
    if (!addrs) {
      std::cerr << "Failed to find addresses for block\n";
      continue;
    }

    llvm::json::Object CurrentSnippet;
    if (addrs->accessed_blocks.size() > 0) {
      llvm::json::Array MemoryDefinitions;
      llvm::json::Object CurrentMemoryDefinition;
      CurrentMemoryDefinition["Name"] = llvm::json::Value(kMemNamePrefix);
      CurrentMemoryDefinition["Size"] = llvm::json::Value(addrs->block_size);
      CurrentMemoryDefinition["Value"] = llvm::json::Value("000000000012345600");
      MemoryDefinitions.push_back(llvm::json::Value(std::move(CurrentMemoryDefinition)));
      CurrentSnippet["MemoryDefinitions"] = llvm::json::Value(std::move(MemoryDefinitions));

      llvm::json::Array MemoryMappings;
      for (const uintptr_t addr : addrs->accessed_blocks) {
        llvm::json::Object CurrentMemoryMapping;
        CurrentMemoryMapping["Value"] = llvm::json::Value(kMemNamePrefix);
        CurrentMemoryMapping["Address"] = llvm::json::Value(addr);
        MemoryMappings.push_back(std::move(CurrentMemoryMapping));
      }
      CurrentSnippet["MemoryMappings"] = llvm::json::Value(std::move(MemoryMappings));
    } else {
      CurrentSnippet["MemoryDefinitions"] = llvm::json::Array();
      CurrentSnippet["MemoryMappings"] = llvm::json::Array();
    }
    std::string HexString = {hex.begin(), hex.end()};
    CurrentSnippet["Hex"] = llvm::json::Value(HexString);

    ProcessedSnippets.push_back(llvm::json::Value(std::move(CurrentSnippet)));

    std::cout << "Just finished writing all annotations for a snippet\n";

    file_counter++;
  }
  std::error_code FileEC;
  llvm::raw_fd_ostream OutputFile("/tmp/test.json", FileEC);
  OutputFile << llvm::formatv("{0:2}", llvm::json::Value(std::move(ProcessedSnippets))).str();
}
