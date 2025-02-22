//==-- llvm/Target/TargetSubtargetInfo.h - Target Information ----*- C++ -*-==//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file describes the subtarget options of a Target machine.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_TARGET_TARGETSUBTARGETINFO_H
#define LLVM_TARGET_TARGETSUBTARGETINFO_H

#include "llvm/MC/MCSubtargetInfo.h"
#include "llvm/Support/CodeGen.h"

namespace llvm {

class DataLayout;
class MachineFunction;
class MachineInstr;
class SDep;
class SUnit;
class TargetFrameLowering;
class TargetInstrInfo;
class TargetLowering;
class TargetRegisterClass;
class TargetRegisterInfo;
class TargetSchedModel;
class TargetSelectionDAGInfo;
struct MachineSchedPolicy;
template <typename T> class SmallVectorImpl;

//===----------------------------------------------------------------------===//
///
/// TargetSubtargetInfo - Generic base class for all target subtargets.  All
/// Target-specific options that control code generation and printing should
/// be exposed through a TargetSubtargetInfo-derived class.
///
class TargetSubtargetInfo : public MCSubtargetInfo {
  TargetSubtargetInfo(const TargetSubtargetInfo&) LLVM_DELETED_FUNCTION;
  void operator=(const TargetSubtargetInfo&) LLVM_DELETED_FUNCTION;
protected: // Can only create subclasses...
  TargetSubtargetInfo();
public:
  // AntiDepBreakMode - Type of anti-dependence breaking that should
  // be performed before post-RA scheduling.
  typedef enum { ANTIDEP_NONE, ANTIDEP_CRITICAL, ANTIDEP_ALL } AntiDepBreakMode;
  typedef SmallVectorImpl<const TargetRegisterClass*> RegClassVector;

  virtual ~TargetSubtargetInfo();

  // Interfaces to the major aspects of target machine information:
  //
  // -- Instruction opcode and operand information
  // -- Pipelines and scheduling information
  // -- Stack frame information
  // -- Selection DAG lowering information
  //
  // N.B. These objects may change during compilation. It's not safe to cache
  // them between functions.
  virtual const TargetInstrInfo *getInstrInfo() const { return nullptr; }
  virtual const TargetFrameLowering *getFrameLowering() const {
    return nullptr;
  }
  virtual const TargetLowering *getTargetLowering() const { return nullptr; }
  virtual const TargetSelectionDAGInfo *getSelectionDAGInfo() const {
    return nullptr;
  }
  virtual const DataLayout *getDataLayout() const { return nullptr; }

  /// getRegisterInfo - If register information is available, return it.  If
  /// not, return null.  This is kept separate from RegInfo until RegInfo has
  /// details of graph coloring register allocation removed from it.
  ///
  virtual const TargetRegisterInfo *getRegisterInfo() const { return nullptr; }

  /// getInstrItineraryData - Returns instruction itinerary data for the target
  /// or specific subtarget.
  ///
  virtual const InstrItineraryData *getInstrItineraryData() const {
    return nullptr;
  }

  /// Resolve a SchedClass at runtime, where SchedClass identifies an
  /// MCSchedClassDesc with the isVariant property. This may return the ID of
  /// another variant SchedClass, but repeated invocation must quickly terminate
  /// in a nonvariant SchedClass.
  virtual unsigned resolveSchedClass(unsigned SchedClass, const MachineInstr *MI,
                                     const TargetSchedModel* SchedModel) const {
    return 0;
  }

  /// \brief Temporary API to test migration to MI scheduler.
  bool useMachineScheduler() const;

  /// \brief True if the subtarget should run MachineScheduler after aggressive
  /// coalescing.
  ///
  /// This currently replaces the SelectionDAG scheduler with the "source" order
  /// scheduler. It does not yet disable the postRA scheduler.
  virtual bool enableMachineScheduler() const;

  /// \brief True if the subtarget should run PostMachineScheduler.
  ///
  /// This only takes effect if the target has configured the
  /// PostMachineScheduler pass to run, or if the global cl::opt flag,
  /// MISchedPostRA, is set.
  virtual bool enablePostMachineScheduler() const;

  /// \brief True if the subtarget should run the atomic expansion pass.
  virtual bool enableAtomicExpand() const;

  /// \brief Override generic scheduling policy within a region.
  ///
  /// This is a convenient way for targets that don't provide any custom
  /// scheduling heuristics (no custom MachineSchedStrategy) to make
  /// changes to the generic scheduling policy.
  virtual void overrideSchedPolicy(MachineSchedPolicy &Policy,
                                   MachineInstr *begin,
                                   MachineInstr *end,
                                   unsigned NumRegionInstrs) const {}

  // \brief Perform target specific adjustments to the latency of a schedule
  // dependency.
  virtual void adjustSchedDependency(SUnit *def, SUnit *use,
                                     SDep& dep) const { }

  // For use with PostRAScheduling: get the anti-dependence breaking that should
  // be performed before post-RA scheduling.
  virtual AntiDepBreakMode getAntiDepBreakMode() const {
    return ANTIDEP_NONE;
  }

  // For use with PostRAScheduling: in CriticalPathRCs, return any register
  // classes that should only be considered for anti-dependence breaking if they
  // are on the critical path.
  virtual void getCriticalPathRCs(RegClassVector &CriticalPathRCs) const {
    return CriticalPathRCs.clear();
  }

  // For use with PostRAScheduling: get the minimum optimization level needed
  // to enable post-RA scheduling.
  virtual CodeGenOpt::Level getOptLevelToEnablePostRAScheduler() const {
    return CodeGenOpt::Default;
  }

  /// \brief True if the subtarget should run the local reassignment
  /// heuristic of the register allocator.
  /// This heuristic may be compile time intensive, \p OptLevel provides
  /// a finer grain to tune the register allocator.
  virtual bool enableRALocalReassignment(CodeGenOpt::Level OptLevel) const;

  /// \brief Enable use of alias analysis during code generation (during MI
  /// scheduling, DAGCombine, etc.).
  virtual bool useAA() const;

  /// \brief Enable the use of the early if conversion pass.
  virtual bool enableEarlyIfConversion() const { return false; }

  /// \brief Reset the features for the subtarget.
  virtual void resetSubtargetFeatures(const MachineFunction *MF) { }

};

} // End llvm namespace

#endif
