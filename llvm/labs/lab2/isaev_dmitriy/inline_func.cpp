#include "llvm/ADT/SmallPtrSet.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Transforms/Utils/ValueMapper.h"

namespace {

struct IsaevInlinePass : public llvm::PassInfoMixin<IsaevInlinePass> {
  llvm::PreservedAnalyses run(llvm::Function &Func,
                              llvm::FunctionAnalysisManager &) {
    llvm::SmallVector<llvm::CallInst *, 8> CallsToInline;
    llvm::IRBuilder<> Builder(Func.getContext());

    for (llvm::BasicBlock &Block : Func) {
      for (llvm::Instruction &Instr : Block) {
        if (auto *CI = llvm::dyn_cast<llvm::CallInst>(&Instr)) {
          llvm::Function *Callee = CI->getCalledFunction();
          if (Callee && Callee->arg_empty() &&
              Callee->getReturnType()->isVoidTy()) {
            CallsToInline.push_back(CI);
          }
        }
      }
    }

    for (auto *CI : CallsToInline) {
      llvm::BasicBlock *InsertionBlock = CI->getParent();
      llvm::ValueToValueMapTy ValueMap;
      llvm::BasicBlock *PostCallBB =
          InsertionBlock->splitBasicBlock(CI->getIterator(), "post-call");
      llvm::Function *Callee = CI->getCalledFunction();
      llvm::BasicBlock *PrevBB = nullptr;
      llvm::BasicBlock *CurrentBB = nullptr;

      // Clone each basic block from Callee and create branches
      for (llvm::BasicBlock &CalleeBB : *Callee) {
        CurrentBB =
            llvm::BasicBlock::Create(Func.getContext(), "", &Func, PostCallBB);
        ValueMap[&CalleeBB] = CurrentBB;
        Builder.SetInsertPoint(CurrentBB);
        for (llvm::Instruction &Inst : CalleeBB) {
          if (Inst.isTerminator()) {
            break;
          }
          llvm::Instruction *NewInst = Inst.clone();
          Builder.Insert(NewInst);
          ValueMap[&Inst] = NewInst;
        }

        if (PrevBB) {
          if (PrevBB->getTerminator()) {
            PrevBB->getTerminator()->eraseFromParent();
          }
          Builder.SetInsertPoint(PrevBB);
          Builder.CreateBr(CurrentBB);
        }
        PrevBB = CurrentBB;
      }

      if (PrevBB) {
        if (PrevBB->getTerminator()) {
          PrevBB->getTerminator()->eraseFromParent();
        }
        Builder.SetInsertPoint(PrevBB);
        Builder.CreateBr(PostCallBB);
      }

      // Fix operands of newly cloned instructions
      for (auto Iter = ValueMap.begin(); Iter != ValueMap.end(); ++Iter) {
        if (llvm::BasicBlock *NewBB =
                llvm::dyn_cast<llvm::BasicBlock>(Iter->second)) {
          for (llvm::Instruction &Inst : *NewBB) {
            for (llvm::Use &Op : Inst.operands()) {
              if (ValueMap.count(Op)) {
                Op.set(ValueMap[Op]);
              }
            }
          }
        }
      }

      for (auto &Use : CI->uses()) {
        llvm::User *User = Use.getUser();
        for (unsigned i = 0; i < User->getNumOperands(); ++i) {
          if (ValueMap.count(User->getOperand(i))) {
            User->getOperand(i)->replaceAllUsesWith(
                ValueMap[User->getOperand(i)]);
          }
        }
      }

      llvm::BasicBlock *PostCallBBNext = PostCallBB->getNextNode();
      if (PostCallBBNext) {
        llvm::Instruction *Term = PostCallBB->getTerminator();
        Term->eraseFromParent();
        Builder.SetInsertPoint(PostCallBB);
        Builder.CreateBr(PostCallBBNext);
      }

      CI->eraseFromParent();
    }
    return llvm::PreservedAnalyses::none();
  }
};

} // namespace

llvm::PassPluginLibraryInfo getIsaevInlinePluginInfo() {
  return {LLVM_PLUGIN_API_VERSION, "IsaevInlinePass", "0.1",
          [](llvm::PassBuilder &PB) {
            PB.registerPipelineParsingCallback(
                [](llvm::StringRef Name, llvm::FunctionPassManager &PM,
                   llvm::ArrayRef<llvm::PassBuilder::PipelineElement>) {
                  if (Name == "isaev-inline") {
                    PM.addPass(IsaevInlinePass());
                    return true;
                  }
                  return false;
                });
          }};
}

extern "C" LLVM_ATTRIBUTE_WEAK ::llvm::PassPluginLibraryInfo
llvmGetPassPluginInfo() {
  return getIsaevInlinePluginInfo();
}