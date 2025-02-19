##===- polly/lib/Makefile -----------------------*- Makefile -*-===##

#
# Indicate where we are relative to the top of the source tree.
#
LEVEL :=..

LIBRARYNAME=LLVMPolly
LOADABLE_MODULE = 1

# TODO: Export symbols for RTTI or EH?

CPP.Flags += $(POLLY_INC)
LD.Flags += $(POLLY_LD) $(POLLY_LIB)

include $(LEVEL)/Makefile.config

# Enable optional source files
ifeq ($(CLOOG_FOUND), yes)
CLOOG_FILES= CodeGen/Cloog.cpp \
	     CodeGen/CodeGeneration.cpp
endif

ifeq ($(GPU_CODEGEN), yes)
GPGPU_CODEGEN_FILES= CodeGen/PTXGenerator.cpp
endif

ifeq ($(PLUTO_FOUND), yes)
POLLY_PLUTO_FILES= Transform/Pluto.cpp
endif

ISL_CODEGEN_FILES= CodeGen/IslAst.cpp \
                   CodeGen/IslExprBuilder.cpp \
                   CodeGen/IslCodeGeneration.cpp

POLLY_JSON_FILES= JSON/json_reader.cpp \
		  JSON/json_value.cpp \
		  JSON/json_writer.cpp

SOURCES= Polly.cpp \
	 Support/GICHelper.cpp \
	 Support/SCEVValidator.cpp \
	 Support/RegisterPasses.cpp \
	 Support/ScopHelper.cpp \
	 Support/AliasCheckBuilders.cpp \
	 Analysis/Dependences.cpp \
	 Analysis/RegionAliasInfo.cpp \
	 Analysis/ScopDetection.cpp \
	 Analysis/ScopDetectionDiagnostic.cpp \
	 Analysis/ScopInfo.cpp \
	 Analysis/ScopGraphPrinter.cpp \
	 Analysis/ScopPass.cpp \
	 Analysis/TempScopInfo.cpp \
	 Analysis/SpeculativeAliasAnalysis.cpp \
	 Analysis/AliasAnalysisEvaluator.cpp \
	 CodeGen/BlockGenerators.cpp \
	 CodeGen/LoopGenerators.cpp \
	 CodeGen/IRBuilder.cpp \
	 CodeGen/Utils.cpp \
	 CodeGen/RuntimeDebugBuilder.cpp \
	 Exchange/JSONExporter.cpp \
	 Transform/AliasProfiler.cpp \
	 Transform/SCEVAliasInstrumenter.cpp \
	 Transform/Canonicalization.cpp \
	 Transform/CodePreparation.cpp \
	 Transform/DeadCodeElimination.cpp \
	 Transform/IndependentBlocks.cpp \
	 Transform/IndVarSimplify.cpp \
	 Transform/ScheduleOptimizer.cpp \
	 Transform/RegionCloningPass.cpp \
	 Transform/CloneRegion.cpp \
	 Transform/SCEVRangeBuilder.cpp \
	 ${GPGPU_FILES} \
	 ${ISL_CODEGEN_FILES} \
         ${CLOOG_FILES} \
	 ${POLLY_JSON_FILES} \
	 ${POLLY_PLUTO_FILES}

#
# Include Makefile.common so we know what to do.
#
include $(LEVEL)/Makefile.common

LIBS += $(POLLY_LD) $(POLLY_LIB)
