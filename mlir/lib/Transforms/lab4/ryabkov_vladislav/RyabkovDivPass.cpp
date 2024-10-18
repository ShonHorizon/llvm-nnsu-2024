#include "mlir/Dialect/Arith/IR/Arith.h"
#include "mlir/Dialect/Func/IR/FuncOps.h"
#include "mlir/Dialect/LLVMIR/LLVMDialect.h"
#include "mlir/IR/PatternMatch.h"
#include "mlir/Pass/Pass.h"
#include "mlir/Tools/Plugins/PassPlugin.h"

using namespace mlir;

class RyabkovDivPass
    : public PassWrapper<RyabkovDivPass, OperationPass<LLVM::LLVMFuncOp>> {
public:
  StringRef getArgument() const final { return "ryabkov_ceildiv"; }
  StringRef getDescription() const final {
    return "Converts arith.ceildivui and arith.ceildivsi operations "
           "into a sequence of basic arithmetic operations.";
  }

  void runOnOperation() override {
    getOperation()->walk(
        [&](arith::CeilDivUIOp ceilDivOp) { processCeilDiv(ceilDivOp); });

    getOperation()->walk(
        [&](arith::CeilDivSIOp ceilDivSIOp) { processCeilDiv(ceilDivSIOp); });
  }

private:
  void processCeilDiv(arith::CeilDivUIOp ceilDivOp) {
    OpBuilder opBuilder(ceilDivOp.getContext());
    opBuilder.setInsertionPoint(ceilDivOp);

    Value lhsValue = ceilDivOp.getLhs();
    Value rhsValue = ceilDivOp.getRhs();
    Type lhsType = lhsValue.getType();
    arith::ConstantOp constOne = opBuilder.create<arith::ConstantOp>(
        ceilDivOp.getLoc(), lhsType, opBuilder.getIntegerAttr(lhsType, 1));
    arith::AddIOp addOp =
        opBuilder.create<arith::AddIOp>(ceilDivOp.getLoc(), lhsValue, rhsValue);
    arith::SubIOp subOp =
        opBuilder.create<arith::SubIOp>(ceilDivOp.getLoc(), addOp, constOne);
    arith::DivUIOp divOp =
        opBuilder.create<arith::DivUIOp>(ceilDivOp.getLoc(), subOp, rhsValue);

    ceilDivOp.replaceAllUsesWith(divOp.getOperation());
    ceilDivOp.erase();
  }

  void processCeilDiv(arith::CeilDivSIOp ceilDivSIOp) {
    OpBuilder opBuilder(ceilDivSIOp.getContext());
    opBuilder.setInsertionPoint(ceilDivSIOp);

    Value lhsValue = ceilDivSIOp.getLhs();
    Value rhsValue = ceilDivSIOp.getRhs();
    Type lhsType = lhsValue.getType();
    arith::ConstantOp constOne = opBuilder.create<arith::ConstantOp>(
        ceilDivSIOp.getLoc(), lhsType, opBuilder.getIntegerAttr(lhsType, 1));
    arith::AddIOp addOp = opBuilder.create<arith::AddIOp>(ceilDivSIOp.getLoc(),
                                                          lhsValue, rhsValue);
    arith::SubIOp subOp =
        opBuilder.create<arith::SubIOp>(ceilDivSIOp.getLoc(), addOp, constOne);
    arith::DivSIOp divOp =
        opBuilder.create<arith::DivSIOp>(ceilDivSIOp.getLoc(), subOp, rhsValue);

    ceilDivSIOp.replaceAllUsesWith(divOp.getOperation());
    ceilDivSIOp.erase();
  }
};

MLIR_DECLARE_EXPLICIT_TYPE_ID(RyabkovDivPass)
MLIR_DEFINE_EXPLICIT_TYPE_ID(RyabkovDivPass)

PassPluginLibraryInfo RyabkovDivPassPluginInfo() {
  return {MLIR_PLUGIN_API_VERSION, "ryabkov_ceildiv", LLVM_VERSION_STRING,
          []() { PassRegistration<RyabkovDivPass>(); }};
}

extern "C" LLVM_ATTRIBUTE_WEAK PassPluginLibraryInfo mlirGetPassPluginInfo() {
  return RyabkovDivPassPluginInfo();
}
