#include <iostream>
#include <memory>

#include "BenchmarkRunner.h"
#include "LlvmState.h"
#include "SnippetFile.h"
#include "Target.h"
#include "TargetSelect.h"
#include "gematria/llvm/disassembler.h"
#include "gematria/llvm/llvm_architecture_support.h"
#include "gematria/utils/string.h"
#include "llvm/include/llvm/Support/Error.h"
#include "llvm/include/llvm/Support/TargetSelect.h"

using namespace gematria;
using namespace llvm;
using namespace llvm::exegesis;

static ExitOnError ExitOnErr("bb_annotator error: ");

// Helper function that logs the error(s) and exits.
template <typename... ArgTs>
static void ExitWithError(ArgTs &&...Args) {
  ExitOnErr(make_error<Failure>(std::forward<ArgTs>(Args)...));
}

int main() {
  InitializeAllTargetInfos();
  InitializeAllTargets();
  InitializeAllTargetMCs();

  InitializeAllAsmPrinters();
  InitializeAllAsmParsers();
  InitializeAllExegesisTargets();

  // Last value (which is a boolean) specifies whether or not we should use
  // dummy perf counters.
  const LLVMState State = ExitOnErr(LLVMState::Create("", "native", "", false));

  std::string HexBB = "4883c2014883fa40";

  std::unique_ptr<LlvmArchitectureSupport> ArchSupport =
      LlvmArchitectureSupport::X86_64();

  std::unique_ptr<llvm::MCInstPrinter> MCPrinter =
      ArchSupport->CreateMCInstPrinter(0);

  auto bytes_or = ParseHexString(HexBB);
  if (!bytes_or.has_value()) {
    ExitWithError("Failed to parse hex string");
  }

  auto Instructions = DisassembleAllInstructions(
      ArchSupport->mc_disassembler(), ArchSupport->mc_instr_info(),
      ArchSupport->mc_register_info(), ArchSupport->mc_subtarget_info(),
      *MCPrinter, 0, bytes_or.value());

  if (!Instructions) {
    ExitOnErr(Instructions.takeError());
  }

  std::vector<MCInst> MachineInstructions;

  for (auto Instruction : *Instructions) {
    MachineInstructions.push_back(Instruction.mc_inst);
  }

  const std::unique_ptr<BenchmarkRunner> Runner =
      ExitOnErr(State.getExegesisTarget().createBenchmarkRunner(
          Benchmark::Latency, State, BenchmarkPhaseSelectorE::Measure,
          BenchmarkRunner::ExecutionModeE::SubProcess, Benchmark::Min));
  if (!Runner) {
    ExitWithError("cannot create benchmark runner");
  }

  if (exegesis::pfm::pfmInitialize()) ExitWithError("cannot initialize libpfm");

  BenchmarkCode BenchCode;
  BenchCode.Key.Instructions = MachineInstructions;

  MemoryValue MemVal;
  MemVal.Value = APInt(4096, 305419776);
  MemVal.Index = 0;
  MemVal.SizeBytes = 4096;

  BenchCode.Key.MemoryValues["test"] = MemVal;

  std::unique_ptr<const SnippetRepetitor> Repetitor =
      SnippetRepetitor::Create(Benchmark::RepetitionModeE::Duplicate, State);

  while (true) {
    auto RC = ExitOnErr(
        Runner->getRunnableConfiguration(BenchCode, 10000, 0, *Repetitor));

    auto BenchmarkResults = Runner->runConfiguration(std::move(RC), {});

    if (!BenchmarkResults) {
      auto ResultError = BenchmarkResults.takeError();
      if (ResultError.isA<SnippetCrash>()) {
        Error blahTest = handleErrors(
            std::move(ResultError), [&](SnippetCrash &testThing) -> Error {
              if (testThing.SegfaultAddress == 0) {
                return Error::success();
              }
              std::cout << testThing.SegfaultAddress << "\n";
              testThing.log(outs());
              std::cout << "\n";
              MemoryMapping MemMap;
              MemMap.Address = testThing.SegfaultAddress;
              MemMap.MemoryValueName = "test";
              BenchCode.Key.MemoryMappings.push_back(MemMap);

              return Error::success();
            });
        if (blahTest) {
          std::cout << "Threw an actual error\n";
        }
      } else {
        std::cout << "was not a snippet crash\n";
      }
      continue;
    } else {
      std::cout << "No errors?\n";
    }
    // std::cout << BenchmarkResults->Measurements[0].PerSnippetValue << "\n";
    break;
    // std::cout << BenchmarkResults->Error << "\n";
  }

  std::cout << "mapping memory\n";
  for (MemoryMapping &Mapping : BenchCode.Key.MemoryMappings) {
    std::cout << "Mapping at: " << Mapping.Address << "\n";
  }

  return 0;
}
