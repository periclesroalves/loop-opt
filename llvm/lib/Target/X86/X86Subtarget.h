//===-- X86Subtarget.h - Define Subtarget for the X86 ----------*- C++ -*--===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file declares the X86 specific subclass of TargetSubtargetInfo.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_X86_X86SUBTARGET_H
#define LLVM_LIB_TARGET_X86_X86SUBTARGET_H

#include "X86FrameLowering.h"
#include "X86ISelLowering.h"
#include "X86InstrInfo.h"
#include "X86SelectionDAGInfo.h"
#include "llvm/ADT/Triple.h"
#include "llvm/IR/CallingConv.h"
#include "llvm/Target/TargetSubtargetInfo.h"
#include <string>

#define GET_SUBTARGETINFO_HEADER
#include "X86GenSubtargetInfo.inc"

namespace llvm {
class GlobalValue;
class StringRef;
class TargetMachine;

/// PICStyles - The X86 backend supports a number of different styles of PIC.
///
namespace PICStyles {
enum Style {
  StubPIC,          // Used on i386-darwin in -fPIC mode.
  StubDynamicNoPIC, // Used on i386-darwin in -mdynamic-no-pic mode.
  GOT,              // Used on many 32-bit unices in -fPIC mode.
  RIPRel,           // Used on X86-64 when not in -static mode.
  None              // Set when in -static mode (not PIC or DynamicNoPIC mode).
};
}

class X86Subtarget final : public X86GenSubtargetInfo {

protected:
  enum X86SSEEnum {
    NoMMXSSE, MMX, SSE1, SSE2, SSE3, SSSE3, SSE41, SSE42, AVX, AVX2, AVX512F
  };

  enum X863DNowEnum {
    NoThreeDNow, ThreeDNow, ThreeDNowA
  };

  enum X86ProcFamilyEnum {
    Others, IntelAtom, IntelSLM
  };

  /// X86ProcFamily - X86 processor family: Intel Atom, and others
  X86ProcFamilyEnum X86ProcFamily;

  /// PICStyle - Which PIC style to use
  ///
  PICStyles::Style PICStyle;

  /// X86SSELevel - MMX, SSE1, SSE2, SSE3, SSSE3, SSE41, SSE42, or
  /// none supported.
  X86SSEEnum X86SSELevel;

  /// X863DNowLevel - 3DNow or 3DNow Athlon, or none supported.
  ///
  X863DNowEnum X863DNowLevel;

  /// HasCMov - True if this processor has conditional move instructions
  /// (generally pentium pro+).
  bool HasCMov;

  /// HasX86_64 - True if the processor supports X86-64 instructions.
  ///
  bool HasX86_64;

  /// HasPOPCNT - True if the processor supports POPCNT.
  bool HasPOPCNT;

  /// HasSSE4A - True if the processor supports SSE4A instructions.
  bool HasSSE4A;

  /// HasAES - Target has AES instructions
  bool HasAES;

  /// HasPCLMUL - Target has carry-less multiplication
  bool HasPCLMUL;

  /// HasFMA - Target has 3-operand fused multiply-add
  bool HasFMA;

  /// HasFMA4 - Target has 4-operand fused multiply-add
  bool HasFMA4;

  /// HasXOP - Target has XOP instructions
  bool HasXOP;

  /// HasTBM - Target has TBM instructions.
  bool HasTBM;

  /// HasMOVBE - True if the processor has the MOVBE instruction.
  bool HasMOVBE;

  /// HasRDRAND - True if the processor has the RDRAND instruction.
  bool HasRDRAND;

  /// HasF16C - Processor has 16-bit floating point conversion instructions.
  bool HasF16C;

  /// HasFSGSBase - Processor has FS/GS base insturctions.
  bool HasFSGSBase;

  /// HasLZCNT - Processor has LZCNT instruction.
  bool HasLZCNT;

  /// HasBMI - Processor has BMI1 instructions.
  bool HasBMI;

  /// HasBMI2 - Processor has BMI2 instructions.
  bool HasBMI2;

  /// HasRTM - Processor has RTM instructions.
  bool HasRTM;

  /// HasHLE - Processor has HLE.
  bool HasHLE;

  /// HasADX - Processor has ADX instructions.
  bool HasADX;

  /// HasSHA - Processor has SHA instructions.
  bool HasSHA;

  /// HasSGX - Processor has SGX instructions.
  bool HasSGX;

  /// HasPRFCHW - Processor has PRFCHW instructions.
  bool HasPRFCHW;

  /// HasRDSEED - Processor has RDSEED instructions.
  bool HasRDSEED;

  /// HasSMAP - Processor has SMAP instructions.
  bool HasSMAP;

  /// IsBTMemSlow - True if BT (bit test) of memory instructions are slow.
  bool IsBTMemSlow;

  /// IsSHLDSlow - True if SHLD instructions are slow.
  bool IsSHLDSlow;

  /// IsUAMemFast - True if unaligned memory access is fast.
  bool IsUAMemFast;

  /// HasVectorUAMem - True if SIMD operations can have unaligned memory
  /// operands. This may require setting a feature bit in the processor.
  bool HasVectorUAMem;

  /// HasCmpxchg16b - True if this processor has the CMPXCHG16B instruction;
  /// this is true for most x86-64 chips, but not the first AMD chips.
  bool HasCmpxchg16b;

  /// UseLeaForSP - True if the LEA instruction should be used for adjusting
  /// the stack pointer. This is an optimization for Intel Atom processors.
  bool UseLeaForSP;

  /// HasSlowDivide - True if smaller divides are significantly faster than
  /// full divides and should be used when possible.
  bool HasSlowDivide;

  /// PadShortFunctions - True if the short functions should be padded to prevent
  /// a stall when returning too early.
  bool PadShortFunctions;

  /// CallRegIndirect - True if the Calls with memory reference should be converted
  /// to a register-based indirect call.
  bool CallRegIndirect;
  /// LEAUsesAG - True if the LEA instruction inputs have to be ready at
  ///             address generation (AG) time.
  bool LEAUsesAG;

  /// SlowLEA - True if the LEA instruction with certain arguments is slow
  bool SlowLEA;

  /// SlowIncDec - True if INC and DEC instructions are slow when writing to flags
  bool SlowIncDec;

  /// Processor has AVX-512 PreFetch Instructions
  bool HasPFI;

  /// Processor has AVX-512 Exponential and Reciprocal Instructions
  bool HasERI;

  /// Processor has AVX-512 Conflict Detection Instructions
  bool HasCDI;

  /// Processor has AVX-512 Doubleword and Quadword instructions
  bool HasDQI;

  /// Processor has AVX-512 Byte and Word instructions
  bool HasBWI;

  /// Processor has AVX-512 Vector Length eXtenstions
  bool HasVLX;

  /// stackAlignment - The minimum alignment known to hold of the stack frame on
  /// entry to the function and which must be maintained by every function.
  unsigned stackAlignment;

  /// Max. memset / memcpy size that is turned into rep/movs, rep/stos ops.
  ///
  unsigned MaxInlineSizeThreshold;

  /// TargetTriple - What processor and OS we're targeting.
  Triple TargetTriple;

  /// Instruction itineraries for scheduling
  InstrItineraryData InstrItins;

private:
  // Calculates type size & alignment
  const DataLayout DL;

  /// StackAlignOverride - Override the stack alignment.
  unsigned StackAlignOverride;

  /// In64BitMode - True if compiling for 64-bit, false for 16-bit or 32-bit.
  bool In64BitMode;

  /// In32BitMode - True if compiling for 32-bit, false for 16-bit or 64-bit.
  bool In32BitMode;

  /// In16BitMode - True if compiling for 16-bit, false for 32-bit or 64-bit.
  bool In16BitMode;

  X86SelectionDAGInfo TSInfo;
  // Ordering here is important. X86InstrInfo initializes X86RegisterInfo which
  // X86TargetLowering needs.
  X86InstrInfo InstrInfo;
  X86TargetLowering TLInfo;
  X86FrameLowering FrameLowering;

public:
  /// This constructor initializes the data members to match that
  /// of the specified triple.
  ///
  X86Subtarget(const std::string &TT, const std::string &CPU,
               const std::string &FS, X86TargetMachine &TM,
               unsigned StackAlignOverride);

  const X86TargetLowering *getTargetLowering() const override {
    return &TLInfo;
  }
  const X86InstrInfo *getInstrInfo() const override { return &InstrInfo; }
  const DataLayout *getDataLayout() const override { return &DL; }
  const X86FrameLowering *getFrameLowering() const override {
    return &FrameLowering;
  }
  const X86SelectionDAGInfo *getSelectionDAGInfo() const override {
    return &TSInfo;
  }
  const X86RegisterInfo *getRegisterInfo() const override {
    return &getInstrInfo()->getRegisterInfo();
  }

  /// getStackAlignment - Returns the minimum alignment known to hold of the
  /// stack frame on entry to the function and which must be maintained by every
  /// function for this subtarget.
  unsigned getStackAlignment() const { return stackAlignment; }

  /// getMaxInlineSizeThreshold - Returns the maximum memset / memcpy size
  /// that still makes it profitable to inline the call.
  unsigned getMaxInlineSizeThreshold() const { return MaxInlineSizeThreshold; }

  /// ParseSubtargetFeatures - Parses features string setting specified
  /// subtarget options.  Definition of function is auto generated by tblgen.
  void ParseSubtargetFeatures(StringRef CPU, StringRef FS);

  /// \brief Reset the features for the X86 target.
  void resetSubtargetFeatures(const MachineFunction *MF) override;
private:
  /// \brief Initialize the full set of dependencies so we can use an initializer
  /// list for X86Subtarget.
  X86Subtarget &initializeSubtargetDependencies(StringRef CPU, StringRef FS);
  void initializeEnvironment();
  void resetSubtargetFeatures(StringRef CPU, StringRef FS);
public:
  /// Is this x86_64? (disregarding specific ABI / programming model)
  bool is64Bit() const {
    return In64BitMode;
  }

  bool is32Bit() const {
    return In32BitMode;
  }

  bool is16Bit() const {
    return In16BitMode;
  }

  /// Is this x86_64 with the ILP32 programming model (x32 ABI)?
  bool isTarget64BitILP32() const {
    return In64BitMode && (TargetTriple.getEnvironment() == Triple::GNUX32 ||
                           TargetTriple.getOS() == Triple::NaCl);
  }

  /// Is this x86_64 with the LP64 programming model (standard AMD64, no x32)?
  bool isTarget64BitLP64() const {
    return In64BitMode && (TargetTriple.getEnvironment() != Triple::GNUX32 &&
                           TargetTriple.getOS() != Triple::NaCl);
  }

  PICStyles::Style getPICStyle() const { return PICStyle; }
  void setPICStyle(PICStyles::Style Style)  { PICStyle = Style; }

  bool hasCMov() const { return HasCMov; }
  bool hasMMX() const { return X86SSELevel >= MMX; }
  bool hasSSE1() const { return X86SSELevel >= SSE1; }
  bool hasSSE2() const { return X86SSELevel >= SSE2; }
  bool hasSSE3() const { return X86SSELevel >= SSE3; }
  bool hasSSSE3() const { return X86SSELevel >= SSSE3; }
  bool hasSSE41() const { return X86SSELevel >= SSE41; }
  bool hasSSE42() const { return X86SSELevel >= SSE42; }
  bool hasAVX() const { return X86SSELevel >= AVX; }
  bool hasAVX2() const { return X86SSELevel >= AVX2; }
  bool hasAVX512() const { return X86SSELevel >= AVX512F; }
  bool hasFp256() const { return hasAVX(); }
  bool hasInt256() const { return hasAVX2(); }
  bool hasSSE4A() const { return HasSSE4A; }
  bool has3DNow() const { return X863DNowLevel >= ThreeDNow; }
  bool has3DNowA() const { return X863DNowLevel >= ThreeDNowA; }
  bool hasPOPCNT() const { return HasPOPCNT; }
  bool hasAES() const { return HasAES; }
  bool hasPCLMUL() const { return HasPCLMUL; }
  bool hasFMA() const { return HasFMA; }
  // FIXME: Favor FMA when both are enabled. Is this the right thing to do?
  bool hasFMA4() const { return HasFMA4 && !HasFMA; }
  bool hasXOP() const { return HasXOP; }
  bool hasTBM() const { return HasTBM; }
  bool hasMOVBE() const { return HasMOVBE; }
  bool hasRDRAND() const { return HasRDRAND; }
  bool hasF16C() const { return HasF16C; }
  bool hasFSGSBase() const { return HasFSGSBase; }
  bool hasLZCNT() const { return HasLZCNT; }
  bool hasBMI() const { return HasBMI; }
  bool hasBMI2() const { return HasBMI2; }
  bool hasRTM() const { return HasRTM; }
  bool hasHLE() const { return HasHLE; }
  bool hasADX() const { return HasADX; }
  bool hasSHA() const { return HasSHA; }
  bool hasSGX() const { return HasSGX; }
  bool hasPRFCHW() const { return HasPRFCHW; }
  bool hasRDSEED() const { return HasRDSEED; }
  bool hasSMAP() const { return HasSMAP; }
  bool isBTMemSlow() const { return IsBTMemSlow; }
  bool isSHLDSlow() const { return IsSHLDSlow; }
  bool isUnalignedMemAccessFast() const { return IsUAMemFast; }
  bool hasVectorUAMem() const { return HasVectorUAMem; }
  bool hasCmpxchg16b() const { return HasCmpxchg16b; }
  bool useLeaForSP() const { return UseLeaForSP; }
  bool hasSlowDivide() const { return HasSlowDivide; }
  bool padShortFunctions() const { return PadShortFunctions; }
  bool callRegIndirect() const { return CallRegIndirect; }
  bool LEAusesAG() const { return LEAUsesAG; }
  bool slowLEA() const { return SlowLEA; }
  bool slowIncDec() const { return SlowIncDec; }
  bool hasCDI() const { return HasCDI; }
  bool hasPFI() const { return HasPFI; }
  bool hasERI() const { return HasERI; }
  bool hasDQI() const { return HasDQI; }
  bool hasBWI() const { return HasBWI; }
  bool hasVLX() const { return HasVLX; }

  bool isAtom() const { return X86ProcFamily == IntelAtom; }
  bool isSLM() const { return X86ProcFamily == IntelSLM; }

  const Triple &getTargetTriple() const { return TargetTriple; }

  bool isTargetDarwin() const { return TargetTriple.isOSDarwin(); }
  bool isTargetFreeBSD() const {
    return TargetTriple.getOS() == Triple::FreeBSD;
  }
  bool isTargetSolaris() const {
    return TargetTriple.getOS() == Triple::Solaris;
  }

  bool isTargetELF() const { return TargetTriple.isOSBinFormatELF(); }
  bool isTargetCOFF() const { return TargetTriple.isOSBinFormatCOFF(); }
  bool isTargetMacho() const { return TargetTriple.isOSBinFormatMachO(); }

  bool isTargetLinux() const { return TargetTriple.isOSLinux(); }
  bool isTargetNaCl() const { return TargetTriple.isOSNaCl(); }
  bool isTargetNaCl32() const { return isTargetNaCl() && !is64Bit(); }
  bool isTargetNaCl64() const { return isTargetNaCl() && is64Bit(); }

  bool isTargetWindowsMSVC() const {
    return TargetTriple.isWindowsMSVCEnvironment();
  }

  bool isTargetKnownWindowsMSVC() const {
    return TargetTriple.isKnownWindowsMSVCEnvironment();
  }

  bool isTargetWindowsCygwin() const {
    return TargetTriple.isWindowsCygwinEnvironment();
  }

  bool isTargetWindowsGNU() const {
    return TargetTriple.isWindowsGNUEnvironment();
  }

  bool isTargetCygMing() const { return TargetTriple.isOSCygMing(); }

  bool isOSWindows() const { return TargetTriple.isOSWindows(); }

  bool isTargetWin64() const {
    return In64BitMode && TargetTriple.isOSWindows();
  }

  bool isTargetWin32() const {
    return !In64BitMode && (isTargetCygMing() || isTargetKnownWindowsMSVC());
  }

  bool isPICStyleSet() const { return PICStyle != PICStyles::None; }
  bool isPICStyleGOT() const { return PICStyle == PICStyles::GOT; }
  bool isPICStyleRIPRel() const { return PICStyle == PICStyles::RIPRel; }

  bool isPICStyleStubPIC() const {
    return PICStyle == PICStyles::StubPIC;
  }

  bool isPICStyleStubNoDynamic() const {
    return PICStyle == PICStyles::StubDynamicNoPIC;
  }
  bool isPICStyleStubAny() const {
    return PICStyle == PICStyles::StubDynamicNoPIC ||
           PICStyle == PICStyles::StubPIC;
  }

  bool isCallingConvWin64(CallingConv::ID CC) const {
    return (isTargetWin64() && CC != CallingConv::X86_64_SysV) ||
           CC == CallingConv::X86_64_Win64;
  }

  /// ClassifyGlobalReference - Classify a global variable reference for the
  /// current subtarget according to how we should reference it in a non-pcrel
  /// context.
  unsigned char ClassifyGlobalReference(const GlobalValue *GV,
                                        const TargetMachine &TM)const;

  /// ClassifyBlockAddressReference - Classify a blockaddress reference for the
  /// current subtarget according to how we should reference it in a non-pcrel
  /// context.
  unsigned char ClassifyBlockAddressReference() const;

  /// IsLegalToCallImmediateAddr - Return true if the subtarget allows calls
  /// to immediate address.
  bool IsLegalToCallImmediateAddr(const TargetMachine &TM) const;

  /// This function returns the name of a function which has an interface
  /// like the non-standard bzero function, if such a function exists on
  /// the current subtarget and it is considered prefereable over
  /// memset with zero passed as the second argument. Otherwise it
  /// returns null.
  const char *getBZeroEntry() const;

  /// This function returns true if the target has sincos() routine in its
  /// compiler runtime or math libraries.
  bool hasSinCos() const;

  /// Enable the MachineScheduler pass for all X86 subtargets.
  bool enableMachineScheduler() const override { return true; }

  bool enableEarlyIfConversion() const override;

  /// getInstrItins = Return the instruction itineraries based on the
  /// subtarget selection.
  const InstrItineraryData *getInstrItineraryData() const override {
    return &InstrItins;
  }

  AntiDepBreakMode getAntiDepBreakMode() const override {
    return TargetSubtargetInfo::ANTIDEP_CRITICAL;
  }
};

} // End llvm namespace

#endif
