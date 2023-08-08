#include <iostream>

#include "llvm/include/llvm/Support/TargetSelect.h"
#include "llvm/include/llvm/Support/Error.h"
#include "LlvmState.h"
#include "BenchmarkRunner.h"
#include "TargetSelect.h"
#include "Target.h"
#include "BenchmarkRunner.h"

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

  const LLVMState State = ExitOnErr(LLVMState::Create("x86_64-unknown-unknown", "x86-64", "", true));

  const std::unique_ptr<BenchmarkRunner> Runner =
      ExitOnErr(State.getExegesisTarget().createBenchmarkRunner(
          exegesis::Benchmark::Latency, State, BenchmarkPhaseSelectorE::Measure, BenchmarkRunner::ExecutionModeE::SubProcess,
          Benchmark::Min));
  if (!Runner) {
    ExitWithError("cannot create benchmark runner");
  }

  std::cout << "testing\n";
  return 0;
}