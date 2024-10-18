#include "X86.h"
#include "X86InstrInfo.h"
#include "llvm/CodeGen/MachineBasicBlock.h"
#include "llvm/CodeGen/MachineFunctionPass.h"
#include "llvm/CodeGen/MachineInstr.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/Register.h"
#include <utility>
#include <vector>

using namespace llvm;

namespace {
class X86PassMGK : public MachineFunctionPass {
public:
  static char ID;

  X86PassMGK() : MachineFunctionPass(ID) {}

  bool runOnMachineFunction(MachineFunction &Mfunc) {

    bool ismodified = false;
    const TargetInstrInfo *info = Mfunc.getSubtarget().getInstrInfo();

    for (auto &basicblock : Mfunc) {

      std::vector<llvm::MachineBasicBlock::iterator> Erase;
      llvm::Register addReg2;
      MachineInstr *AInstr;

      for (auto instruction = basicblock.begin();
           instruction != basicblock.end(); ++instruction) {

        if (instruction->getOpcode() == X86::MULPDrr) {

          const llvm::Register mullReg0 = instruction->getOperand(0).getReg();
          bool foundAdd = false;

          for (auto next = instruction; next != basicblock.end(); ++next) {

            if (next->getOpcode() == X86::ADDPDrr) {

              llvm::Register addReg1 = next->getOperand(1).getReg();
              addReg2 = next->getOperand(2).getReg();

              if (mullReg0 != addReg1 && mullReg0 != addReg2)
                continue;

              if (foundAdd) {
                foundAdd = false;
                break;
              }

              if (mullReg0 == addReg2) {
                addReg2 = addReg1;
              }

              AInstr = &(*next);
              foundAdd = true;
            }
          }

          if (!foundAdd)
            continue;

          auto &MInstr = *instruction;

          MIMetadata MIMD(*AInstr);

          MachineInstrBuilder builder =
              BuildMI(basicblock, *AInstr, MIMD, info->get(X86::VFMADD213PDr));
          builder.addReg(AInstr->getOperand(0).getReg(), RegState::Define);
          builder.addReg(MInstr.getOperand(1).getReg());
          builder.addReg(MInstr.getOperand(2).getReg());
          builder.addReg(addReg2);
          AInstr->eraseFromParent();
          Erase.emplace_back(instruction);
          ismodified = true;
        }
      }
      for (auto &mul : Erase) {
        mul->eraseFromParent();
      }
    }

    return ismodified;
  }
};
} // namespace

char X86PassMGK::ID = 0;

static RegisterPass<X86PassMGK> X("x86-pass-mgk", "X86 Pass MGK", false, false);
