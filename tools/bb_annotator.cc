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

  const LLVMState State = ExitOnErr(LLVMState::Create("x86_64-unknown-unknown", "x86-64", "", true));

  const std::unique_ptr<BenchmarkRunner> Runner =
      ExitOnErr(State.getExegesisTarget().createBenchmarkRunner(
          Benchmark::Latency, State, BenchmarkPhaseSelectorE::Measure, BenchmarkRunner::ExecutionModeE::SubProcess,
          Benchmark::Min));
  if (!Runner) {
    ExitWithError("cannot create benchmark runner");
  }

  std::vector<BenchmarkCode> Configurations = ExitOnErr(readSnippets(State, "/gematria/test.asm"));

  std::unique_ptr<const SnippetRepetitor> Repetitor = SnippetRepetitor::Create(Benchmark::RepetitionModeE::Duplicate, State);

  auto RC = ExitOnErr(Runner->getRunnableConfiguration(Configurations[0], 10000, 0, *Repetitor));

  auto BenchmarkResults = ExitOnErr(Runner->runConfiguration(std::move(RC), {}));
  std::cout << BenchmarkResults.Measurements[0].PerSnippetValue << "\n";

  std::cout << "testing\n";
  return 0;
}