#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Support/raw_ostream.h"

namespace {
struct InstrFunctionsPass : llvm::PassInfoMixin<InstrFunctionsPass> {
  llvm::PreservedAnalyses run(llvm::Function &F,
                              llvm::FunctionAnalysisManager &) {
    llvm::LLVMContext &Ctx = F.getContext();
    llvm::IRBuilder<> Builder(Ctx);
    llvm::Module *Mod = F.getParent();
    bool hasStartInstr = false;
    bool hasEndInstr = false;

    llvm::FunctionType *voidFuncType =
        llvm::FunctionType::get(llvm::Type::getVoidTy(Ctx), false);
    llvm::FunctionCallee startInstrFunc =
        (*Mod).getOrInsertFunction("instrument_start", voidFuncType);
    llvm::FunctionCallee endInstrFunc =
        (*Mod).getOrInsertFunction("instrument_end", voidFuncType);

    for (auto &BB : F) {
      for (auto &Instr : BB) {
        if (llvm::isa<llvm::CallInst>(&Instr)) {
          llvm::CallInst *callInstr = llvm::cast<llvm::CallInst>(&Instr);
          if (callInstr->getCalledFunction() == startInstrFunc.getCallee()) {
            hasStartInstr = true;
          } else if (callInstr->getCalledFunction() ==
                     endInstrFunc.getCallee()) {
            hasEndInstr = true;
          }
        }
      }
    }

    if (!hasStartInstr) {
      Builder.SetInsertPoint(&F.getEntryBlock().front());
      Builder.CreateCall(startInstrFunc);
    }
    if (!hasEndInstr) {
      for (llvm::BasicBlock &BB : F) {
        if (llvm::dyn_cast<llvm::ReturnInst>(BB.getTerminator())) {
          Builder.SetInsertPoint(BB.getTerminator());
          Builder.CreateCall(endInstrFunc);
        }
      }
    }

    return llvm::PreservedAnalyses::all();
  }

  static bool isRequired() { return true; }
};
} // namespace

extern "C" LLVM_ATTRIBUTE_WEAK ::llvm::PassPluginLibraryInfo
llvmGetPassPluginInfo() {
  return {LLVM_PLUGIN_API_VERSION, "instrumentation_functions", "0.1",
          [](llvm::PassBuilder &PB) {
            PB.registerPipelineParsingCallback(
                [](llvm::StringRef passName, llvm::FunctionPassManager &FPM,
                   llvm::ArrayRef<llvm::PassBuilder::PipelineElement>) -> bool {
                  if (passName == "instrumentation-functions") {
                    FPM.addPass(InstrFunctionsPass{});
                    return true;
                  }
                  return false;
                });
          }};
}
