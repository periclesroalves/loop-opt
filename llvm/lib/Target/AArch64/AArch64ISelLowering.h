//==-- AArch64ISelLowering.h - AArch64 DAG Lowering Interface ----*- C++ -*-==//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file defines the interfaces that AArch64 uses to lower LLVM code into a
// selection DAG.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_AARCH64_AARCH64ISELLOWERING_H
#define LLVM_LIB_TARGET_AARCH64_AARCH64ISELLOWERING_H

#include "llvm/CodeGen/CallingConvLower.h"
#include "llvm/CodeGen/SelectionDAG.h"
#include "llvm/IR/CallingConv.h"
#include "llvm/Target/TargetLowering.h"

namespace llvm {

namespace AArch64ISD {

enum {
  FIRST_NUMBER = ISD::BUILTIN_OP_END,
  WrapperLarge, // 4-instruction MOVZ/MOVK sequence for 64-bit addresses.
  CALL,         // Function call.

  // Almost the same as a normal call node, except that a TLSDesc relocation is
  // needed so the linker can relax it correctly if possible.
  TLSDESC_CALL,
  ADRP,     // Page address of a TargetGlobalAddress operand.
  ADDlow,   // Add the low 12 bits of a TargetGlobalAddress operand.
  LOADgot,  // Load from automatically generated descriptor (e.g. Global
            // Offset Table, TLS record).
  RET_FLAG, // Return with a flag operand. Operand 0 is the chain operand.
  BRCOND,   // Conditional branch instruction; "b.cond".
  CSEL,
  FCSEL, // Conditional move instruction.
  CSINV, // Conditional select invert.
  CSNEG, // Conditional select negate.
  CSINC, // Conditional select increment.

  // Pointer to the thread's local storage area. Materialised from TPIDR_EL0 on
  // ELF.
  THREAD_POINTER,
  ADC,
  SBC, // adc, sbc instructions

  // Arithmetic instructions which write flags.
  ADDS,
  SUBS,
  ADCS,
  SBCS,
  ANDS,

  // Floating point comparison
  FCMP,

  // Floating point max and min instructions.
  FMAX,
  FMIN,

  // Scalar extract
  EXTR,

  // Scalar-to-vector duplication
  DUP,
  DUPLANE8,
  DUPLANE16,
  DUPLANE32,
  DUPLANE64,

  // Vector immedate moves
  MOVI,
  MOVIshift,
  MOVIedit,
  MOVImsl,
  FMOV,
  MVNIshift,
  MVNImsl,

  // Vector immediate ops
  BICi,
  ORRi,

  // Vector bit select: similar to ISD::VSELECT but not all bits within an
  // element must be identical.
  BSL,

  // Vector arithmetic negation
  NEG,

  // Vector shuffles
  ZIP1,
  ZIP2,
  UZP1,
  UZP2,
  TRN1,
  TRN2,
  REV16,
  REV32,
  REV64,
  EXT,

  // Vector shift by scalar
  VSHL,
  VLSHR,
  VASHR,

  // Vector shift by scalar (again)
  SQSHL_I,
  UQSHL_I,
  SQSHLU_I,
  SRSHR_I,
  URSHR_I,

  // Vector comparisons
  CMEQ,
  CMGE,
  CMGT,
  CMHI,
  CMHS,
  FCMEQ,
  FCMGE,
  FCMGT,

  // Vector zero comparisons
  CMEQz,
  CMGEz,
  CMGTz,
  CMLEz,
  CMLTz,
  FCMEQz,
  FCMGEz,
  FCMGTz,
  FCMLEz,
  FCMLTz,

  // Vector bitwise negation
  NOT,

  // Vector bitwise selection
  BIT,

  // Compare-and-branch
  CBZ,
  CBNZ,
  TBZ,
  TBNZ,

  // Tail calls
  TC_RETURN,

  // Custom prefetch handling
  PREFETCH,

  // {s|u}int to FP within a FP register.
  SITOF,
  UITOF,

  // NEON Load/Store with post-increment base updates
  LD2post = ISD::FIRST_TARGET_MEMORY_OPCODE,
  LD3post,
  LD4post,
  ST2post,
  ST3post,
  ST4post,
  LD1x2post,
  LD1x3post,
  LD1x4post,
  ST1x2post,
  ST1x3post,
  ST1x4post,
  LD1DUPpost,
  LD2DUPpost,
  LD3DUPpost,
  LD4DUPpost,
  LD1LANEpost,
  LD2LANEpost,
  LD3LANEpost,
  LD4LANEpost,
  ST2LANEpost,
  ST3LANEpost,
  ST4LANEpost
};

} // end namespace AArch64ISD

class AArch64Subtarget;
class AArch64TargetMachine;

class AArch64TargetLowering : public TargetLowering {
  bool RequireStrictAlign;

public:
  explicit AArch64TargetLowering(TargetMachine &TM);

  /// Selects the correct CCAssignFn for a given CallingConvention value.
  CCAssignFn *CCAssignFnForCall(CallingConv::ID CC, bool IsVarArg) const;

  /// computeKnownBitsForTargetNode - Determine which of the bits specified in
  /// Mask are known to be either zero or one and return them in the
  /// KnownZero/KnownOne bitsets.
  void computeKnownBitsForTargetNode(const SDValue Op, APInt &KnownZero,
                                     APInt &KnownOne, const SelectionDAG &DAG,
                                     unsigned Depth = 0) const override;

  MVT getScalarShiftAmountTy(EVT LHSTy) const override;

  /// allowsMisalignedMemoryAccesses - Returns true if the target allows
  /// unaligned memory accesses. of the specified type.
  bool allowsMisalignedMemoryAccesses(EVT VT, unsigned AddrSpace = 0,
                                      unsigned Align = 1,
                                      bool *Fast = nullptr) const override {
    if (RequireStrictAlign)
      return false;
    // FIXME: True for Cyclone, but not necessary others.
    if (Fast)
      *Fast = true;
    return true;
  }

  /// LowerOperation - Provide custom lowering hooks for some operations.
  SDValue LowerOperation(SDValue Op, SelectionDAG &DAG) const override;

  const char *getTargetNodeName(unsigned Opcode) const override;

  SDValue PerformDAGCombine(SDNode *N, DAGCombinerInfo &DCI) const override;

  /// getFunctionAlignment - Return the Log2 alignment of this function.
  unsigned getFunctionAlignment(const Function *F) const;

  /// getMaximalGlobalOffset - Returns the maximal possible offset which can
  /// be used for loads / stores from the global.
  unsigned getMaximalGlobalOffset() const override;

  /// Returns true if a cast between SrcAS and DestAS is a noop.
  bool isNoopAddrSpaceCast(unsigned SrcAS, unsigned DestAS) const override {
    // Addrspacecasts are always noops.
    return true;
  }

  /// createFastISel - This method returns a target specific FastISel object,
  /// or null if the target does not support "fast" ISel.
  FastISel *createFastISel(FunctionLoweringInfo &funcInfo,
                           const TargetLibraryInfo *libInfo) const override;

  bool isOffsetFoldingLegal(const GlobalAddressSDNode *GA) const override;

  bool isFPImmLegal(const APFloat &Imm, EVT VT) const override;

  /// isShuffleMaskLegal - Return true if the given shuffle mask can be
  /// codegen'd directly, or if it should be stack expanded.
  bool isShuffleMaskLegal(const SmallVectorImpl<int> &M, EVT VT) const override;

  /// getSetCCResultType - Return the ISD::SETCC ValueType
  EVT getSetCCResultType(LLVMContext &Context, EVT VT) const override;

  SDValue ReconstructShuffle(SDValue Op, SelectionDAG &DAG) const;

  MachineBasicBlock *EmitF128CSEL(MachineInstr *MI,
                                  MachineBasicBlock *BB) const;

  MachineBasicBlock *
  EmitInstrWithCustomInserter(MachineInstr *MI,
                              MachineBasicBlock *MBB) const override;

  bool getTgtMemIntrinsic(IntrinsicInfo &Info, const CallInst &I,
                          unsigned Intrinsic) const override;

  bool isTruncateFree(Type *Ty1, Type *Ty2) const override;
  bool isTruncateFree(EVT VT1, EVT VT2) const override;

  bool isZExtFree(Type *Ty1, Type *Ty2) const override;
  bool isZExtFree(EVT VT1, EVT VT2) const override;
  bool isZExtFree(SDValue Val, EVT VT2) const override;

  bool hasPairedLoad(Type *LoadedType,
                     unsigned &RequiredAligment) const override;
  bool hasPairedLoad(EVT LoadedType, unsigned &RequiredAligment) const override;

  bool isLegalAddImmediate(int64_t) const override;
  bool isLegalICmpImmediate(int64_t) const override;

  EVT getOptimalMemOpType(uint64_t Size, unsigned DstAlign, unsigned SrcAlign,
                          bool IsMemset, bool ZeroMemset, bool MemcpyStrSrc,
                          MachineFunction &MF) const override;

  /// isLegalAddressingMode - Return true if the addressing mode represented
  /// by AM is legal for this target, for a load/store of the specified type.
  bool isLegalAddressingMode(const AddrMode &AM, Type *Ty) const override;

  /// \brief Return the cost of the scaling factor used in the addressing
  /// mode represented by AM for this target, for a load/store
  /// of the specified type.
  /// If the AM is supported, the return value must be >= 0.
  /// If the AM is not supported, it returns a negative value.
  int getScalingFactorCost(const AddrMode &AM, Type *Ty) const override;

  /// isFMAFasterThanFMulAndFAdd - Return true if an FMA operation is faster
  /// than a pair of fmul and fadd instructions. fmuladd intrinsics will be
  /// expanded to FMAs when this method returns true, otherwise fmuladd is
  /// expanded to fmul + fadd.
  bool isFMAFasterThanFMulAndFAdd(EVT VT) const override;

  const MCPhysReg *getScratchRegisters(CallingConv::ID CC) const override;

  /// \brief Returns false if N is a bit extraction pattern of (X >> C) & Mask.
  bool isDesirableToCommuteWithShift(const SDNode *N) const override;

  /// \brief Returns true if it is beneficial to convert a load of a constant
  /// to just the constant itself.
  bool shouldConvertConstantLoadToIntImm(const APInt &Imm,
                                         Type *Ty) const override;

  Value *emitLoadLinked(IRBuilder<> &Builder, Value *Addr,
                        AtomicOrdering Ord) const override;
  Value *emitStoreConditional(IRBuilder<> &Builder, Value *Val,
                              Value *Addr, AtomicOrdering Ord) const override;

  bool shouldExpandAtomicInIR(Instruction *Inst) const override;

  bool useLoadStackGuardNode() const override;
  TargetLoweringBase::LegalizeTypeAction
  getPreferredVectorAction(EVT VT) const override;

private:
  /// Subtarget - Keep a pointer to the AArch64Subtarget around so that we can
  /// make the right decision when generating code for different targets.
  const AArch64Subtarget *Subtarget;

  void addTypeForNEON(EVT VT, EVT PromotedBitwiseVT);
  void addDRTypeForNEON(MVT VT);
  void addQRTypeForNEON(MVT VT);

  SDValue
  LowerFormalArguments(SDValue Chain, CallingConv::ID CallConv, bool isVarArg,
                       const SmallVectorImpl<ISD::InputArg> &Ins, SDLoc DL,
                       SelectionDAG &DAG,
                       SmallVectorImpl<SDValue> &InVals) const override;

  SDValue LowerCall(CallLoweringInfo & /*CLI*/,
                    SmallVectorImpl<SDValue> &InVals) const override;

  SDValue LowerCallResult(SDValue Chain, SDValue InFlag,
                          CallingConv::ID CallConv, bool isVarArg,
                          const SmallVectorImpl<ISD::InputArg> &Ins, SDLoc DL,
                          SelectionDAG &DAG, SmallVectorImpl<SDValue> &InVals,
                          bool isThisReturn, SDValue ThisVal) const;

  bool isEligibleForTailCallOptimization(
      SDValue Callee, CallingConv::ID CalleeCC, bool isVarArg,
      bool isCalleeStructRet, bool isCallerStructRet,
      const SmallVectorImpl<ISD::OutputArg> &Outs,
      const SmallVectorImpl<SDValue> &OutVals,
      const SmallVectorImpl<ISD::InputArg> &Ins, SelectionDAG &DAG) const;

  /// Finds the incoming stack arguments which overlap the given fixed stack
  /// object and incorporates their load into the current chain. This prevents
  /// an upcoming store from clobbering the stack argument before it's used.
  SDValue addTokenForArgument(SDValue Chain, SelectionDAG &DAG,
                              MachineFrameInfo *MFI, int ClobberedFI) const;

  bool DoesCalleeRestoreStack(CallingConv::ID CallCC, bool TailCallOpt) const;

  bool IsTailCallConvention(CallingConv::ID CallCC) const;

  void saveVarArgRegisters(CCState &CCInfo, SelectionDAG &DAG, SDLoc DL,
                           SDValue &Chain) const;

  bool CanLowerReturn(CallingConv::ID CallConv, MachineFunction &MF,
                      bool isVarArg,
                      const SmallVectorImpl<ISD::OutputArg> &Outs,
                      LLVMContext &Context) const override;

  SDValue LowerReturn(SDValue Chain, CallingConv::ID CallConv, bool isVarArg,
                      const SmallVectorImpl<ISD::OutputArg> &Outs,
                      const SmallVectorImpl<SDValue> &OutVals, SDLoc DL,
                      SelectionDAG &DAG) const override;

  SDValue LowerGlobalAddress(SDValue Op, SelectionDAG &DAG) const;
  SDValue LowerGlobalTLSAddress(SDValue Op, SelectionDAG &DAG) const;
  SDValue LowerDarwinGlobalTLSAddress(SDValue Op, SelectionDAG &DAG) const;
  SDValue LowerELFGlobalTLSAddress(SDValue Op, SelectionDAG &DAG) const;
  SDValue LowerELFTLSDescCall(SDValue SymAddr, SDValue DescAddr, SDLoc DL,
                              SelectionDAG &DAG) const;
  SDValue LowerSETCC(SDValue Op, SelectionDAG &DAG) const;
  SDValue LowerBR_CC(SDValue Op, SelectionDAG &DAG) const;
  SDValue LowerSELECT(SDValue Op, SelectionDAG &DAG) const;
  SDValue LowerSELECT_CC(SDValue Op, SelectionDAG &DAG) const;
  SDValue LowerJumpTable(SDValue Op, SelectionDAG &DAG) const;
  SDValue LowerConstantPool(SDValue Op, SelectionDAG &DAG) const;
  SDValue LowerBlockAddress(SDValue Op, SelectionDAG &DAG) const;
  SDValue LowerAAPCS_VASTART(SDValue Op, SelectionDAG &DAG) const;
  SDValue LowerDarwin_VASTART(SDValue Op, SelectionDAG &DAG) const;
  SDValue LowerVASTART(SDValue Op, SelectionDAG &DAG) const;
  SDValue LowerVACOPY(SDValue Op, SelectionDAG &DAG) const;
  SDValue LowerVAARG(SDValue Op, SelectionDAG &DAG) const;
  SDValue LowerFRAMEADDR(SDValue Op, SelectionDAG &DAG) const;
  SDValue LowerRETURNADDR(SDValue Op, SelectionDAG &DAG) const;
  SDValue LowerINSERT_VECTOR_ELT(SDValue Op, SelectionDAG &DAG) const;
  SDValue LowerEXTRACT_VECTOR_ELT(SDValue Op, SelectionDAG &DAG) const;
  SDValue LowerSCALAR_TO_VECTOR(SDValue Op, SelectionDAG &DAG) const;
  SDValue LowerBUILD_VECTOR(SDValue Op, SelectionDAG &DAG) const;
  SDValue LowerVECTOR_SHUFFLE(SDValue Op, SelectionDAG &DAG) const;
  SDValue LowerEXTRACT_SUBVECTOR(SDValue Op, SelectionDAG &DAG) const;
  SDValue LowerVectorSRA_SRL_SHL(SDValue Op, SelectionDAG &DAG) const;
  SDValue LowerShiftLeftParts(SDValue Op, SelectionDAG &DAG) const;
  SDValue LowerShiftRightParts(SDValue Op, SelectionDAG &DAG) const;
  SDValue LowerVSETCC(SDValue Op, SelectionDAG &DAG) const;
  SDValue LowerCTPOP(SDValue Op, SelectionDAG &DAG) const;
  SDValue LowerF128Call(SDValue Op, SelectionDAG &DAG,
                        RTLIB::Libcall Call) const;
  SDValue LowerFCOPYSIGN(SDValue Op, SelectionDAG &DAG) const;
  SDValue LowerFP_EXTEND(SDValue Op, SelectionDAG &DAG) const;
  SDValue LowerFP_ROUND(SDValue Op, SelectionDAG &DAG) const;
  SDValue LowerFP_TO_INT(SDValue Op, SelectionDAG &DAG) const;
  SDValue LowerINT_TO_FP(SDValue Op, SelectionDAG &DAG) const;
  SDValue LowerVectorAND(SDValue Op, SelectionDAG &DAG) const;
  SDValue LowerVectorOR(SDValue Op, SelectionDAG &DAG) const;
  SDValue LowerCONCAT_VECTORS(SDValue Op, SelectionDAG &DAG) const;
  SDValue LowerFSINCOS(SDValue Op, SelectionDAG &DAG) const;

  SDValue BuildSDIVPow2(SDNode *N, const APInt &Divisor, SelectionDAG &DAG,
                        std::vector<SDNode *> *Created) const override;

  ConstraintType
  getConstraintType(const std::string &Constraint) const override;
  unsigned getRegisterByName(const char* RegName, EVT VT) const override;

  /// Examine constraint string and operand type and determine a weight value.
  /// The operand object must already have been set up with the operand type.
  ConstraintWeight
  getSingleConstraintMatchWeight(AsmOperandInfo &info,
                                 const char *constraint) const override;

  std::pair<unsigned, const TargetRegisterClass *>
  getRegForInlineAsmConstraint(const std::string &Constraint,
                               MVT VT) const override;
  void LowerAsmOperandForConstraint(SDValue Op, std::string &Constraint,
                                    std::vector<SDValue> &Ops,
                                    SelectionDAG &DAG) const override;

  bool isUsedByReturnOnly(SDNode *N, SDValue &Chain) const override;
  bool mayBeEmittedAsTailCall(CallInst *CI) const override;
  bool getIndexedAddressParts(SDNode *Op, SDValue &Base, SDValue &Offset,
                              ISD::MemIndexedMode &AM, bool &IsInc,
                              SelectionDAG &DAG) const;
  bool getPreIndexedAddressParts(SDNode *N, SDValue &Base, SDValue &Offset,
                                 ISD::MemIndexedMode &AM,
                                 SelectionDAG &DAG) const override;
  bool getPostIndexedAddressParts(SDNode *N, SDNode *Op, SDValue &Base,
                                  SDValue &Offset, ISD::MemIndexedMode &AM,
                                  SelectionDAG &DAG) const override;

  void ReplaceNodeResults(SDNode *N, SmallVectorImpl<SDValue> &Results,
                          SelectionDAG &DAG) const override;
};

namespace AArch64 {
FastISel *createFastISel(FunctionLoweringInfo &funcInfo,
                         const TargetLibraryInfo *libInfo);
} // end namespace AArch64

} // end namespace llvm

#endif
