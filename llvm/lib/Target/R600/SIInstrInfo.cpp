//===-- SIInstrInfo.cpp - SI Instruction Information  ---------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
/// \file
/// \brief SI Implementation of TargetInstrInfo.
//
//===----------------------------------------------------------------------===//


#include "SIInstrInfo.h"
#include "AMDGPUTargetMachine.h"
#include "SIDefines.h"
#include "SIMachineFunctionInfo.h"
#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/IR/Function.h"
#include "llvm/MC/MCInstrDesc.h"

using namespace llvm;

SIInstrInfo::SIInstrInfo(const AMDGPUSubtarget &st)
  : AMDGPUInstrInfo(st),
    RI(st) { }

//===----------------------------------------------------------------------===//
// TargetInstrInfo callbacks
//===----------------------------------------------------------------------===//

static unsigned getNumOperandsNoGlue(SDNode *Node) {
  unsigned N = Node->getNumOperands();
  while (N && Node->getOperand(N - 1).getValueType() == MVT::Glue)
    --N;
  return N;
}

static SDValue findChainOperand(SDNode *Load) {
  SDValue LastOp = Load->getOperand(getNumOperandsNoGlue(Load) - 1);
  assert(LastOp.getValueType() == MVT::Other && "Chain missing from load node");
  return LastOp;
}

/// \brief Returns true if both nodes have the same value for the given
///        operand \p Op, or if both nodes do not have this operand.
static bool nodesHaveSameOperandValue(SDNode *N0, SDNode* N1, unsigned OpName) {
  unsigned Opc0 = N0->getMachineOpcode();
  unsigned Opc1 = N1->getMachineOpcode();

  int Op0Idx = AMDGPU::getNamedOperandIdx(Opc0, OpName);
  int Op1Idx = AMDGPU::getNamedOperandIdx(Opc1, OpName);

  if (Op0Idx == -1 && Op1Idx == -1)
    return true;


  if ((Op0Idx == -1 && Op1Idx != -1) ||
      (Op1Idx == -1 && Op0Idx != -1))
    return false;

  // getNamedOperandIdx returns the index for the MachineInstr's operands,
  // which includes the result as the first operand. We are indexing into the
  // MachineSDNode's operands, so we need to skip the result operand to get
  // the real index.
  --Op0Idx;
  --Op1Idx;

  return N0->getOperand(Op0Idx) == N1->getOperand(Op1Idx);
}

bool SIInstrInfo::areLoadsFromSameBasePtr(SDNode *Load0, SDNode *Load1,
                                          int64_t &Offset0,
                                          int64_t &Offset1) const {
  if (!Load0->isMachineOpcode() || !Load1->isMachineOpcode())
    return false;

  unsigned Opc0 = Load0->getMachineOpcode();
  unsigned Opc1 = Load1->getMachineOpcode();

  // Make sure both are actually loads.
  if (!get(Opc0).mayLoad() || !get(Opc1).mayLoad())
    return false;

  if (isDS(Opc0) && isDS(Opc1)) {
    assert(getNumOperandsNoGlue(Load0) == getNumOperandsNoGlue(Load1));

    // TODO: Also shouldn't see read2st
    assert(Opc0 != AMDGPU::DS_READ2_B32 &&
           Opc0 != AMDGPU::DS_READ2_B64 &&
           Opc1 != AMDGPU::DS_READ2_B32 &&
           Opc1 != AMDGPU::DS_READ2_B64);

    // Check base reg.
    if (Load0->getOperand(1) != Load1->getOperand(1))
      return false;

    // Check chain.
    if (findChainOperand(Load0) != findChainOperand(Load1))
      return false;

    Offset0 = cast<ConstantSDNode>(Load0->getOperand(2))->getZExtValue();
    Offset1 = cast<ConstantSDNode>(Load1->getOperand(2))->getZExtValue();
    return true;
  }

  if (isSMRD(Opc0) && isSMRD(Opc1)) {
    assert(getNumOperandsNoGlue(Load0) == getNumOperandsNoGlue(Load1));

    // Check base reg.
    if (Load0->getOperand(0) != Load1->getOperand(0))
      return false;

    // Check chain.
    if (findChainOperand(Load0) != findChainOperand(Load1))
      return false;

    Offset0 = cast<ConstantSDNode>(Load0->getOperand(1))->getZExtValue();
    Offset1 = cast<ConstantSDNode>(Load1->getOperand(1))->getZExtValue();
    return true;
  }

  // MUBUF and MTBUF can access the same addresses.
  if ((isMUBUF(Opc0) || isMTBUF(Opc0)) && (isMUBUF(Opc1) || isMTBUF(Opc1))) {

    // MUBUF and MTBUF have vaddr at different indices.
    if (!nodesHaveSameOperandValue(Load0, Load1, AMDGPU::OpName::soffset) ||
        findChainOperand(Load0) != findChainOperand(Load1) ||
        !nodesHaveSameOperandValue(Load0, Load1, AMDGPU::OpName::vaddr) ||
        !nodesHaveSameOperandValue(Load0, Load1, AMDGPU::OpName::srsrc))
      return false;

    int OffIdx0 = AMDGPU::getNamedOperandIdx(Opc0, AMDGPU::OpName::offset);
    int OffIdx1 = AMDGPU::getNamedOperandIdx(Opc1, AMDGPU::OpName::offset);

    if (OffIdx0 == -1 || OffIdx1 == -1)
      return false;

    // getNamedOperandIdx returns the index for MachineInstrs.  Since they
    // inlcude the output in the operand list, but SDNodes don't, we need to
    // subtract the index by one.
    --OffIdx0;
    --OffIdx1;

    SDValue Off0 = Load0->getOperand(OffIdx0);
    SDValue Off1 = Load1->getOperand(OffIdx1);

    // The offset might be a FrameIndexSDNode.
    if (!isa<ConstantSDNode>(Off0) || !isa<ConstantSDNode>(Off1))
      return false;

    Offset0 = cast<ConstantSDNode>(Off0)->getZExtValue();
    Offset1 = cast<ConstantSDNode>(Off1)->getZExtValue();
    return true;
  }

  return false;
}

bool SIInstrInfo::getLdStBaseRegImmOfs(MachineInstr *LdSt,
                                       unsigned &BaseReg, unsigned &Offset,
                                       const TargetRegisterInfo *TRI) const {
  unsigned Opc = LdSt->getOpcode();
  if (isDS(Opc)) {
    const MachineOperand *OffsetImm = getNamedOperand(*LdSt,
                                                      AMDGPU::OpName::offset);
    if (OffsetImm) {
      // Normal, single offset LDS instruction.
      const MachineOperand *AddrReg = getNamedOperand(*LdSt,
                                                      AMDGPU::OpName::addr);

      BaseReg = AddrReg->getReg();
      Offset = OffsetImm->getImm();
      return true;
    }

    // The 2 offset instructions use offset0 and offset1 instead. We can treat
    // these as a load with a single offset if the 2 offsets are consecutive. We
    // will use this for some partially aligned loads.
    const MachineOperand *Offset0Imm = getNamedOperand(*LdSt,
                                                       AMDGPU::OpName::offset0);
    const MachineOperand *Offset1Imm = getNamedOperand(*LdSt,
                                                       AMDGPU::OpName::offset1);

    uint8_t Offset0 = Offset0Imm->getImm();
    uint8_t Offset1 = Offset1Imm->getImm();
    assert(Offset1 > Offset0);

    if (Offset1 - Offset0 == 1) {
      // Each of these offsets is in element sized units, so we need to convert
      // to bytes of the individual reads.

      unsigned EltSize;
      if (LdSt->mayLoad())
        EltSize = getOpRegClass(*LdSt, 0)->getSize() / 2;
      else {
        assert(LdSt->mayStore());
        int Data0Idx = AMDGPU::getNamedOperandIdx(Opc, AMDGPU::OpName::data0);
        EltSize = getOpRegClass(*LdSt, Data0Idx)->getSize();
      }

      const MachineOperand *AddrReg = getNamedOperand(*LdSt,
                                                      AMDGPU::OpName::addr);
      BaseReg = AddrReg->getReg();
      Offset = EltSize * Offset0;
      return true;
    }

    return false;
  }

  if (isMUBUF(Opc) || isMTBUF(Opc)) {
    if (AMDGPU::getNamedOperandIdx(Opc, AMDGPU::OpName::soffset) != -1)
      return false;

    const MachineOperand *AddrReg = getNamedOperand(*LdSt,
                                                    AMDGPU::OpName::vaddr);
    if (!AddrReg)
      return false;

    const MachineOperand *OffsetImm = getNamedOperand(*LdSt,
                                                      AMDGPU::OpName::offset);
    BaseReg = AddrReg->getReg();
    Offset = OffsetImm->getImm();
    return true;
  }

  if (isSMRD(Opc)) {
    const MachineOperand *OffsetImm = getNamedOperand(*LdSt,
                                                      AMDGPU::OpName::offset);
    if (!OffsetImm)
      return false;

    const MachineOperand *SBaseReg = getNamedOperand(*LdSt,
                                                     AMDGPU::OpName::sbase);
    BaseReg = SBaseReg->getReg();
    Offset = OffsetImm->getImm();
    return true;
  }

  return false;
}

void
SIInstrInfo::copyPhysReg(MachineBasicBlock &MBB,
                         MachineBasicBlock::iterator MI, DebugLoc DL,
                         unsigned DestReg, unsigned SrcReg,
                         bool KillSrc) const {

  // If we are trying to copy to or from SCC, there is a bug somewhere else in
  // the backend.  While it may be theoretically possible to do this, it should
  // never be necessary.
  assert(DestReg != AMDGPU::SCC && SrcReg != AMDGPU::SCC);

  static const int16_t Sub0_15[] = {
    AMDGPU::sub0, AMDGPU::sub1, AMDGPU::sub2, AMDGPU::sub3,
    AMDGPU::sub4, AMDGPU::sub5, AMDGPU::sub6, AMDGPU::sub7,
    AMDGPU::sub8, AMDGPU::sub9, AMDGPU::sub10, AMDGPU::sub11,
    AMDGPU::sub12, AMDGPU::sub13, AMDGPU::sub14, AMDGPU::sub15, 0
  };

  static const int16_t Sub0_7[] = {
    AMDGPU::sub0, AMDGPU::sub1, AMDGPU::sub2, AMDGPU::sub3,
    AMDGPU::sub4, AMDGPU::sub5, AMDGPU::sub6, AMDGPU::sub7, 0
  };

  static const int16_t Sub0_3[] = {
    AMDGPU::sub0, AMDGPU::sub1, AMDGPU::sub2, AMDGPU::sub3, 0
  };

  static const int16_t Sub0_2[] = {
    AMDGPU::sub0, AMDGPU::sub1, AMDGPU::sub2, 0
  };

  static const int16_t Sub0_1[] = {
    AMDGPU::sub0, AMDGPU::sub1, 0
  };

  unsigned Opcode;
  const int16_t *SubIndices;

  if (AMDGPU::M0 == DestReg) {
    // Check if M0 isn't already set to this value
    for (MachineBasicBlock::reverse_iterator E = MBB.rend(),
      I = MachineBasicBlock::reverse_iterator(MI); I != E; ++I) {

      if (!I->definesRegister(AMDGPU::M0))
        continue;

      unsigned Opc = I->getOpcode();
      if (Opc != TargetOpcode::COPY && Opc != AMDGPU::S_MOV_B32)
        break;

      if (!I->readsRegister(SrcReg))
        break;

      // The copy isn't necessary
      return;
    }
  }

  if (AMDGPU::SReg_32RegClass.contains(DestReg)) {
    assert(AMDGPU::SReg_32RegClass.contains(SrcReg));
    BuildMI(MBB, MI, DL, get(AMDGPU::S_MOV_B32), DestReg)
            .addReg(SrcReg, getKillRegState(KillSrc));
    return;

  } else if (AMDGPU::SReg_64RegClass.contains(DestReg)) {
    assert(AMDGPU::SReg_64RegClass.contains(SrcReg));
    BuildMI(MBB, MI, DL, get(AMDGPU::S_MOV_B64), DestReg)
            .addReg(SrcReg, getKillRegState(KillSrc));
    return;

  } else if (AMDGPU::SReg_128RegClass.contains(DestReg)) {
    assert(AMDGPU::SReg_128RegClass.contains(SrcReg));
    Opcode = AMDGPU::S_MOV_B32;
    SubIndices = Sub0_3;

  } else if (AMDGPU::SReg_256RegClass.contains(DestReg)) {
    assert(AMDGPU::SReg_256RegClass.contains(SrcReg));
    Opcode = AMDGPU::S_MOV_B32;
    SubIndices = Sub0_7;

  } else if (AMDGPU::SReg_512RegClass.contains(DestReg)) {
    assert(AMDGPU::SReg_512RegClass.contains(SrcReg));
    Opcode = AMDGPU::S_MOV_B32;
    SubIndices = Sub0_15;

  } else if (AMDGPU::VReg_32RegClass.contains(DestReg)) {
    assert(AMDGPU::VReg_32RegClass.contains(SrcReg) ||
           AMDGPU::SReg_32RegClass.contains(SrcReg));
    BuildMI(MBB, MI, DL, get(AMDGPU::V_MOV_B32_e32), DestReg)
            .addReg(SrcReg, getKillRegState(KillSrc));
    return;

  } else if (AMDGPU::VReg_64RegClass.contains(DestReg)) {
    assert(AMDGPU::VReg_64RegClass.contains(SrcReg) ||
           AMDGPU::SReg_64RegClass.contains(SrcReg));
    Opcode = AMDGPU::V_MOV_B32_e32;
    SubIndices = Sub0_1;

  } else if (AMDGPU::VReg_96RegClass.contains(DestReg)) {
    assert(AMDGPU::VReg_96RegClass.contains(SrcReg));
    Opcode = AMDGPU::V_MOV_B32_e32;
    SubIndices = Sub0_2;

  } else if (AMDGPU::VReg_128RegClass.contains(DestReg)) {
    assert(AMDGPU::VReg_128RegClass.contains(SrcReg) ||
           AMDGPU::SReg_128RegClass.contains(SrcReg));
    Opcode = AMDGPU::V_MOV_B32_e32;
    SubIndices = Sub0_3;

  } else if (AMDGPU::VReg_256RegClass.contains(DestReg)) {
    assert(AMDGPU::VReg_256RegClass.contains(SrcReg) ||
           AMDGPU::SReg_256RegClass.contains(SrcReg));
    Opcode = AMDGPU::V_MOV_B32_e32;
    SubIndices = Sub0_7;

  } else if (AMDGPU::VReg_512RegClass.contains(DestReg)) {
    assert(AMDGPU::VReg_512RegClass.contains(SrcReg) ||
           AMDGPU::SReg_512RegClass.contains(SrcReg));
    Opcode = AMDGPU::V_MOV_B32_e32;
    SubIndices = Sub0_15;

  } else {
    llvm_unreachable("Can't copy register!");
  }

  while (unsigned SubIdx = *SubIndices++) {
    MachineInstrBuilder Builder = BuildMI(MBB, MI, DL,
      get(Opcode), RI.getSubReg(DestReg, SubIdx));

    Builder.addReg(RI.getSubReg(SrcReg, SubIdx), getKillRegState(KillSrc));

    if (*SubIndices)
      Builder.addReg(DestReg, RegState::Define | RegState::Implicit);
  }
}

unsigned SIInstrInfo::commuteOpcode(unsigned Opcode) const {
  int NewOpc;

  // Try to map original to commuted opcode
  if ((NewOpc = AMDGPU::getCommuteRev(Opcode)) != -1)
    return NewOpc;

  // Try to map commuted to original opcode
  if ((NewOpc = AMDGPU::getCommuteOrig(Opcode)) != -1)
    return NewOpc;

  return Opcode;
}

void SIInstrInfo::storeRegToStackSlot(MachineBasicBlock &MBB,
                                      MachineBasicBlock::iterator MI,
                                      unsigned SrcReg, bool isKill,
                                      int FrameIndex,
                                      const TargetRegisterClass *RC,
                                      const TargetRegisterInfo *TRI) const {
  MachineFunction *MF = MBB.getParent();
  MachineFrameInfo *FrameInfo = MF->getFrameInfo();
  DebugLoc DL = MBB.findDebugLoc(MI);

  if (RI.hasVGPRs(RC)) {
    LLVMContext &Ctx = MF->getFunction()->getContext();
    Ctx.emitError("SIInstrInfo::storeRegToStackSlot - Can't spill VGPR!");
    BuildMI(MBB, MI, DL, get(AMDGPU::V_MOV_B32_e32), AMDGPU::VGPR0)
            .addReg(SrcReg);
  } else if (RI.isSGPRClass(RC)) {
    // We are only allowed to create one new instruction when spilling
    // registers, so we need to use pseudo instruction for spilling
    // SGPRs.
    unsigned Opcode;
    switch (RC->getSize() * 8) {
    case 32:  Opcode = AMDGPU::SI_SPILL_S32_SAVE;  break;
    case 64:  Opcode = AMDGPU::SI_SPILL_S64_SAVE;  break;
    case 128: Opcode = AMDGPU::SI_SPILL_S128_SAVE; break;
    case 256: Opcode = AMDGPU::SI_SPILL_S256_SAVE; break;
    case 512: Opcode = AMDGPU::SI_SPILL_S512_SAVE; break;
    default: llvm_unreachable("Cannot spill register class");
    }

    FrameInfo->setObjectAlignment(FrameIndex, 4);
    BuildMI(MBB, MI, DL, get(Opcode))
            .addReg(SrcReg)
            .addFrameIndex(FrameIndex);
  } else {
    llvm_unreachable("VGPR spilling not supported");
  }
}

void SIInstrInfo::loadRegFromStackSlot(MachineBasicBlock &MBB,
                                       MachineBasicBlock::iterator MI,
                                       unsigned DestReg, int FrameIndex,
                                       const TargetRegisterClass *RC,
                                       const TargetRegisterInfo *TRI) const {
  MachineFunction *MF = MBB.getParent();
  MachineFrameInfo *FrameInfo = MF->getFrameInfo();
  DebugLoc DL = MBB.findDebugLoc(MI);

  if (RI.hasVGPRs(RC)) {
    LLVMContext &Ctx = MF->getFunction()->getContext();
    Ctx.emitError("SIInstrInfo::loadRegToStackSlot - Can't retrieve spilled VGPR!");
    BuildMI(MBB, MI, DL, get(AMDGPU::V_MOV_B32_e32), DestReg)
            .addImm(0);
  } else if (RI.isSGPRClass(RC)){
    unsigned Opcode;
    switch(RC->getSize() * 8) {
    case 32:  Opcode = AMDGPU::SI_SPILL_S32_RESTORE; break;
    case 64:  Opcode = AMDGPU::SI_SPILL_S64_RESTORE;  break;
    case 128: Opcode = AMDGPU::SI_SPILL_S128_RESTORE; break;
    case 256: Opcode = AMDGPU::SI_SPILL_S256_RESTORE; break;
    case 512: Opcode = AMDGPU::SI_SPILL_S512_RESTORE; break;
    default: llvm_unreachable("Cannot spill register class");
    }

    FrameInfo->setObjectAlignment(FrameIndex, 4);
    BuildMI(MBB, MI, DL, get(Opcode), DestReg)
            .addFrameIndex(FrameIndex);
  } else {
    llvm_unreachable("VGPR spilling not supported");
  }
}

void SIInstrInfo::insertNOPs(MachineBasicBlock::iterator MI,
                             int Count) const {
  while (Count > 0) {
    int Arg;
    if (Count >= 8)
      Arg = 7;
    else
      Arg = Count - 1;
    Count -= 8;
    BuildMI(*MI->getParent(), MI, MI->getDebugLoc(), get(AMDGPU::S_NOP))
            .addImm(Arg);
  }
}

bool SIInstrInfo::expandPostRAPseudo(MachineBasicBlock::iterator MI) const {
  MachineBasicBlock &MBB = *MI->getParent();
  DebugLoc DL = MBB.findDebugLoc(MI);
  switch (MI->getOpcode()) {
  default: return AMDGPUInstrInfo::expandPostRAPseudo(MI);

  case AMDGPU::SI_CONSTDATA_PTR: {
    unsigned Reg = MI->getOperand(0).getReg();
    unsigned RegLo = RI.getSubReg(Reg, AMDGPU::sub0);
    unsigned RegHi = RI.getSubReg(Reg, AMDGPU::sub1);

    BuildMI(MBB, MI, DL, get(AMDGPU::S_GETPC_B64), Reg);

    // Add 32-bit offset from this instruction to the start of the constant data.
    BuildMI(MBB, MI, DL, get(AMDGPU::S_ADD_I32), RegLo)
            .addReg(RegLo)
            .addTargetIndex(AMDGPU::TI_CONSTDATA_START)
            .addReg(AMDGPU::SCC, RegState::Define | RegState::Implicit);
    BuildMI(MBB, MI, DL, get(AMDGPU::S_ADDC_U32), RegHi)
            .addReg(RegHi)
            .addImm(0)
            .addReg(AMDGPU::SCC, RegState::Define | RegState::Implicit)
            .addReg(AMDGPU::SCC, RegState::Implicit);
    MI->eraseFromParent();
    break;
  }
  }
  return true;
}

MachineInstr *SIInstrInfo::commuteInstruction(MachineInstr *MI,
                                              bool NewMI) const {

  if (MI->getNumOperands() < 3 || !MI->getOperand(1).isReg())
    return nullptr;

  // Make sure it s legal to commute operands for VOP2.
  if (isVOP2(MI->getOpcode()) &&
      (!isOperandLegal(MI, 1, &MI->getOperand(2)) ||
       !isOperandLegal(MI, 2, &MI->getOperand(1))))
    return nullptr;

  if (!MI->getOperand(2).isReg()) {
    // XXX: Commute instructions with FPImm operands
    if (NewMI || MI->getOperand(2).isFPImm() ||
       (!isVOP2(MI->getOpcode()) && !isVOP3(MI->getOpcode()))) {
      return nullptr;
    }

    // XXX: Commute VOP3 instructions with abs and neg set .
    const MachineOperand *Abs = getNamedOperand(*MI, AMDGPU::OpName::abs);
    const MachineOperand *Neg = getNamedOperand(*MI, AMDGPU::OpName::neg);
    const MachineOperand *Src0Mods = getNamedOperand(*MI,
                                          AMDGPU::OpName::src0_modifiers);
    const MachineOperand *Src1Mods = getNamedOperand(*MI,
                                          AMDGPU::OpName::src1_modifiers);
    const MachineOperand *Src2Mods = getNamedOperand(*MI,
                                          AMDGPU::OpName::src2_modifiers);

    if ((Abs && Abs->getImm()) || (Neg && Neg->getImm()) ||
        (Src0Mods && Src0Mods->getImm()) || (Src1Mods && Src1Mods->getImm()) ||
        (Src2Mods && Src2Mods->getImm()))
      return nullptr;

    unsigned Reg = MI->getOperand(1).getReg();
    unsigned SubReg = MI->getOperand(1).getSubReg();
    MI->getOperand(1).ChangeToImmediate(MI->getOperand(2).getImm());
    MI->getOperand(2).ChangeToRegister(Reg, false);
    MI->getOperand(2).setSubReg(SubReg);
  } else {
    MI = TargetInstrInfo::commuteInstruction(MI, NewMI);
  }

  if (MI)
    MI->setDesc(get(commuteOpcode(MI->getOpcode())));

  return MI;
}

MachineInstr *SIInstrInfo::buildMovInstr(MachineBasicBlock *MBB,
                                         MachineBasicBlock::iterator I,
                                         unsigned DstReg,
                                         unsigned SrcReg) const {
  return BuildMI(*MBB, I, MBB->findDebugLoc(I), get(AMDGPU::V_MOV_B32_e32),
                 DstReg) .addReg(SrcReg);
}

bool SIInstrInfo::isMov(unsigned Opcode) const {
  switch(Opcode) {
  default: return false;
  case AMDGPU::S_MOV_B32:
  case AMDGPU::S_MOV_B64:
  case AMDGPU::V_MOV_B32_e32:
  case AMDGPU::V_MOV_B32_e64:
    return true;
  }
}

bool
SIInstrInfo::isSafeToMoveRegClassDefs(const TargetRegisterClass *RC) const {
  return RC != &AMDGPU::EXECRegRegClass;
}

bool
SIInstrInfo::isTriviallyReMaterializable(const MachineInstr *MI,
                                         AliasAnalysis *AA) const {
  switch(MI->getOpcode()) {
  default: return AMDGPUInstrInfo::isTriviallyReMaterializable(MI, AA);
  case AMDGPU::S_MOV_B32:
  case AMDGPU::S_MOV_B64:
  case AMDGPU::V_MOV_B32_e32:
    return MI->getOperand(1).isImm();
  }
}

namespace llvm {
namespace AMDGPU {
// Helper function generated by tablegen.  We are wrapping this with
// an SIInstrInfo function that returns bool rather than int.
int isDS(uint16_t Opcode);
}
}

bool SIInstrInfo::isDS(uint16_t Opcode) const {
  return ::AMDGPU::isDS(Opcode) != -1;
}

bool SIInstrInfo::isMIMG(uint16_t Opcode) const {
  return get(Opcode).TSFlags & SIInstrFlags::MIMG;
}

bool SIInstrInfo::isSMRD(uint16_t Opcode) const {
  return get(Opcode).TSFlags & SIInstrFlags::SMRD;
}

bool SIInstrInfo::isMUBUF(uint16_t Opcode) const {
  return get(Opcode).TSFlags & SIInstrFlags::MUBUF;
}

bool SIInstrInfo::isMTBUF(uint16_t Opcode) const {
  return get(Opcode).TSFlags & SIInstrFlags::MTBUF;
}

bool SIInstrInfo::isVOP1(uint16_t Opcode) const {
  return get(Opcode).TSFlags & SIInstrFlags::VOP1;
}

bool SIInstrInfo::isVOP2(uint16_t Opcode) const {
  return get(Opcode).TSFlags & SIInstrFlags::VOP2;
}

bool SIInstrInfo::isVOP3(uint16_t Opcode) const {
  return get(Opcode).TSFlags & SIInstrFlags::VOP3;
}

bool SIInstrInfo::isVOPC(uint16_t Opcode) const {
  return get(Opcode).TSFlags & SIInstrFlags::VOPC;
}

bool SIInstrInfo::isSALUInstr(const MachineInstr &MI) const {
  return get(MI.getOpcode()).TSFlags & SIInstrFlags::SALU;
}

bool SIInstrInfo::isInlineConstant(const APInt &Imm) const {
  int32_t Val = Imm.getSExtValue();
  if (Val >= -16 && Val <= 64)
    return true;

  // The actual type of the operand does not seem to matter as long
  // as the bits match one of the inline immediate values.  For example:
  //
  // -nan has the hexadecimal encoding of 0xfffffffe which is -2 in decimal,
  // so it is a legal inline immediate.
  //
  // 1065353216 has the hexadecimal encoding 0x3f800000 which is 1.0f in
  // floating-point, so it is a legal inline immediate.

  return (APInt::floatToBits(0.0f) == Imm) ||
         (APInt::floatToBits(1.0f) == Imm) ||
         (APInt::floatToBits(-1.0f) == Imm) ||
         (APInt::floatToBits(0.5f) == Imm) ||
         (APInt::floatToBits(-0.5f) == Imm) ||
         (APInt::floatToBits(2.0f) == Imm) ||
         (APInt::floatToBits(-2.0f) == Imm) ||
         (APInt::floatToBits(4.0f) == Imm) ||
         (APInt::floatToBits(-4.0f) == Imm);
}

bool SIInstrInfo::isInlineConstant(const MachineOperand &MO) const {
  if (MO.isImm())
    return isInlineConstant(APInt(32, MO.getImm(), true));

  if (MO.isFPImm()) {
    APFloat FpImm = MO.getFPImm()->getValueAPF();
    return isInlineConstant(FpImm.bitcastToAPInt());
  }

  return false;
}

bool SIInstrInfo::isLiteralConstant(const MachineOperand &MO) const {
  return (MO.isImm() || MO.isFPImm()) && !isInlineConstant(MO);
}

static bool compareMachineOp(const MachineOperand &Op0,
                             const MachineOperand &Op1) {
  if (Op0.getType() != Op1.getType())
    return false;

  switch (Op0.getType()) {
  case MachineOperand::MO_Register:
    return Op0.getReg() == Op1.getReg();
  case MachineOperand::MO_Immediate:
    return Op0.getImm() == Op1.getImm();
  case MachineOperand::MO_FPImmediate:
    return Op0.getFPImm() == Op1.getFPImm();
  default:
    llvm_unreachable("Didn't expect to be comparing these operand types");
  }
}

bool SIInstrInfo::isImmOperandLegal(const MachineInstr *MI, unsigned OpNo,
                                 const MachineOperand &MO) const {
  const MCOperandInfo &OpInfo = get(MI->getOpcode()).OpInfo[OpNo];

  assert(MO.isImm() || MO.isFPImm());

  if (OpInfo.OperandType == MCOI::OPERAND_IMMEDIATE)
    return true;

  if (OpInfo.RegClass < 0)
    return false;

  return RI.regClassCanUseImmediate(OpInfo.RegClass);
}

bool SIInstrInfo::canFoldOffset(unsigned OffsetSize, unsigned AS) {
  switch (AS) {
  case AMDGPUAS::GLOBAL_ADDRESS: {
    // MUBUF instructions a 12-bit offset in bytes.
    return isUInt<12>(OffsetSize);
  }
  case AMDGPUAS::CONSTANT_ADDRESS: {
    // SMRD instructions have an 8-bit offset in dwords.
    return (OffsetSize % 4 == 0) && isUInt<8>(OffsetSize / 4);
  }
  case AMDGPUAS::LOCAL_ADDRESS:
  case AMDGPUAS::REGION_ADDRESS: {
    // The single offset versions have a 16-bit offset in bytes.
    return isUInt<16>(OffsetSize);
  }
  case AMDGPUAS::PRIVATE_ADDRESS:
    // Indirect register addressing does not use any offsets.
  default:
    return 0;
  }
}

bool SIInstrInfo::hasVALU32BitEncoding(unsigned Opcode) const {
  return AMDGPU::getVOPe32(Opcode) != -1;
}

bool SIInstrInfo::hasModifiers(unsigned Opcode) const {
  // The src0_modifier operand is present on all instructions
  // that have modifiers.

  return AMDGPU::getNamedOperandIdx(Opcode,
                                    AMDGPU::OpName::src0_modifiers) != -1;
}

bool SIInstrInfo::verifyInstruction(const MachineInstr *MI,
                                    StringRef &ErrInfo) const {
  uint16_t Opcode = MI->getOpcode();
  int Src0Idx = AMDGPU::getNamedOperandIdx(Opcode, AMDGPU::OpName::src0);
  int Src1Idx = AMDGPU::getNamedOperandIdx(Opcode, AMDGPU::OpName::src1);
  int Src2Idx = AMDGPU::getNamedOperandIdx(Opcode, AMDGPU::OpName::src2);

  // Make sure the number of operands is correct.
  const MCInstrDesc &Desc = get(Opcode);
  if (!Desc.isVariadic() &&
      Desc.getNumOperands() != MI->getNumExplicitOperands()) {
     ErrInfo = "Instruction has wrong number of operands.";
     return false;
  }

  // Make sure the register classes are correct
  for (int i = 0, e = Desc.getNumOperands(); i != e; ++i) {
    switch (Desc.OpInfo[i].OperandType) {
    case MCOI::OPERAND_REGISTER: {
      int RegClass = Desc.OpInfo[i].RegClass;
      if (!RI.regClassCanUseImmediate(RegClass) &&
          (MI->getOperand(i).isImm() || MI->getOperand(i).isFPImm())) {
        // Handle some special cases:
        // Src0 can of VOP1, VOP2, VOPC can be an immediate no matter what
        // the register class.
        if (i != Src0Idx || (!isVOP1(Opcode) && !isVOP2(Opcode) &&
                                  !isVOPC(Opcode))) {
          ErrInfo = "Expected register, but got immediate";
          return false;
        }
      }
    }
      break;
    case MCOI::OPERAND_IMMEDIATE:
      // Check if this operand is an immediate.
      // FrameIndex operands will be replaced by immediates, so they are
      // allowed.
      if (!MI->getOperand(i).isImm() && !MI->getOperand(i).isFPImm() &&
          !MI->getOperand(i).isFI()) {
        ErrInfo = "Expected immediate, but got non-immediate";
        return false;
      }
      // Fall-through
    default:
      continue;
    }

    if (!MI->getOperand(i).isReg())
      continue;

    int RegClass = Desc.OpInfo[i].RegClass;
    if (RegClass != -1) {
      unsigned Reg = MI->getOperand(i).getReg();
      if (TargetRegisterInfo::isVirtualRegister(Reg))
        continue;

      const TargetRegisterClass *RC = RI.getRegClass(RegClass);
      if (!RC->contains(Reg)) {
        ErrInfo = "Operand has incorrect register class.";
        return false;
      }
    }
  }


  // Verify VOP*
  if (isVOP1(Opcode) || isVOP2(Opcode) || isVOP3(Opcode) || isVOPC(Opcode)) {
    unsigned ConstantBusCount = 0;
    unsigned SGPRUsed = AMDGPU::NoRegister;
    for (int i = 0, e = MI->getNumOperands(); i != e; ++i) {
      const MachineOperand &MO = MI->getOperand(i);
      if (MO.isReg() && MO.isUse() &&
          !TargetRegisterInfo::isVirtualRegister(MO.getReg())) {

        // EXEC register uses the constant bus.
        if (!MO.isImplicit() && MO.getReg() == AMDGPU::EXEC)
          ++ConstantBusCount;

        // SGPRs use the constant bus
        if (MO.getReg() == AMDGPU::M0 || MO.getReg() == AMDGPU::VCC ||
            (!MO.isImplicit() &&
            (AMDGPU::SGPR_32RegClass.contains(MO.getReg()) ||
            AMDGPU::SGPR_64RegClass.contains(MO.getReg())))) {
          if (SGPRUsed != MO.getReg()) {
            ++ConstantBusCount;
            SGPRUsed = MO.getReg();
          }
        }
      }
      // Literal constants use the constant bus.
      if (isLiteralConstant(MO))
        ++ConstantBusCount;
    }
    if (ConstantBusCount > 1) {
      ErrInfo = "VOP* instruction uses the constant bus more than once";
      return false;
    }
  }

  // Verify SRC1 for VOP2 and VOPC
  if (Src1Idx != -1 && (isVOP2(Opcode) || isVOPC(Opcode))) {
    const MachineOperand &Src1 = MI->getOperand(Src1Idx);
    if (Src1.isImm() || Src1.isFPImm()) {
      ErrInfo = "VOP[2C] src1 cannot be an immediate.";
      return false;
    }
  }

  // Verify VOP3
  if (isVOP3(Opcode)) {
    if (Src0Idx != -1 && isLiteralConstant(MI->getOperand(Src0Idx))) {
      ErrInfo = "VOP3 src0 cannot be a literal constant.";
      return false;
    }
    if (Src1Idx != -1 && isLiteralConstant(MI->getOperand(Src1Idx))) {
      ErrInfo = "VOP3 src1 cannot be a literal constant.";
      return false;
    }
    if (Src2Idx != -1 && isLiteralConstant(MI->getOperand(Src2Idx))) {
      ErrInfo = "VOP3 src2 cannot be a literal constant.";
      return false;
    }
  }

  // Verify misc. restrictions on specific instructions.
  if (Desc.getOpcode() == AMDGPU::V_DIV_SCALE_F32 ||
      Desc.getOpcode() == AMDGPU::V_DIV_SCALE_F64) {
    MI->dump();

    const MachineOperand &Src0 = MI->getOperand(2);
    const MachineOperand &Src1 = MI->getOperand(3);
    const MachineOperand &Src2 = MI->getOperand(4);
    if (Src0.isReg() && Src1.isReg() && Src2.isReg()) {
      if (!compareMachineOp(Src0, Src1) &&
          !compareMachineOp(Src0, Src2)) {
        ErrInfo = "v_div_scale_{f32|f64} require src0 = src1 or src2";
        return false;
      }
    }
  }

  return true;
}

unsigned SIInstrInfo::getVALUOp(const MachineInstr &MI) {
  switch (MI.getOpcode()) {
  default: return AMDGPU::INSTRUCTION_LIST_END;
  case AMDGPU::REG_SEQUENCE: return AMDGPU::REG_SEQUENCE;
  case AMDGPU::COPY: return AMDGPU::COPY;
  case AMDGPU::PHI: return AMDGPU::PHI;
  case AMDGPU::INSERT_SUBREG: return AMDGPU::INSERT_SUBREG;
  case AMDGPU::S_MOV_B32:
    return MI.getOperand(1).isReg() ?
           AMDGPU::COPY : AMDGPU::V_MOV_B32_e32;
  case AMDGPU::S_ADD_I32: return AMDGPU::V_ADD_I32_e32;
  case AMDGPU::S_ADDC_U32: return AMDGPU::V_ADDC_U32_e32;
  case AMDGPU::S_SUB_I32: return AMDGPU::V_SUB_I32_e32;
  case AMDGPU::S_SUBB_U32: return AMDGPU::V_SUBB_U32_e32;
  case AMDGPU::S_AND_B32: return AMDGPU::V_AND_B32_e32;
  case AMDGPU::S_OR_B32: return AMDGPU::V_OR_B32_e32;
  case AMDGPU::S_XOR_B32: return AMDGPU::V_XOR_B32_e32;
  case AMDGPU::S_MIN_I32: return AMDGPU::V_MIN_I32_e32;
  case AMDGPU::S_MIN_U32: return AMDGPU::V_MIN_U32_e32;
  case AMDGPU::S_MAX_I32: return AMDGPU::V_MAX_I32_e32;
  case AMDGPU::S_MAX_U32: return AMDGPU::V_MAX_U32_e32;
  case AMDGPU::S_ASHR_I32: return AMDGPU::V_ASHR_I32_e32;
  case AMDGPU::S_ASHR_I64: return AMDGPU::V_ASHR_I64;
  case AMDGPU::S_LSHL_B32: return AMDGPU::V_LSHL_B32_e32;
  case AMDGPU::S_LSHL_B64: return AMDGPU::V_LSHL_B64;
  case AMDGPU::S_LSHR_B32: return AMDGPU::V_LSHR_B32_e32;
  case AMDGPU::S_LSHR_B64: return AMDGPU::V_LSHR_B64;
  case AMDGPU::S_SEXT_I32_I8: return AMDGPU::V_BFE_I32;
  case AMDGPU::S_SEXT_I32_I16: return AMDGPU::V_BFE_I32;
  case AMDGPU::S_BFE_U32: return AMDGPU::V_BFE_U32;
  case AMDGPU::S_BFE_I32: return AMDGPU::V_BFE_I32;
  case AMDGPU::S_BREV_B32: return AMDGPU::V_BFREV_B32_e32;
  case AMDGPU::S_NOT_B32: return AMDGPU::V_NOT_B32_e32;
  case AMDGPU::S_NOT_B64: return AMDGPU::V_NOT_B32_e32;
  case AMDGPU::S_CMP_EQ_I32: return AMDGPU::V_CMP_EQ_I32_e32;
  case AMDGPU::S_CMP_LG_I32: return AMDGPU::V_CMP_NE_I32_e32;
  case AMDGPU::S_CMP_GT_I32: return AMDGPU::V_CMP_GT_I32_e32;
  case AMDGPU::S_CMP_GE_I32: return AMDGPU::V_CMP_GE_I32_e32;
  case AMDGPU::S_CMP_LT_I32: return AMDGPU::V_CMP_LT_I32_e32;
  case AMDGPU::S_CMP_LE_I32: return AMDGPU::V_CMP_LE_I32_e32;
  case AMDGPU::S_LOAD_DWORD_IMM:
  case AMDGPU::S_LOAD_DWORD_SGPR: return AMDGPU::BUFFER_LOAD_DWORD_ADDR64;
  case AMDGPU::S_LOAD_DWORDX2_IMM:
  case AMDGPU::S_LOAD_DWORDX2_SGPR: return AMDGPU::BUFFER_LOAD_DWORDX2_ADDR64;
  case AMDGPU::S_LOAD_DWORDX4_IMM:
  case AMDGPU::S_LOAD_DWORDX4_SGPR: return AMDGPU::BUFFER_LOAD_DWORDX4_ADDR64;
  case AMDGPU::S_BCNT1_I32_B32: return AMDGPU::V_BCNT_U32_B32_e32;
  case AMDGPU::S_FF1_I32_B32: return AMDGPU::V_FFBL_B32_e32;
  case AMDGPU::S_FLBIT_I32_B32: return AMDGPU::V_FFBH_U32_e32;
  }
}

bool SIInstrInfo::isSALUOpSupportedOnVALU(const MachineInstr &MI) const {
  return getVALUOp(MI) != AMDGPU::INSTRUCTION_LIST_END;
}

const TargetRegisterClass *SIInstrInfo::getOpRegClass(const MachineInstr &MI,
                                                      unsigned OpNo) const {
  const MachineRegisterInfo &MRI = MI.getParent()->getParent()->getRegInfo();
  const MCInstrDesc &Desc = get(MI.getOpcode());
  if (MI.isVariadic() || OpNo >= Desc.getNumOperands() ||
      Desc.OpInfo[OpNo].RegClass == -1)
    return MRI.getRegClass(MI.getOperand(OpNo).getReg());

  unsigned RCID = Desc.OpInfo[OpNo].RegClass;
  return RI.getRegClass(RCID);
}

bool SIInstrInfo::canReadVGPR(const MachineInstr &MI, unsigned OpNo) const {
  switch (MI.getOpcode()) {
  case AMDGPU::COPY:
  case AMDGPU::REG_SEQUENCE:
  case AMDGPU::PHI:
  case AMDGPU::INSERT_SUBREG:
    return RI.hasVGPRs(getOpRegClass(MI, 0));
  default:
    return RI.hasVGPRs(getOpRegClass(MI, OpNo));
  }
}

void SIInstrInfo::legalizeOpWithMove(MachineInstr *MI, unsigned OpIdx) const {
  MachineBasicBlock::iterator I = MI;
  MachineOperand &MO = MI->getOperand(OpIdx);
  MachineRegisterInfo &MRI = MI->getParent()->getParent()->getRegInfo();
  unsigned RCID = get(MI->getOpcode()).OpInfo[OpIdx].RegClass;
  const TargetRegisterClass *RC = RI.getRegClass(RCID);
  unsigned Opcode = AMDGPU::V_MOV_B32_e32;
  if (MO.isReg()) {
    Opcode = AMDGPU::COPY;
  } else if (RI.isSGPRClass(RC)) {
    Opcode = AMDGPU::S_MOV_B32;
  }

  const TargetRegisterClass *VRC = RI.getEquivalentVGPRClass(RC);
  unsigned Reg = MRI.createVirtualRegister(VRC);
  BuildMI(*MI->getParent(), I, MI->getParent()->findDebugLoc(I), get(Opcode),
          Reg).addOperand(MO);
  MO.ChangeToRegister(Reg, false);
}

unsigned SIInstrInfo::buildExtractSubReg(MachineBasicBlock::iterator MI,
                                         MachineRegisterInfo &MRI,
                                         MachineOperand &SuperReg,
                                         const TargetRegisterClass *SuperRC,
                                         unsigned SubIdx,
                                         const TargetRegisterClass *SubRC)
                                         const {
  assert(SuperReg.isReg());

  unsigned NewSuperReg = MRI.createVirtualRegister(SuperRC);
  unsigned SubReg = MRI.createVirtualRegister(SubRC);

  // Just in case the super register is itself a sub-register, copy it to a new
  // value so we don't need to worry about merging its subreg index with the
  // SubIdx passed to this function. The register coalescer should be able to
  // eliminate this extra copy.
  BuildMI(*MI->getParent(), MI, MI->getDebugLoc(), get(TargetOpcode::COPY),
          NewSuperReg)
          .addOperand(SuperReg);

  BuildMI(*MI->getParent(), MI, MI->getDebugLoc(), get(TargetOpcode::COPY),
          SubReg)
          .addReg(NewSuperReg, 0, SubIdx);
  return SubReg;
}

MachineOperand SIInstrInfo::buildExtractSubRegOrImm(
  MachineBasicBlock::iterator MII,
  MachineRegisterInfo &MRI,
  MachineOperand &Op,
  const TargetRegisterClass *SuperRC,
  unsigned SubIdx,
  const TargetRegisterClass *SubRC) const {
  if (Op.isImm()) {
    // XXX - Is there a better way to do this?
    if (SubIdx == AMDGPU::sub0)
      return MachineOperand::CreateImm(Op.getImm() & 0xFFFFFFFF);
    if (SubIdx == AMDGPU::sub1)
      return MachineOperand::CreateImm(Op.getImm() >> 32);

    llvm_unreachable("Unhandled register index for immediate");
  }

  unsigned SubReg = buildExtractSubReg(MII, MRI, Op, SuperRC,
                                       SubIdx, SubRC);
  return MachineOperand::CreateReg(SubReg, false);
}

unsigned SIInstrInfo::split64BitImm(SmallVectorImpl<MachineInstr *> &Worklist,
                                    MachineBasicBlock::iterator MI,
                                    MachineRegisterInfo &MRI,
                                    const TargetRegisterClass *RC,
                                    const MachineOperand &Op) const {
  MachineBasicBlock *MBB = MI->getParent();
  DebugLoc DL = MI->getDebugLoc();
  unsigned LoDst = MRI.createVirtualRegister(&AMDGPU::SGPR_32RegClass);
  unsigned HiDst = MRI.createVirtualRegister(&AMDGPU::SGPR_32RegClass);
  unsigned Dst = MRI.createVirtualRegister(RC);

  MachineInstr *Lo = BuildMI(*MBB, MI, DL, get(AMDGPU::S_MOV_B32),
                             LoDst)
    .addImm(Op.getImm() & 0xFFFFFFFF);
  MachineInstr *Hi = BuildMI(*MBB, MI, DL, get(AMDGPU::S_MOV_B32),
                             HiDst)
    .addImm(Op.getImm() >> 32);

  BuildMI(*MBB, MI, DL, get(TargetOpcode::REG_SEQUENCE), Dst)
    .addReg(LoDst)
    .addImm(AMDGPU::sub0)
    .addReg(HiDst)
    .addImm(AMDGPU::sub1);

  Worklist.push_back(Lo);
  Worklist.push_back(Hi);

  return Dst;
}

bool SIInstrInfo::isOperandLegal(const MachineInstr *MI, unsigned OpIdx,
                                 const MachineOperand *MO) const {
  const MachineRegisterInfo &MRI = MI->getParent()->getParent()->getRegInfo();
  const MCInstrDesc &InstDesc = get(MI->getOpcode());
  const MCOperandInfo &OpInfo = InstDesc.OpInfo[OpIdx];
  const TargetRegisterClass *DefinedRC =
      OpInfo.RegClass != -1 ? RI.getRegClass(OpInfo.RegClass) : nullptr;
  if (!MO)
    MO = &MI->getOperand(OpIdx);

  if (MO->isReg()) {
    assert(DefinedRC);
    const TargetRegisterClass *RC = MRI.getRegClass(MO->getReg());
    return RI.getCommonSubClass(RC, RI.getRegClass(OpInfo.RegClass));
  }


  // Handle non-register types that are treated like immediates.
  assert(MO->isImm() || MO->isFPImm() || MO->isTargetIndex() || MO->isFI());

  if (!DefinedRC)
    // This opperand expects an immediate
    return true;

  return RI.regClassCanUseImmediate(DefinedRC);
}

void SIInstrInfo::legalizeOperands(MachineInstr *MI) const {
  MachineRegisterInfo &MRI = MI->getParent()->getParent()->getRegInfo();

  int Src0Idx = AMDGPU::getNamedOperandIdx(MI->getOpcode(),
                                           AMDGPU::OpName::src0);
  int Src1Idx = AMDGPU::getNamedOperandIdx(MI->getOpcode(),
                                           AMDGPU::OpName::src1);
  int Src2Idx = AMDGPU::getNamedOperandIdx(MI->getOpcode(),
                                           AMDGPU::OpName::src2);

  // Legalize VOP2
  if (isVOP2(MI->getOpcode()) && Src1Idx != -1) {
    // Legalize src0
    if (!isOperandLegal(MI, Src0Idx))
      legalizeOpWithMove(MI, Src0Idx);

    // Legalize src1
    if (isOperandLegal(MI, Src1Idx))
      return;

    // Usually src0 of VOP2 instructions allow more types of inputs
    // than src1, so try to commute the instruction to decrease our
    // chances of having to insert a MOV instruction to legalize src1.
    if (MI->isCommutable()) {
      if (commuteInstruction(MI))
        // If we are successful in commuting, then we know MI is legal, so
        // we are done.
        return;
    }

    legalizeOpWithMove(MI, Src1Idx);
    return;
  }

  // XXX - Do any VOP3 instructions read VCC?
  // Legalize VOP3
  if (isVOP3(MI->getOpcode())) {
    int VOP3Idx[3] = {Src0Idx, Src1Idx, Src2Idx};
    unsigned SGPRReg = AMDGPU::NoRegister;
    for (unsigned i = 0; i < 3; ++i) {
      int Idx = VOP3Idx[i];
      if (Idx == -1)
        continue;
      MachineOperand &MO = MI->getOperand(Idx);

      if (MO.isReg()) {
        if (!RI.isSGPRClass(MRI.getRegClass(MO.getReg())))
          continue; // VGPRs are legal

        assert(MO.getReg() != AMDGPU::SCC && "SCC operand to VOP3 instruction");

        if (SGPRReg == AMDGPU::NoRegister || SGPRReg == MO.getReg()) {
          SGPRReg = MO.getReg();
          // We can use one SGPR in each VOP3 instruction.
          continue;
        }
      } else if (!isLiteralConstant(MO)) {
        // If it is not a register and not a literal constant, then it must be
        // an inline constant which is always legal.
        continue;
      }
      // If we make it this far, then the operand is not legal and we must
      // legalize it.
      legalizeOpWithMove(MI, Idx);
    }
  }

  // Legalize REG_SEQUENCE and PHI
  // The register class of the operands much be the same type as the register
  // class of the output.
  if (MI->getOpcode() == AMDGPU::REG_SEQUENCE ||
      MI->getOpcode() == AMDGPU::PHI) {
    const TargetRegisterClass *RC = nullptr, *SRC = nullptr, *VRC = nullptr;
    for (unsigned i = 1, e = MI->getNumOperands(); i != e; i+=2) {
      if (!MI->getOperand(i).isReg() ||
          !TargetRegisterInfo::isVirtualRegister(MI->getOperand(i).getReg()))
        continue;
      const TargetRegisterClass *OpRC =
              MRI.getRegClass(MI->getOperand(i).getReg());
      if (RI.hasVGPRs(OpRC)) {
        VRC = OpRC;
      } else {
        SRC = OpRC;
      }
    }

    // If any of the operands are VGPR registers, then they all most be
    // otherwise we will create illegal VGPR->SGPR copies when legalizing
    // them.
    if (VRC || !RI.isSGPRClass(getOpRegClass(*MI, 0))) {
      if (!VRC) {
        assert(SRC);
        VRC = RI.getEquivalentVGPRClass(SRC);
      }
      RC = VRC;
    } else {
      RC = SRC;
    }

    // Update all the operands so they have the same type.
    for (unsigned i = 1, e = MI->getNumOperands(); i != e; i+=2) {
      if (!MI->getOperand(i).isReg() ||
          !TargetRegisterInfo::isVirtualRegister(MI->getOperand(i).getReg()))
        continue;
      unsigned DstReg = MRI.createVirtualRegister(RC);
      MachineBasicBlock *InsertBB;
      MachineBasicBlock::iterator Insert;
      if (MI->getOpcode() == AMDGPU::REG_SEQUENCE) {
        InsertBB = MI->getParent();
        Insert = MI;
      } else {
        // MI is a PHI instruction.
        InsertBB = MI->getOperand(i + 1).getMBB();
        Insert = InsertBB->getFirstTerminator();
      }
      BuildMI(*InsertBB, Insert, MI->getDebugLoc(),
              get(AMDGPU::COPY), DstReg)
              .addOperand(MI->getOperand(i));
      MI->getOperand(i).setReg(DstReg);
    }
  }

  // Legalize INSERT_SUBREG
  // src0 must have the same register class as dst
  if (MI->getOpcode() == AMDGPU::INSERT_SUBREG) {
    unsigned Dst = MI->getOperand(0).getReg();
    unsigned Src0 = MI->getOperand(1).getReg();
    const TargetRegisterClass *DstRC = MRI.getRegClass(Dst);
    const TargetRegisterClass *Src0RC = MRI.getRegClass(Src0);
    if (DstRC != Src0RC) {
      MachineBasicBlock &MBB = *MI->getParent();
      unsigned NewSrc0 = MRI.createVirtualRegister(DstRC);
      BuildMI(MBB, MI, MI->getDebugLoc(), get(AMDGPU::COPY), NewSrc0)
              .addReg(Src0);
      MI->getOperand(1).setReg(NewSrc0);
    }
    return;
  }

  // Legalize MUBUF* instructions
  // FIXME: If we start using the non-addr64 instructions for compute, we
  // may need to legalize them here.
  int SRsrcIdx =
      AMDGPU::getNamedOperandIdx(MI->getOpcode(), AMDGPU::OpName::srsrc);
  if (SRsrcIdx != -1) {
    // We have an MUBUF instruction
    MachineOperand *SRsrc = &MI->getOperand(SRsrcIdx);
    unsigned SRsrcRC = get(MI->getOpcode()).OpInfo[SRsrcIdx].RegClass;
    if (RI.getCommonSubClass(MRI.getRegClass(SRsrc->getReg()),
                                             RI.getRegClass(SRsrcRC))) {
      // The operands are legal.
      // FIXME: We may need to legalize operands besided srsrc.
      return;
    }

    MachineBasicBlock &MBB = *MI->getParent();
    // Extract the the ptr from the resource descriptor.

    // SRsrcPtrLo = srsrc:sub0
    unsigned SRsrcPtrLo = buildExtractSubReg(MI, MRI, *SRsrc,
        &AMDGPU::VReg_128RegClass, AMDGPU::sub0, &AMDGPU::VReg_32RegClass);

    // SRsrcPtrHi = srsrc:sub1
    unsigned SRsrcPtrHi = buildExtractSubReg(MI, MRI, *SRsrc,
        &AMDGPU::VReg_128RegClass, AMDGPU::sub1, &AMDGPU::VReg_32RegClass);

    // Create an empty resource descriptor
    unsigned Zero64 = MRI.createVirtualRegister(&AMDGPU::SReg_64RegClass);
    unsigned SRsrcFormatLo = MRI.createVirtualRegister(&AMDGPU::SGPR_32RegClass);
    unsigned SRsrcFormatHi = MRI.createVirtualRegister(&AMDGPU::SGPR_32RegClass);
    unsigned NewSRsrc = MRI.createVirtualRegister(&AMDGPU::SReg_128RegClass);

    // Zero64 = 0
    BuildMI(MBB, MI, MI->getDebugLoc(), get(AMDGPU::S_MOV_B64),
            Zero64)
            .addImm(0);

    // SRsrcFormatLo = RSRC_DATA_FORMAT{31-0}
    BuildMI(MBB, MI, MI->getDebugLoc(), get(AMDGPU::S_MOV_B32),
            SRsrcFormatLo)
            .addImm(AMDGPU::RSRC_DATA_FORMAT & 0xFFFFFFFF);

    // SRsrcFormatHi = RSRC_DATA_FORMAT{63-32}
    BuildMI(MBB, MI, MI->getDebugLoc(), get(AMDGPU::S_MOV_B32),
            SRsrcFormatHi)
            .addImm(AMDGPU::RSRC_DATA_FORMAT >> 32);

    // NewSRsrc = {Zero64, SRsrcFormat}
    BuildMI(MBB, MI, MI->getDebugLoc(), get(AMDGPU::REG_SEQUENCE),
            NewSRsrc)
            .addReg(Zero64)
            .addImm(AMDGPU::sub0_sub1)
            .addReg(SRsrcFormatLo)
            .addImm(AMDGPU::sub2)
            .addReg(SRsrcFormatHi)
            .addImm(AMDGPU::sub3);

    MachineOperand *VAddr = getNamedOperand(*MI, AMDGPU::OpName::vaddr);
    unsigned NewVAddr = MRI.createVirtualRegister(&AMDGPU::VReg_64RegClass);
    unsigned NewVAddrLo;
    unsigned NewVAddrHi;
    if (VAddr) {
      // This is already an ADDR64 instruction so we need to add the pointer
      // extracted from the resource descriptor to the current value of VAddr.
      NewVAddrLo = MRI.createVirtualRegister(&AMDGPU::VReg_32RegClass);
      NewVAddrHi = MRI.createVirtualRegister(&AMDGPU::VReg_32RegClass);

      // NewVaddrLo = SRsrcPtrLo + VAddr:sub0
      BuildMI(MBB, MI, MI->getDebugLoc(), get(AMDGPU::V_ADD_I32_e32),
              NewVAddrLo)
              .addReg(SRsrcPtrLo)
              .addReg(VAddr->getReg(), 0, AMDGPU::sub0)
              .addReg(AMDGPU::VCC, RegState::ImplicitDefine);

      // NewVaddrHi = SRsrcPtrHi + VAddr:sub1
      BuildMI(MBB, MI, MI->getDebugLoc(), get(AMDGPU::V_ADDC_U32_e32),
              NewVAddrHi)
              .addReg(SRsrcPtrHi)
              .addReg(VAddr->getReg(), 0, AMDGPU::sub1)
              .addReg(AMDGPU::VCC, RegState::ImplicitDefine)
              .addReg(AMDGPU::VCC, RegState::Implicit);

    } else {
      // This instructions is the _OFFSET variant, so we need to convert it to
      // ADDR64.
      MachineOperand *VData = getNamedOperand(*MI, AMDGPU::OpName::vdata);
      MachineOperand *Offset = getNamedOperand(*MI, AMDGPU::OpName::offset);
      MachineOperand *SOffset = getNamedOperand(*MI, AMDGPU::OpName::soffset);
      assert(SOffset->isImm() && SOffset->getImm() == 0 && "Legalizing MUBUF "
             "with non-zero soffset is not implemented");
      (void)SOffset;

      // Create the new instruction.
      unsigned Addr64Opcode = AMDGPU::getAddr64Inst(MI->getOpcode());
      MachineInstr *Addr64 =
          BuildMI(MBB, MI, MI->getDebugLoc(), get(Addr64Opcode))
                  .addOperand(*VData)
                  .addOperand(*SRsrc)
                  .addReg(AMDGPU::NoRegister) // Dummy value for vaddr.
                                              // This will be replaced later
                                              // with the new value of vaddr.
                  .addOperand(*Offset);

      MI->removeFromParent();
      MI = Addr64;

      NewVAddrLo = SRsrcPtrLo;
      NewVAddrHi = SRsrcPtrHi;
      VAddr = getNamedOperand(*MI, AMDGPU::OpName::vaddr);
      SRsrc = getNamedOperand(*MI, AMDGPU::OpName::srsrc);
    }

    // NewVaddr = {NewVaddrHi, NewVaddrLo}
    BuildMI(MBB, MI, MI->getDebugLoc(), get(AMDGPU::REG_SEQUENCE),
            NewVAddr)
            .addReg(NewVAddrLo)
            .addImm(AMDGPU::sub0)
            .addReg(NewVAddrHi)
            .addImm(AMDGPU::sub1);


    // Update the instruction to use NewVaddr
    VAddr->setReg(NewVAddr);
    // Update the instruction to use NewSRsrc
    SRsrc->setReg(NewSRsrc);
  }
}

void SIInstrInfo::splitSMRD(MachineInstr *MI,
                            const TargetRegisterClass *HalfRC,
                            unsigned HalfImmOp, unsigned HalfSGPROp,
                            MachineInstr *&Lo, MachineInstr *&Hi) const {

  DebugLoc DL = MI->getDebugLoc();
  MachineBasicBlock *MBB = MI->getParent();
  MachineRegisterInfo &MRI = MBB->getParent()->getRegInfo();
  unsigned RegLo = MRI.createVirtualRegister(HalfRC);
  unsigned RegHi = MRI.createVirtualRegister(HalfRC);
  unsigned HalfSize = HalfRC->getSize();
  const MachineOperand *OffOp =
      getNamedOperand(*MI, AMDGPU::OpName::offset);
  const MachineOperand *SBase = getNamedOperand(*MI, AMDGPU::OpName::sbase);

  if (OffOp) {
    // Handle the _IMM variant
    unsigned LoOffset = OffOp->getImm();
    unsigned HiOffset = LoOffset + (HalfSize / 4);
    Lo = BuildMI(*MBB, MI, DL, get(HalfImmOp), RegLo)
                  .addOperand(*SBase)
                  .addImm(LoOffset);

    if (!isUInt<8>(HiOffset)) {
      unsigned OffsetSGPR =
          MRI.createVirtualRegister(&AMDGPU::SReg_32RegClass);
      BuildMI(*MBB, MI, DL, get(AMDGPU::S_MOV_B32), OffsetSGPR)
              .addImm(HiOffset << 2);  // The immediate offset is in dwords,
                                       // but offset in register is in bytes.
      Hi = BuildMI(*MBB, MI, DL, get(HalfSGPROp), RegHi)
                    .addOperand(*SBase)
                    .addReg(OffsetSGPR);
    } else {
      Hi = BuildMI(*MBB, MI, DL, get(HalfImmOp), RegHi)
                     .addOperand(*SBase)
                     .addImm(HiOffset);
    }
  } else {
    // Handle the _SGPR variant
    MachineOperand *SOff = getNamedOperand(*MI, AMDGPU::OpName::soff);
    Lo = BuildMI(*MBB, MI, DL, get(HalfSGPROp), RegLo)
                  .addOperand(*SBase)
                  .addOperand(*SOff);
    unsigned OffsetSGPR = MRI.createVirtualRegister(&AMDGPU::SReg_32RegClass);
    BuildMI(*MBB, MI, DL, get(AMDGPU::S_ADD_I32), OffsetSGPR)
            .addOperand(*SOff)
            .addImm(HalfSize);
    Hi = BuildMI(*MBB, MI, DL, get(HalfSGPROp))
                  .addOperand(*SBase)
                  .addReg(OffsetSGPR);
  }

  unsigned SubLo, SubHi;
  switch (HalfSize) {
    case 4:
      SubLo = AMDGPU::sub0;
      SubHi = AMDGPU::sub1;
      break;
    case 8:
      SubLo = AMDGPU::sub0_sub1;
      SubHi = AMDGPU::sub2_sub3;
      break;
    case 16:
      SubLo = AMDGPU::sub0_sub1_sub2_sub3;
      SubHi = AMDGPU::sub4_sub5_sub6_sub7;
      break;
    case 32:
      SubLo = AMDGPU::sub0_sub1_sub2_sub3_sub4_sub5_sub6_sub7;
      SubHi = AMDGPU::sub8_sub9_sub10_sub11_sub12_sub13_sub14_sub15;
      break;
    default:
      llvm_unreachable("Unhandled HalfSize");
  }

  BuildMI(*MBB, MI, DL, get(AMDGPU::REG_SEQUENCE))
          .addOperand(MI->getOperand(0))
          .addReg(RegLo)
          .addImm(SubLo)
          .addReg(RegHi)
          .addImm(SubHi);
}

void SIInstrInfo::moveSMRDToVALU(MachineInstr *MI, MachineRegisterInfo &MRI) const {
  MachineBasicBlock *MBB = MI->getParent();
  switch (MI->getOpcode()) {
    case AMDGPU::S_LOAD_DWORD_IMM:
    case AMDGPU::S_LOAD_DWORD_SGPR:
    case AMDGPU::S_LOAD_DWORDX2_IMM:
    case AMDGPU::S_LOAD_DWORDX2_SGPR:
    case AMDGPU::S_LOAD_DWORDX4_IMM:
    case AMDGPU::S_LOAD_DWORDX4_SGPR: {
      unsigned NewOpcode = getVALUOp(*MI);
      unsigned RegOffset;
      unsigned ImmOffset;

      if (MI->getOperand(2).isReg()) {
        RegOffset = MI->getOperand(2).getReg();
        ImmOffset = 0;
      } else {
        assert(MI->getOperand(2).isImm());
        // SMRD instructions take a dword offsets and MUBUF instructions
        // take a byte offset.
        ImmOffset = MI->getOperand(2).getImm() << 2;
        RegOffset = MRI.createVirtualRegister(&AMDGPU::SGPR_32RegClass);
        if (isUInt<12>(ImmOffset)) {
          BuildMI(*MBB, MI, MI->getDebugLoc(), get(AMDGPU::S_MOV_B32),
                  RegOffset)
                  .addImm(0);
        } else {
          BuildMI(*MBB, MI, MI->getDebugLoc(), get(AMDGPU::S_MOV_B32),
                  RegOffset)
                  .addImm(ImmOffset);
          ImmOffset = 0;
        }
      }

      unsigned SRsrc = MRI.createVirtualRegister(&AMDGPU::SReg_128RegClass);
      unsigned DWord0 = RegOffset;
      unsigned DWord1 = MRI.createVirtualRegister(&AMDGPU::SGPR_32RegClass);
      unsigned DWord2 = MRI.createVirtualRegister(&AMDGPU::SGPR_32RegClass);
      unsigned DWord3 = MRI.createVirtualRegister(&AMDGPU::SGPR_32RegClass);

      BuildMI(*MBB, MI, MI->getDebugLoc(), get(AMDGPU::S_MOV_B32), DWord1)
              .addImm(0);
      BuildMI(*MBB, MI, MI->getDebugLoc(), get(AMDGPU::S_MOV_B32), DWord2)
              .addImm(AMDGPU::RSRC_DATA_FORMAT & 0xFFFFFFFF);
      BuildMI(*MBB, MI, MI->getDebugLoc(), get(AMDGPU::S_MOV_B32), DWord3)
              .addImm(AMDGPU::RSRC_DATA_FORMAT >> 32);
      BuildMI(*MBB, MI, MI->getDebugLoc(), get(AMDGPU::REG_SEQUENCE), SRsrc)
              .addReg(DWord0)
              .addImm(AMDGPU::sub0)
              .addReg(DWord1)
              .addImm(AMDGPU::sub1)
              .addReg(DWord2)
              .addImm(AMDGPU::sub2)
              .addReg(DWord3)
              .addImm(AMDGPU::sub3);
      MI->setDesc(get(NewOpcode));
      if (MI->getOperand(2).isReg()) {
        MI->getOperand(2).setReg(MI->getOperand(1).getReg());
      } else {
        MI->getOperand(2).ChangeToRegister(MI->getOperand(1).getReg(), false);
      }
      MI->getOperand(1).setReg(SRsrc);
      MI->addOperand(*MBB->getParent(), MachineOperand::CreateImm(ImmOffset));

      const TargetRegisterClass *NewDstRC =
          RI.getRegClass(get(NewOpcode).OpInfo[0].RegClass);

      unsigned DstReg = MI->getOperand(0).getReg();
      unsigned NewDstReg = MRI.createVirtualRegister(NewDstRC);
      MRI.replaceRegWith(DstReg, NewDstReg);
      break;
    }
    case AMDGPU::S_LOAD_DWORDX8_IMM:
    case AMDGPU::S_LOAD_DWORDX8_SGPR: {
      MachineInstr *Lo, *Hi;
      splitSMRD(MI, &AMDGPU::SReg_128RegClass, AMDGPU::S_LOAD_DWORDX4_IMM,
                AMDGPU::S_LOAD_DWORDX4_SGPR, Lo, Hi);
      MI->eraseFromParent();
      moveSMRDToVALU(Lo, MRI);
      moveSMRDToVALU(Hi, MRI);
      break;
    }

    case AMDGPU::S_LOAD_DWORDX16_IMM:
    case AMDGPU::S_LOAD_DWORDX16_SGPR: {
      MachineInstr *Lo, *Hi;
      splitSMRD(MI, &AMDGPU::SReg_256RegClass, AMDGPU::S_LOAD_DWORDX8_IMM,
                AMDGPU::S_LOAD_DWORDX8_SGPR, Lo, Hi);
      MI->eraseFromParent();
      moveSMRDToVALU(Lo, MRI);
      moveSMRDToVALU(Hi, MRI);
      break;
    }
  }
}

void SIInstrInfo::moveToVALU(MachineInstr &TopInst) const {
  SmallVector<MachineInstr *, 128> Worklist;
  Worklist.push_back(&TopInst);

  while (!Worklist.empty()) {
    MachineInstr *Inst = Worklist.pop_back_val();
    MachineBasicBlock *MBB = Inst->getParent();
    MachineRegisterInfo &MRI = MBB->getParent()->getRegInfo();

    unsigned Opcode = Inst->getOpcode();
    unsigned NewOpcode = getVALUOp(*Inst);

    // Handle some special cases
    switch (Opcode) {
    default:
      if (isSMRD(Inst->getOpcode())) {
        moveSMRDToVALU(Inst, MRI);
      }
      break;
    case AMDGPU::S_MOV_B64: {
      DebugLoc DL = Inst->getDebugLoc();

      // If the source operand is a register we can replace this with a
      // copy.
      if (Inst->getOperand(1).isReg()) {
        MachineInstr *Copy = BuildMI(*MBB, Inst, DL, get(TargetOpcode::COPY))
          .addOperand(Inst->getOperand(0))
          .addOperand(Inst->getOperand(1));
        Worklist.push_back(Copy);
      } else {
        // Otherwise, we need to split this into two movs, because there is
        // no 64-bit VALU move instruction.
        unsigned Reg = Inst->getOperand(0).getReg();
        unsigned Dst = split64BitImm(Worklist,
                                     Inst,
                                     MRI,
                                     MRI.getRegClass(Reg),
                                     Inst->getOperand(1));
        MRI.replaceRegWith(Reg, Dst);
      }
      Inst->eraseFromParent();
      continue;
    }
    case AMDGPU::S_AND_B64:
      splitScalar64BitBinaryOp(Worklist, Inst, AMDGPU::S_AND_B32);
      Inst->eraseFromParent();
      continue;

    case AMDGPU::S_OR_B64:
      splitScalar64BitBinaryOp(Worklist, Inst, AMDGPU::S_OR_B32);
      Inst->eraseFromParent();
      continue;

    case AMDGPU::S_XOR_B64:
      splitScalar64BitBinaryOp(Worklist, Inst, AMDGPU::S_XOR_B32);
      Inst->eraseFromParent();
      continue;

    case AMDGPU::S_NOT_B64:
      splitScalar64BitUnaryOp(Worklist, Inst, AMDGPU::S_NOT_B32);
      Inst->eraseFromParent();
      continue;

    case AMDGPU::S_BCNT1_I32_B64:
      splitScalar64BitBCNT(Worklist, Inst);
      Inst->eraseFromParent();
      continue;

    case AMDGPU::S_BFE_U64:
    case AMDGPU::S_BFE_I64:
    case AMDGPU::S_BFM_B64:
      llvm_unreachable("Moving this op to VALU not implemented");
    }

    if (NewOpcode == AMDGPU::INSTRUCTION_LIST_END) {
      // We cannot move this instruction to the VALU, so we should try to
      // legalize its operands instead.
      legalizeOperands(Inst);
      continue;
    }

    // Use the new VALU Opcode.
    const MCInstrDesc &NewDesc = get(NewOpcode);
    Inst->setDesc(NewDesc);

    // Remove any references to SCC. Vector instructions can't read from it, and
    // We're just about to add the implicit use / defs of VCC, and we don't want
    // both.
    for (unsigned i = Inst->getNumOperands() - 1; i > 0; --i) {
      MachineOperand &Op = Inst->getOperand(i);
      if (Op.isReg() && Op.getReg() == AMDGPU::SCC)
        Inst->RemoveOperand(i);
    }

    if (Opcode == AMDGPU::S_SEXT_I32_I8 || Opcode == AMDGPU::S_SEXT_I32_I16) {
      // We are converting these to a BFE, so we need to add the missing
      // operands for the size and offset.
      unsigned Size = (Opcode == AMDGPU::S_SEXT_I32_I8) ? 8 : 16;
      Inst->addOperand(MachineOperand::CreateImm(0));
      Inst->addOperand(MachineOperand::CreateImm(Size));

    } else if (Opcode == AMDGPU::S_BCNT1_I32_B32) {
      // The VALU version adds the second operand to the result, so insert an
      // extra 0 operand.
      Inst->addOperand(MachineOperand::CreateImm(0));
    }

    addDescImplicitUseDef(NewDesc, Inst);

    if (Opcode == AMDGPU::S_BFE_I32 || Opcode == AMDGPU::S_BFE_U32) {
      const MachineOperand &OffsetWidthOp = Inst->getOperand(2);
      // If we need to move this to VGPRs, we need to unpack the second operand
      // back into the 2 separate ones for bit offset and width.
      assert(OffsetWidthOp.isImm() &&
             "Scalar BFE is only implemented for constant width and offset");
      uint32_t Imm = OffsetWidthOp.getImm();

      uint32_t Offset = Imm & 0x3f; // Extract bits [5:0].
      uint32_t BitWidth = (Imm & 0x7f0000) >> 16; // Extract bits [22:16].
      Inst->RemoveOperand(2); // Remove old immediate.
      Inst->addOperand(MachineOperand::CreateImm(Offset));
      Inst->addOperand(MachineOperand::CreateImm(BitWidth));
    }

    // Update the destination register class.

    const TargetRegisterClass *NewDstRC = getOpRegClass(*Inst, 0);

    switch (Opcode) {
      // For target instructions, getOpRegClass just returns the virtual
      // register class associated with the operand, so we need to find an
      // equivalent VGPR register class in order to move the instruction to the
      // VALU.
    case AMDGPU::COPY:
    case AMDGPU::PHI:
    case AMDGPU::REG_SEQUENCE:
    case AMDGPU::INSERT_SUBREG:
      if (RI.hasVGPRs(NewDstRC))
        continue;
      NewDstRC = RI.getEquivalentVGPRClass(NewDstRC);
      if (!NewDstRC)
        continue;
      break;
    default:
      break;
    }

    unsigned DstReg = Inst->getOperand(0).getReg();
    unsigned NewDstReg = MRI.createVirtualRegister(NewDstRC);
    MRI.replaceRegWith(DstReg, NewDstReg);

    // Legalize the operands
    legalizeOperands(Inst);

    for (MachineRegisterInfo::use_iterator I = MRI.use_begin(NewDstReg),
           E = MRI.use_end(); I != E; ++I) {
      MachineInstr &UseMI = *I->getParent();
      if (!canReadVGPR(UseMI, I.getOperandNo())) {
        Worklist.push_back(&UseMI);
      }
    }
  }
}

//===----------------------------------------------------------------------===//
// Indirect addressing callbacks
//===----------------------------------------------------------------------===//

unsigned SIInstrInfo::calculateIndirectAddress(unsigned RegIndex,
                                                 unsigned Channel) const {
  assert(Channel == 0);
  return RegIndex;
}

const TargetRegisterClass *SIInstrInfo::getIndirectAddrRegClass() const {
  return &AMDGPU::VReg_32RegClass;
}

void SIInstrInfo::splitScalar64BitUnaryOp(
  SmallVectorImpl<MachineInstr *> &Worklist,
  MachineInstr *Inst,
  unsigned Opcode) const {
  MachineBasicBlock &MBB = *Inst->getParent();
  MachineRegisterInfo &MRI = MBB.getParent()->getRegInfo();

  MachineOperand &Dest = Inst->getOperand(0);
  MachineOperand &Src0 = Inst->getOperand(1);
  DebugLoc DL = Inst->getDebugLoc();

  MachineBasicBlock::iterator MII = Inst;

  const MCInstrDesc &InstDesc = get(Opcode);
  const TargetRegisterClass *Src0RC = Src0.isReg() ?
    MRI.getRegClass(Src0.getReg()) :
    &AMDGPU::SGPR_32RegClass;

  const TargetRegisterClass *Src0SubRC = RI.getSubRegClass(Src0RC, AMDGPU::sub0);

  MachineOperand SrcReg0Sub0 = buildExtractSubRegOrImm(MII, MRI, Src0, Src0RC,
                                                       AMDGPU::sub0, Src0SubRC);

  const TargetRegisterClass *DestRC = MRI.getRegClass(Dest.getReg());
  const TargetRegisterClass *DestSubRC = RI.getSubRegClass(DestRC, AMDGPU::sub0);

  unsigned DestSub0 = MRI.createVirtualRegister(DestRC);
  MachineInstr *LoHalf = BuildMI(MBB, MII, DL, InstDesc, DestSub0)
    .addOperand(SrcReg0Sub0);

  MachineOperand SrcReg0Sub1 = buildExtractSubRegOrImm(MII, MRI, Src0, Src0RC,
                                                       AMDGPU::sub1, Src0SubRC);

  unsigned DestSub1 = MRI.createVirtualRegister(DestSubRC);
  MachineInstr *HiHalf = BuildMI(MBB, MII, DL, InstDesc, DestSub1)
    .addOperand(SrcReg0Sub1);

  unsigned FullDestReg = MRI.createVirtualRegister(DestRC);
  BuildMI(MBB, MII, DL, get(TargetOpcode::REG_SEQUENCE), FullDestReg)
    .addReg(DestSub0)
    .addImm(AMDGPU::sub0)
    .addReg(DestSub1)
    .addImm(AMDGPU::sub1);

  MRI.replaceRegWith(Dest.getReg(), FullDestReg);

  // Try to legalize the operands in case we need to swap the order to keep it
  // valid.
  Worklist.push_back(LoHalf);
  Worklist.push_back(HiHalf);
}

void SIInstrInfo::splitScalar64BitBinaryOp(
  SmallVectorImpl<MachineInstr *> &Worklist,
  MachineInstr *Inst,
  unsigned Opcode) const {
  MachineBasicBlock &MBB = *Inst->getParent();
  MachineRegisterInfo &MRI = MBB.getParent()->getRegInfo();

  MachineOperand &Dest = Inst->getOperand(0);
  MachineOperand &Src0 = Inst->getOperand(1);
  MachineOperand &Src1 = Inst->getOperand(2);
  DebugLoc DL = Inst->getDebugLoc();

  MachineBasicBlock::iterator MII = Inst;

  const MCInstrDesc &InstDesc = get(Opcode);
  const TargetRegisterClass *Src0RC = Src0.isReg() ?
    MRI.getRegClass(Src0.getReg()) :
    &AMDGPU::SGPR_32RegClass;

  const TargetRegisterClass *Src0SubRC = RI.getSubRegClass(Src0RC, AMDGPU::sub0);
  const TargetRegisterClass *Src1RC = Src1.isReg() ?
    MRI.getRegClass(Src1.getReg()) :
    &AMDGPU::SGPR_32RegClass;

  const TargetRegisterClass *Src1SubRC = RI.getSubRegClass(Src1RC, AMDGPU::sub0);

  MachineOperand SrcReg0Sub0 = buildExtractSubRegOrImm(MII, MRI, Src0, Src0RC,
                                                       AMDGPU::sub0, Src0SubRC);
  MachineOperand SrcReg1Sub0 = buildExtractSubRegOrImm(MII, MRI, Src1, Src1RC,
                                                       AMDGPU::sub0, Src1SubRC);

  const TargetRegisterClass *DestRC = MRI.getRegClass(Dest.getReg());
  const TargetRegisterClass *DestSubRC = RI.getSubRegClass(DestRC, AMDGPU::sub0);

  unsigned DestSub0 = MRI.createVirtualRegister(DestRC);
  MachineInstr *LoHalf = BuildMI(MBB, MII, DL, InstDesc, DestSub0)
    .addOperand(SrcReg0Sub0)
    .addOperand(SrcReg1Sub0);

  MachineOperand SrcReg0Sub1 = buildExtractSubRegOrImm(MII, MRI, Src0, Src0RC,
                                                       AMDGPU::sub1, Src0SubRC);
  MachineOperand SrcReg1Sub1 = buildExtractSubRegOrImm(MII, MRI, Src1, Src1RC,
                                                       AMDGPU::sub1, Src1SubRC);

  unsigned DestSub1 = MRI.createVirtualRegister(DestSubRC);
  MachineInstr *HiHalf = BuildMI(MBB, MII, DL, InstDesc, DestSub1)
    .addOperand(SrcReg0Sub1)
    .addOperand(SrcReg1Sub1);

  unsigned FullDestReg = MRI.createVirtualRegister(DestRC);
  BuildMI(MBB, MII, DL, get(TargetOpcode::REG_SEQUENCE), FullDestReg)
    .addReg(DestSub0)
    .addImm(AMDGPU::sub0)
    .addReg(DestSub1)
    .addImm(AMDGPU::sub1);

  MRI.replaceRegWith(Dest.getReg(), FullDestReg);

  // Try to legalize the operands in case we need to swap the order to keep it
  // valid.
  Worklist.push_back(LoHalf);
  Worklist.push_back(HiHalf);
}

void SIInstrInfo::splitScalar64BitBCNT(SmallVectorImpl<MachineInstr *> &Worklist,
                                       MachineInstr *Inst) const {
  MachineBasicBlock &MBB = *Inst->getParent();
  MachineRegisterInfo &MRI = MBB.getParent()->getRegInfo();

  MachineBasicBlock::iterator MII = Inst;
  DebugLoc DL = Inst->getDebugLoc();

  MachineOperand &Dest = Inst->getOperand(0);
  MachineOperand &Src = Inst->getOperand(1);

  const MCInstrDesc &InstDesc = get(AMDGPU::V_BCNT_U32_B32_e32);
  const TargetRegisterClass *SrcRC = Src.isReg() ?
    MRI.getRegClass(Src.getReg()) :
    &AMDGPU::SGPR_32RegClass;

  unsigned MidReg = MRI.createVirtualRegister(&AMDGPU::VGPR_32RegClass);
  unsigned ResultReg = MRI.createVirtualRegister(&AMDGPU::VGPR_32RegClass);

  const TargetRegisterClass *SrcSubRC = RI.getSubRegClass(SrcRC, AMDGPU::sub0);

  MachineOperand SrcRegSub0 = buildExtractSubRegOrImm(MII, MRI, Src, SrcRC,
                                                      AMDGPU::sub0, SrcSubRC);
  MachineOperand SrcRegSub1 = buildExtractSubRegOrImm(MII, MRI, Src, SrcRC,
                                                      AMDGPU::sub1, SrcSubRC);

  MachineInstr *First = BuildMI(MBB, MII, DL, InstDesc, MidReg)
    .addOperand(SrcRegSub0)
    .addImm(0);

  MachineInstr *Second = BuildMI(MBB, MII, DL, InstDesc, ResultReg)
    .addOperand(SrcRegSub1)
    .addReg(MidReg);

  MRI.replaceRegWith(Dest.getReg(), ResultReg);

  Worklist.push_back(First);
  Worklist.push_back(Second);
}

void SIInstrInfo::addDescImplicitUseDef(const MCInstrDesc &NewDesc,
                                        MachineInstr *Inst) const {
  // Add the implict and explicit register definitions.
  if (NewDesc.ImplicitUses) {
    for (unsigned i = 0; NewDesc.ImplicitUses[i]; ++i) {
      unsigned Reg = NewDesc.ImplicitUses[i];
      Inst->addOperand(MachineOperand::CreateReg(Reg, false, true));
    }
  }

  if (NewDesc.ImplicitDefs) {
    for (unsigned i = 0; NewDesc.ImplicitDefs[i]; ++i) {
      unsigned Reg = NewDesc.ImplicitDefs[i];
      Inst->addOperand(MachineOperand::CreateReg(Reg, true, true));
    }
  }
}

MachineInstrBuilder SIInstrInfo::buildIndirectWrite(
                                   MachineBasicBlock *MBB,
                                   MachineBasicBlock::iterator I,
                                   unsigned ValueReg,
                                   unsigned Address, unsigned OffsetReg) const {
  const DebugLoc &DL = MBB->findDebugLoc(I);
  unsigned IndirectBaseReg = AMDGPU::VReg_32RegClass.getRegister(
                                      getIndirectIndexBegin(*MBB->getParent()));

  return BuildMI(*MBB, I, DL, get(AMDGPU::SI_INDIRECT_DST_V1))
          .addReg(IndirectBaseReg, RegState::Define)
          .addOperand(I->getOperand(0))
          .addReg(IndirectBaseReg)
          .addReg(OffsetReg)
          .addImm(0)
          .addReg(ValueReg);
}

MachineInstrBuilder SIInstrInfo::buildIndirectRead(
                                   MachineBasicBlock *MBB,
                                   MachineBasicBlock::iterator I,
                                   unsigned ValueReg,
                                   unsigned Address, unsigned OffsetReg) const {
  const DebugLoc &DL = MBB->findDebugLoc(I);
  unsigned IndirectBaseReg = AMDGPU::VReg_32RegClass.getRegister(
                                      getIndirectIndexBegin(*MBB->getParent()));

  return BuildMI(*MBB, I, DL, get(AMDGPU::SI_INDIRECT_SRC))
          .addOperand(I->getOperand(0))
          .addOperand(I->getOperand(1))
          .addReg(IndirectBaseReg)
          .addReg(OffsetReg)
          .addImm(0);

}

void SIInstrInfo::reserveIndirectRegisters(BitVector &Reserved,
                                            const MachineFunction &MF) const {
  int End = getIndirectIndexEnd(MF);
  int Begin = getIndirectIndexBegin(MF);

  if (End == -1)
    return;


  for (int Index = Begin; Index <= End; ++Index)
    Reserved.set(AMDGPU::VReg_32RegClass.getRegister(Index));

  for (int Index = std::max(0, Begin - 1); Index <= End; ++Index)
    Reserved.set(AMDGPU::VReg_64RegClass.getRegister(Index));

  for (int Index = std::max(0, Begin - 2); Index <= End; ++Index)
    Reserved.set(AMDGPU::VReg_96RegClass.getRegister(Index));

  for (int Index = std::max(0, Begin - 3); Index <= End; ++Index)
    Reserved.set(AMDGPU::VReg_128RegClass.getRegister(Index));

  for (int Index = std::max(0, Begin - 7); Index <= End; ++Index)
    Reserved.set(AMDGPU::VReg_256RegClass.getRegister(Index));

  for (int Index = std::max(0, Begin - 15); Index <= End; ++Index)
    Reserved.set(AMDGPU::VReg_512RegClass.getRegister(Index));
}

MachineOperand *SIInstrInfo::getNamedOperand(MachineInstr &MI,
                                                   unsigned OperandName) const {
  int Idx = AMDGPU::getNamedOperandIdx(MI.getOpcode(), OperandName);
  if (Idx == -1)
    return nullptr;

  return &MI.getOperand(Idx);
}
