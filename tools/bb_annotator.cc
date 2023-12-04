#include <iostream>
#include <memory>

#include "llvm/include/llvm/Support/TargetSelect.h"
#include "llvm/include/llvm/Support/Error.h"
#include "LlvmState.h"
#include "BenchmarkRunner.h"
#include "TargetSelect.h"
#include "Target.h"
#include "BenchmarkRunner.h"
#include "SnippetFile.h"

using namespace llvm;
using namespace llvm::exegesis;

static ExitOnError ExitOnErr("bb_annotator error: ");

// Helper function that logs the error(s) and exits.
template <typename... ArgTs> static void ExitWithError(ArgTs &&... Args) {
  ExitOnErr(make_error<Failure>(std::forward<ArgTs>(Args)...));
}

int main() {
  InitializeAllTargetInfos();
  InitializeAllTargets();
  InitializeAllTargetMCs();

  InitializeAllAsmPrinters();
  InitializeAllAsmParsers();
  InitializeAllExegesisTargets();

  // Last value (which is a boolean) specifies whether or not we should use dummy perf counters.
  const LLVMState State = ExitOnErr(LLVMState::Create("x86_64-unknown-unknown", "znver2", "", false));

  const std::unique_ptr<BenchmarkRunner> Runner =
      ExitOnErr(State.getExegesisTarget().createBenchmarkRunner(
          Benchmark::Latency, State, BenchmarkPhaseSelectorE::Measure, BenchmarkRunner::ExecutionModeE::SubProcess,
          Benchmark::Min));
  if (!Runner) {
    ExitWithError("cannot create benchmark runner");
  }

  if (exegesis::pfm::pfmInitialize())
    ExitWithError("cannot initialize libpfm");

  std::vector<BenchmarkCode> Configurations = ExitOnErr(readSnippets(State, "/tmp/gematria/test.asm"));

  MemoryValue MemVal;
  MemVal.Value = APInt(64, 0x0000000012345600);
  MemVal.Index = 0;
  MemVal.SizeBytes = 4096;

  Configurations[0].Key.MemoryValues["test"] = MemVal;

  std::unique_ptr<const SnippetRepetitor> Repetitor = SnippetRepetitor::Create(Benchmark::RepetitionModeE::Duplicate, State);

  while(true) {
    auto RC = ExitOnErr(Runner->getRunnableConfiguration(Configurations[0], 10000, 0, *Repetitor));

    auto BenchmarkResults = Runner->runConfiguration(std::move(RC), {});

    if(!BenchmarkResults) {
      auto ResultError = BenchmarkResults.takeError();
      if(ResultError.isA<SnippetCrash>()) {
        Error blahTest = handleErrors(std::move(ResultError), [&](SnippetCrash &testThing) -> Error {
	  if (testThing.SegfaultAddress == 0) {
	    std::cout << "got zero mapping\n";
	    return Error::success();
	  }
          std::cout << testThing.SegfaultAddress << "\n";
          testThing.log(outs());
          std::cout << "\n";
          MemoryMapping MemMap;
          MemMap.Address = (testThing.SegfaultAddress / 4096) * 4096;
          MemMap.MemoryValueName = "test";
          Configurations[0].Key.MemoryMappings.push_back(MemMap);

          return Error::success();
        });
        if(blahTest) {
          std::cout << "Threw an actual error\n";
        }
      } else {
        std::cout << "was not a snippet crash\n";
      }
      continue;
    } else {
      std::cout << "No errors?\n";
    }
    //std::cout << BenchmarkResults->Measurements[0].PerSnippetValue << "\n";
    break;
    //std::cout << BenchmarkResults->Error << "\n";
  }

  std::cout << "mapping memory\n";
  for(MemoryMapping &Mapping : Configurations[0].Key.MemoryMappings) {
    std::cout << "Mapping at: " << Mapping.Address << "\n";
  }

  return 0;
}
