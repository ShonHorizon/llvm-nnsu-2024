#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Support/raw_ostream.h"

namespace {

struct ShmelevFuncInstrumentationPass
    : llvm::PassInfoMixin<ShmelevFuncInstrumentationPass> {

  llvm::PreservedAnalyses
  run(llvm::Function &function,
      llvm::FunctionAnalysisManager &func_analysis_manager) {

    llvm::LLVMContext &con = function.getContext();

    llvm::IRBuilder<> builder(con);

    auto module = function.getParent();

    llvm::FunctionType *func_type =
        llvm::FunctionType::get(llvm::Type::getVoidTy(con), false);

    llvm::FunctionCallee start_instr =
        module->getOrInsertFunction("instrument_start", func_type);

    llvm::FunctionCallee end_instr =
        module->getOrInsertFunction("instrument_end", func_type);

    bool start_func_add = false;
    bool end_func_add = false;

    llvm::CallInst *call_inst;

    for (auto *user : start_instr.getCallee()->users()) {
      if ((call_inst = llvm::dyn_cast<llvm::CallInst>(user)) != NULL) {
        call_inst = llvm::cast<llvm::CallInst>(user);
        if (call_inst->getParent()->getParent() == &function) {
          start_func_add = true;
          break;
        }
      }
    }

    for (auto *user : end_instr.getCallee()->users()) {
      if ((call_inst = llvm::dyn_cast<llvm::CallInst>(user)) != NULL) {
        call_inst = llvm::cast<llvm::CallInst>(user);
        if (call_inst->getParent()->getParent() == &function) {
          end_func_add = true;
          break;
        }
      }
    }

    if (!start_func_add) {
      builder.SetInsertPoint(&function.getEntryBlock().front());
      builder.CreateCall(start_instr);
    }

    if (!end_func_add) {
      llvm::ReturnInst *return_inst;
      for (llvm::BasicBlock &basic_block : function) {
        if ((return_inst = llvm::dyn_cast<llvm::ReturnInst>(
                 basic_block.getTerminator())) != NULL) {
          builder.SetInsertPoint(basic_block.getTerminator());
          builder.CreateCall(end_instr);
        }
      }
    }

    return llvm::PreservedAnalyses::all();
  }

  static bool isRequired() { return true; }
};
} // namespace

extern "C" LLVM_ATTRIBUTE_WEAK::llvm::PassPluginLibraryInfo
llvmGetPassPluginInfo() {
  return {LLVM_PLUGIN_API_VERSION, "instrument_functions", "0.1",
          [](llvm::PassBuilder &pass_builder) {
            pass_builder.registerPipelineParsingCallback(
                [](llvm::StringRef name,
                   llvm::FunctionPassManager &func_pass_manager,
                   llvm::ArrayRef<llvm::PassBuilder::PipelineElement>) -> bool {
                  if (name == "instrument_functions") {
                    func_pass_manager.addPass(ShmelevFuncInstrumentationPass{});
                    return true;
                  }
                  return false;
                });
          }};
}