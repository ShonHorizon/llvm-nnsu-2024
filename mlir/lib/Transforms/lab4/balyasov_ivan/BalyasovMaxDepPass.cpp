#include "mlir/Dialect/Func/IR/FuncOps.h"
#include "mlir/Dialect/LLVMIR/LLVMDialect.h"
#include "mlir/Dialect/SCF/IR/SCF.h"
#include "mlir/IR/BuiltinOps.h"
#include "mlir/IR/Operation.h"
#include "mlir/IR/PatternMatch.h"
#include "mlir/IR/Region.h"
#include "mlir/Pass/Pass.h"
#include "mlir/Tools/Plugins/PassPlugin.h"

using namespace mlir;

namespace {
class BalyasovMaxDepPass
    : public PassWrapper<BalyasovMaxDepPass, OperationPass<func::FuncOp>> {
public:
  StringRef getArgument() const final { return "BalyasovMaxDepPass"; }
  StringRef getDescription() const final {
    return "Calculates the maximum depth of nested regions in the function.";
  }

  void runOnOperation() override {
    getOperation()->walk([this](Operation *operation) {
      int maxDepth = calculateMaxDepth(operation);
      operation->setAttr(
          "maxDepth",
          IntegerAttr::get(IntegerType::get(operation->getContext(), 32),
                           maxDepth));
    });
  }

private:
  int calculateMaxDepth(Operation *operation, int currentDepth = 0) {
    int maxDepth = currentDepth;

    for (Region &region : operation->getRegions()) {
      for (Block &block : region) {
        for (Operation &nestedOperation : block) {
          maxDepth = std::max(
              maxDepth, calculateMaxDepth(&nestedOperation, currentDepth + 1));
        }
      }
    }

    return maxDepth;
  }
};
} // namespace

MLIR_DECLARE_EXPLICIT_TYPE_ID(BalyasovMaxDepPass)
MLIR_DEFINE_EXPLICIT_TYPE_ID(BalyasovMaxDepPass)

PassPluginLibraryInfo getFunctionCallCounterPassPluginInfo() {
  return {MLIR_PLUGIN_API_VERSION, "BalyasovMaxDepPass", LLVM_VERSION_STRING,
          []() { PassRegistration<BalyasovMaxDepPass>(); }};
}

extern "C" LLVM_ATTRIBUTE_WEAK PassPluginLibraryInfo mlirGetPassPluginInfo() {
  return getFunctionCallCounterPassPluginInfo();
}
