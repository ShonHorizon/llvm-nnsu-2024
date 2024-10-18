#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Support/raw_ostream.h"

namespace {
struct FunctionInstrumentation : llvm::PassInfoMixin<FunctionInstrumentation> {
  llvm::PreservedAnalyses run(llvm::Function &func,
                              llvm::FunctionAnalysisManager &) {
    llvm::LLVMContext &ctx = func.getContext();
    llvm::IRBuilder<> builder(ctx);
    llvm::Module *mod = func.getParent();
    bool hasStart = false;
    bool hasEnd = false;

    llvm::FunctionType *funcType =
        llvm::FunctionType::get(llvm::Type::getVoidTy(ctx), false);
    llvm::FunctionCallee startInstr =
        mod->getOrInsertFunction("start_instrument", funcType);
    llvm::FunctionCallee endInstr =
        mod->getOrInsertFunction("end_instrument", funcType);

    llvm::Value *val = startInstr.getCallee()->stripPointerCasts();
    llvm::Value *val1 = endInstr.getCallee()->stripPointerCasts();
    if (llvm::isa<llvm::Function>(val) && llvm::isa<llvm::Function>(val1)) {
      llvm::Function *startFunc = llvm::cast<llvm::Function>(val);
      llvm::Function *endFunc = llvm::cast<llvm::Function>(val1);

      for (auto *user : startFunc->users()) {
        if (auto *callInst = llvm::dyn_cast<llvm::CallInst>(user)) {
          if (callInst->getFunction() == &func) {
            hasStart = true;
            break;
          }
        }
      }

      for (auto *user : endFunc->users()) {
        if (auto *callInst = llvm::dyn_cast<llvm::CallInst>(user)) {
          if (callInst->getFunction() == &func) {
            hasEnd = true;
            break;
          }
        }
      }

      if (!hasStart) {
        builder.SetInsertPoint(&func.getEntryBlock().front());
        builder.CreateCall(startInstr);
      }

      if (!hasEnd) {
        for (llvm::BasicBlock &BB : func) {
          if (llvm::dyn_cast<llvm::ReturnInst>(BB.getTerminator())) {
            builder.SetInsertPoint(BB.getTerminator());
            builder.CreateCall(endInstr);
          }
        }
      }
      return llvm::PreservedAnalyses::all();
    }

    return llvm::PreservedAnalyses::none();
  }

  static bool isRequired() { return true; }
};
} // namespace

extern "C" LLVM_ATTRIBUTE_WEAK ::llvm::PassPluginLibraryInfo
llvmGetPassPluginInfo() {
  return {LLVM_PLUGIN_API_VERSION, "mgk_func_wrapper", "0.1",
          [](llvm::PassBuilder &PB) {
            PB.registerPipelineParsingCallback(
                [](llvm::StringRef name, llvm::FunctionPassManager &FPM,
                   llvm::ArrayRef<llvm::PassBuilder::PipelineElement>) -> bool {
                  if (name == "mgk_func_wrapper") {
                    FPM.addPass(FunctionInstrumentation{});
                    return true;
                  }
                  return false;
                });
          }};
}