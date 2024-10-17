#include "X86.h"
#include "X86InstrInfo.h"
#include "X86Subtarget.h"
#include "llvm/CodeGen/MachineFunctionPass.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include <utility>
#include <vector>

using namespace llvm;

namespace {
class x86ZawadaMulAddPass : public MachineFunctionPass {
public:
  static char ID;
  x86ZawadaMulAddPass() : MachineFunctionPass(ID) {}

  bool runOnMachineFunction(MachineFunction &machineFunction) override {
    const TargetInstrInfo *targetInstrInfo =
        machineFunction.getSubtarget().getInstrInfo();
    std::vector<std::pair<MachineInstr *, MachineInstr *>> delInstr;
    bool flag = false;
    bool reg = false;

    for (auto &MBB : machineFunction) {
      MachineInstr *mulInstr = nullptr;
      MachineInstr *addInstr = nullptr;
      Register regMul;
      Register regAdd1;
      Register regAdd2;

      for (auto &instr : MBB) {
        if (instr.getOpcode() == X86::MULPDrr) {
          mulInstr = &instr;
          auto nextInstr = std::next(instr.getIterator());

          for (nextInstr; nextInstr != MBB.end(); ++nextInstr) {
            if (nextInstr->getOpcode() == X86::ADDPDrr) {
              addInstr = &*nextInstr;
              regMul = mulInstr->getOperand(0).getReg();
              regAdd1 = addInstr->getOperand(1).getReg();
              regAdd2 = addInstr->getOperand(2).getReg();

              if (regMul == regAdd1 || regMul == regAdd2) {
                delInstr.emplace_back(mulInstr, addInstr);
                flag = true;

                reg = regMul != regAdd1;

                break;
              }
            } else if (nextInstr->definesRegister(
                           mulInstr->getOperand(0).getReg()))
              break;
          }
        }
      }
    }

    for (auto &[mul, add] : delInstr) {
      MachineInstrBuilder machineInstrBuilder =
          BuildMI(*mul->getParent(), *mul, mul->getDebugLoc(),
                  targetInstrInfo->get(X86::VFMADD213PDZ128r));
      machineInstrBuilder.addReg(add->getOperand(0).getReg(), RegState::Define);
      machineInstrBuilder.addReg(mul->getOperand(1).getReg());
      machineInstrBuilder.addReg(mul->getOperand(2).getReg());

      if (!reg)
        machineInstrBuilder.addReg(add->getOperand(2).getReg());
      else
        machineInstrBuilder.addReg(add->getOperand(1).getReg());

      mul->eraseFromParent();
      add->eraseFromParent();
    }

    return flag;
  }
};
} // namespace

char x86ZawadaMulAddPass::ID = 0;
static RegisterPass<x86ZawadaMulAddPass> X("x86-mul-add-pass",
                                           "x86 Zawada Pass", false, false);