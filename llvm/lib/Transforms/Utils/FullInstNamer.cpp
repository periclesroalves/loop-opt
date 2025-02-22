#include "llvm/Transforms/Utils/FullInstNamer.h"
#include "llvm/Transforms/Scalar.h"
#include <llvm/IR/Function.h>
#include <llvm/IR/Value.h>
#include <llvm/IR/InstrTypes.h>
#include <llvm/IR/Metadata.h>
#include <llvm/IR/MDBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/ValueSymbolTable.h>
#include <llvm/Pass.h>
#include <llvm/Support/Debug.h>

#include <string>
#include <cassert>

using namespace llvm;
using namespace std;

char FullInstNamer::ID = 0;

INITIALIZE_PASS(FullInstNamer,
	"full-instnamer",
	"Assign names to anonymous instructions (including void instructions)",
	false,
	false
)

FunctionPass *llvm::createFullInstructionNamerPass() {
  return new FullInstNamer();
}

FullInstNamer::FullInstNamer() : FunctionPass(ID) {
	initializeFullInstNamerPass(*PassRegistry::getPassRegistry());
}


bool FullInstNamer::runOnFunction(Function &F)
{
  size_t void_type_counter = 1;

  unsigned kindId = getNameMDKindID();

  LLVMContext& ctx = getGlobalContext();

  MDBuilder md{ctx};

  for(Function::arg_iterator AI = F.arg_begin(), AE = F.arg_end(); AI != AE; ++AI)
  {
    if(!AI->hasName() && !AI->getType()->isVoidTy())
    	AI->setName("arg");
  }

  for(Function::iterator BB = F.begin(), E = F.end(); BB != E; ++BB)
  {
    if(!BB->hasName()) BB->setName("bb");

    for (BasicBlock::iterator I = BB->begin(), E = BB->end(); I != E; ++I)
    {
      if(I->getType()->isVoidTy()) {
      	if (!I->getMetadata(kindId))
		I->setMetadata(
			kindId,
			MDNode::get(ctx, md.createString("void" + std::to_string(void_type_counter++)))
		);
	} else {
		I->setName("tmp");
	}
    }
  }

  return true;
}

static llvm::MDNode* createNameMetadata(LLVMContext& ctx, llvm::StringRef name)
{
	MDBuilder md{ctx};

	return MDNode::get(ctx, md.createString(name));
}

unsigned FullInstNamer::getNameMDKindID()
{
	return getNameMDKindID(getGlobalContext());
}
unsigned FullInstNamer::getNameMDKindID(LLVMContext& ctx)
{
	static unsigned kindId;

	if (kindId == 0) {
		kindId = ctx.getMDKindID("name");
	}

	return kindId;
}

StringRef FullInstNamer::getName(const Value *v)
{
	StringRef name = v->getName();

	if (!name.empty())
		return name;

	const Instruction *instr = dyn_cast<Instruction>(v);

	if (!instr)
		return name;

	const MDNode *md = instr->getMetadata(getNameMDKindID());

	if (!md)
		return name;

	assert((md->getNumOperands() == 1) && "Invalid !name metadata");

	const MDString *md_str = dyn_cast<MDString>(md->getOperand(0));

	if (!md_str)
		return name;

	return md_str->getString();
}


StringRef FullInstNamer::getNameOrFail(const Pass *pass, const Value* v)
{
	StringRef name = getName(v);

	if (name.empty()) {
		errs() << "You must run the `-full-instnamer' pass before using the `-" << pass->getPassName() << "' pass\n";
		exit(1);
	}

	return name;
}

StringRef FullInstNamer::getNameOrFail(StringRef caller, const Value* v)
{
	StringRef name = getName(v);

	if (name.empty()) {
		errs() << "You must run the `-full-instnamer' pass before using the " << caller << "\n";
		exit(1);
	}

	return name;
}


void FullInstNamer::setName(llvm::LLVMContext& ctx, llvm::Value *v, llvm::StringRef name)
{
	if (v->getType()->isVoidTy()) {
		if (auto i = dyn_cast<Instruction>(v)) {
			i->setMetadata(
				getNameMDKindID(ctx),
				createNameMetadata(ctx, name)
			);
		} else {
			errs() << "Only instructions with type void can be named (not globals, etc.).\n";
			exit(1);
		}
	} else {
		v->setName(name);
	}
}

void FullInstNamer::setNameIfAbsent(llvm::LLVMContext& ctx, llvm::Value *v, llvm::StringRef name)
{
	StringRef old_name = getName(v);

	if (!(old_name.empty()))
		return;

	setName(ctx, v, name);
}

llvm::Value* FullInstNamer::lookup(llvm::Function *fn, StringRef name)
{
	FullInstNamer fin;

	auto value = fn->getValueSymbolTable().lookup(name);

	if (value)
		return value;

	for (auto& bb : fn->getBasicBlockList())
	{
		for (auto it = bb.begin(), end = bb.end(); it != end; ++it)
		{
			if (fin.getName(it) == name)
				return it;
		}
	}

	return nullptr;
}
