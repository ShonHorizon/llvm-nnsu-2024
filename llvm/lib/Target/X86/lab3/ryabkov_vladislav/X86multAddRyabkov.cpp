#include "X86.h"
#include "X86InstrInfo.h"
#include "X86Subtarget.h"
#include "llvm/CodeGen/MachineFunctionPass.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include <utility>
#include <vector>

using namespace llvm;

namespace {
class X86multAddRyabkovPass : public MachineFunctionPass {
public:
  static char ID;
  X86multAddRyabkovPass() : MachineFunctionPass(ID) {}
  bool runOnMachineFunction(MachineFunction &mach_func) override {
    const TargetInstrInfo *information =
        mach_func.getSubtarget().getInstrInfo();
    std::vector<std::pair<MachineInstr *, MachineInstr *>> instruction_delete;
    bool modify = false;
    bool regstr = false;

    for (auto &mach_basic_block : mach_func) {
      MachineInstr *instruction_mult = nullptr;
      MachineInstr *instruction_add = nullptr;
      Register mult_regstr;
      Register add_first_regstr;
      Register add_second_regstr;
      for (auto &instruction : mach_basic_block) {
        if (instruction.getOpcode() == X86::MULPDrr) {
          instruction_mult = &instruction;
          auto instruction_next = std::next(instruction.getIterator());
          for (; instruction_next != mach_basic_block.end();
               ++instruction_next) {
            if (instruction_next->getOpcode() == X86::ADDPDrr) {
              instruction_add = &*instruction_next;
              mult_regstr = instruction_mult->getOperand(0).getReg();
              add_first_regstr = instruction_add->getOperand(1).getReg();
              add_second_regstr = instruction_add->getOperand(2).getReg();
              if (mult_regstr == add_first_regstr ||
                  mult_regstr == add_second_regstr) {
                instruction_delete.emplace_back(instruction_mult,
                                                instruction_add);
                modify = true;
                if (mult_regstr == add_first_regstr) {
                  regstr = false;
                } else {
                  regstr = true;
                }
                break;
              }
            } else if (instruction_next->definesRegister(
                           instruction_mult->getOperand(0).getReg())) {
              break;
            }
          }
        }
      }
    }

    for (auto &[mul, add] : instruction_delete) {
      MachineInstrBuilder builder =
          BuildMI(*mul->getParent(), *mul, mul->getDebugLoc(),
                  information->get(X86::VFMADD213PDZ128r));
      builder.addReg(add->getOperand(0).getReg(), RegState::Define);
      builder.addReg(mul->getOperand(1).getReg());
      builder.addReg(mul->getOperand(2).getReg());
      if (regstr) {
        builder.addReg(add->getOperand(1).getReg());
      } else {
        builder.addReg(add->getOperand(2).getReg());
      }
      mul->eraseFromParent();
      add->eraseFromParent();
    }

    return modify;
  }
};
} // namespace

char X86multAddRyabkovPass::ID = 0;
static RegisterPass<X86multAddRyabkovPass> X("x86-mult-add-ryabkov-pass",
                                             "RyabkovVA X86", false, false);