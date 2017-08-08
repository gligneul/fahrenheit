/*
 * MIT License
 * 
 * Copyright (c) 2017 Gabriel de Quadros Ligneul
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#include <sstream>
#include <memory>
#include <vector>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wshadow"
#include <llvm/ExecutionEngine/MCJIT.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Verifier.h>
#include <llvm/Support/DynamicLibrary.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Support/TargetSelect.h>
#pragma GCC diagnostic pop

extern "C" {
#include <fahrenheit/backend.h>
#include <fahrenheit/ir.h>
}

namespace {

static llvm::LLVMContext TheContext;

/* Engine exported */
struct FEngineData {
  std::unique_ptr<llvm::ExecutionEngine> ee;
  std::vector<FJitFunc> functions;
};

/* Compile state for a module */
struct ModuleState {
  FEngineData &engine;
  FModule *irmodule;
  std::unique_ptr<llvm::Module> module;
  std::vector<llvm::Function *> functions;

  ModuleState(FEngineData &engine_, FModule *irmodule_)
    : engine(engine_)
    , irmodule(irmodule_)
    , module(new llvm::Module("m", TheContext)) {}
};

/* Compile state for a function */
struct FunctionState {
  int function;
  std::vector<llvm::BasicBlock *> bblocks;
  std::vector<std::vector<llvm::Value *>> values;
};

/* Convert an fahrenheit type to a llvm type */
llvm::Type *convert_type(enum FType type) {
  switch (type) {
    case FBool:
      return llvm::IntegerType::get(TheContext, 1);
    case FInt8:
      return llvm::IntegerType::get(TheContext, 8);
    case FInt16:
      return llvm::IntegerType::get(TheContext, 16);
    case FInt32:
      return llvm::IntegerType::get(TheContext, 32);
    case FInt64:
      return llvm::IntegerType::get(TheContext, 64);
    case FFloat:
      return llvm::Type::getFloatTy(TheContext);
    case FDouble:
      return llvm::Type::getDoubleTy(TheContext);
    case FPointer:
      return llvm::PointerType::get(
        llvm::IntegerType::get(TheContext, 8), 0);
    case FVoid:
      return llvm::Type::getVoidTy(TheContext);
  }
  return nullptr;
}

llvm::Instruction::BinaryOps convert_binop(enum FBinopTag op, enum FType type) {
  switch (op) {
    case FAdd:
      if (f_is_int(type))
        return llvm::Instruction::Add;
      else
        return llvm::Instruction::FAdd;
    case FSub:
      if (f_is_int(type))
        return llvm::Instruction::Sub;
      else
        return llvm::Instruction::FSub;
    case FMul:
      if (f_is_int(type))
        return llvm::Instruction::Mul;
      else
        return llvm::Instruction::FMul;
    case FDiv:
      if (f_is_int(type))
        return llvm::Instruction::SDiv;
      else
        return llvm::Instruction::FDiv;
    case FRem: return llvm::Instruction::URem;
    case FShl: return llvm::Instruction::Shl;
    case FShr: return llvm::Instruction::LShr;
    case FAnd: return llvm::Instruction::And;
    case FOr:  return llvm::Instruction::Or;
    case FXor: return llvm::Instruction::Xor;
  }
  return llvm::Instruction::Add;
}

/* Declare a function */
void declare_function(ModuleState &ms, int function) {
  auto f = f_get_function(ms.irmodule, function);
  auto ftype = f_get_ftype_by_function(ms.irmodule, function);
  auto ret = convert_type(ftype->ret);
  std::vector<llvm::Type*> args;
  for (int i = 0; i < ftype->nargs; ++i)
    args.push_back(convert_type(ftype->args[i]));
  auto type = llvm::FunctionType::get(ret, args, ftype->vararg);
  llvm::Function *llvm_f;
  if (f->tag == FExtFunc) {
    std::stringstream name;
    name << "f" << f->u.ptr;
    auto addr = reinterpret_cast<void*>(f->u.ptr);
    llvm::sys::DynamicLibrary::AddSymbol(name.str(), addr);
    llvm_f = ms.module->getFunction(name.str());
    if (!llvm_f)
      llvm_f = llvm::Function::Create(type, llvm::Function::ExternalLinkage,
        name.str(), ms.module.get());
  } else {
    llvm_f = llvm::Function::Create(type, llvm::Function::ExternalLinkage,
      "f" + std::to_string(function), ms.module.get());
  }
  ms.functions.push_back(llvm_f);
}

/* Obtain a llvm value given the ir value */
llvm::Value *get_value(FunctionState &fs, FValue irvalue) {
  return fs.values[irvalue.bblock][irvalue.instr];
}

/* Convert an integer comparison */
llvm::CmpInst::Predicate convert_intcmp(enum FIntCmpTag op) {
  switch (op) {
    case FIntEq:  return llvm::CmpInst::ICMP_EQ;
    case FIntNe:  return llvm::CmpInst::ICMP_NE;
    case FIntSLe: return llvm::CmpInst::ICMP_SLE;
    case FIntSLt: return llvm::CmpInst::ICMP_SLT;
    case FIntSGe: return llvm::CmpInst::ICMP_SGE;
    case FIntSGt: return llvm::CmpInst::ICMP_SGT;
    case FIntULe: return llvm::CmpInst::ICMP_ULE;
    case FIntULt: return llvm::CmpInst::ICMP_ULT;
    case FIntUGe: return llvm::CmpInst::ICMP_UGE;
    case FIntUGt: return llvm::CmpInst::ICMP_UGT;
  }
  return llvm::CmpInst::ICMP_EQ;
}

/* Convert an float point comparison */
llvm::CmpInst::Predicate convert_fpcmp(enum FFpCmpTag op) {
  switch (op) {
    case FFpOEq: return llvm::CmpInst::FCMP_OEQ;
    case FFpONe: return llvm::CmpInst::FCMP_ONE;
    case FFpOLe: return llvm::CmpInst::FCMP_OLE;
    case FFpOLt: return llvm::CmpInst::FCMP_OLT;
    case FFpOGe: return llvm::CmpInst::FCMP_OGE;
    case FFpOGt: return llvm::CmpInst::FCMP_OGT;
    case FFpUEq: return llvm::CmpInst::FCMP_UEQ;
    case FFpUNe: return llvm::CmpInst::FCMP_UNE;
    case FFpULe: return llvm::CmpInst::FCMP_ULE;
    case FFpULt: return llvm::CmpInst::FCMP_ULT;
    case FFpUGe: return llvm::CmpInst::FCMP_UGE;
    case FFpUGt: return llvm::CmpInst::FCMP_UGT;
  }
  return llvm::CmpInst::FCMP_OEQ;
}

/* Compile a single instruction */
void compile_instruction(ModuleState &ms, FunctionState &fs, FValue irvalue) {
  auto function = ms.functions[fs.function];
  llvm::IRBuilder<> b(TheContext);
  b.SetInsertPoint(fs.bblocks[irvalue.bblock]);
  auto i = f_instr(ms.irmodule, fs.function, irvalue);
  auto &v = fs.values[irvalue.bblock][irvalue.instr];
  switch (i->tag) {
    case FKonst: {
      auto ktype = convert_type(i->type);
      if (i->type == FBool || f_is_int(i->type))
        v = llvm::ConstantInt::get(ktype, i->u.konst.i);
      else if (f_is_float(i->type))
        v = llvm::ConstantFP::get(ktype, i->u.konst.f);
      else {
        auto intptrt = llvm::IntegerType::get(TheContext, 8 * sizeof(void *));
        auto intptr = llvm::ConstantInt::get(intptrt,
          (uintptr_t)i->u.konst.p);
        v = b.CreateIntToPtr(intptr, convert_type(FPointer), "");
      }
      break;
    }
    case FGetarg: {
      int n = i->u.getarg.n;
      auto& args = function->getArgumentList();
      v = &*std::next(args.begin(), n);
      break;
    }
    case FLoad: {
      auto raw_addr = get_value(fs, i->u.load.addr);
      auto raw_addrtype = convert_type(i->type);
      auto addrtype = llvm::PointerType::get(raw_addrtype, 0);
      auto addr = b.CreateBitCast(raw_addr, addrtype, "");
      v = b.CreateLoad(addr);
      break;
    }
    case FStore: {
      auto raw_addr = get_value(fs, i->u.store.addr);
      auto val = get_value(fs, i->u.store.val);
      auto addrtype = llvm::PointerType::get(val->getType(), 0);
      auto addr = b.CreateBitCast(raw_addr, addrtype, "");
      v = b.CreateStore(val, addr);
      break;
    }
    case FOffset: {
      auto addr = get_value(fs, i->u.offset.addr);
      auto offset = get_value(fs, i->u.offset.offset);
      if (i->u.offset.negative)
        offset = b.CreateNeg(offset);
      v = b.CreateGEP(addr, offset);
      break;
    }
    case FCast: {
      auto val = get_value(fs, i->u.cast.val);
      auto t = convert_type(i->type);
      auto op = i->u.cast.op;
      switch (op) {
        case FUIntCast:
        case FSIntCast:
          v = b.CreateIntCast(val, t, op == FSIntCast);
          break;
        case FFloatCast:
          v = b.CreateFPCast(val, t);
          break;
        case FFloatToUInt:
          v = b.CreateFPToUI(val, t);
          break;
        case FFloatToSInt:
          v = b.CreateFPToSI(val, t);
          break;
        case FUIntToFloat:
          v = b.CreateUIToFP(val, t);
          break;
        case FSIntToFloat:
          v = b.CreateSIToFP(val, t);
          break;
      }
      break;
    }
    case FBinop: {
      auto lhs = get_value(fs, i->u.binop.lhs);
      auto rhs = get_value(fs, i->u.binop.rhs);
      auto op = convert_binop(i->u.binop.op, i->type);
      v = b.CreateBinOp(op, lhs, rhs);
      break;
    }
    case FIntCmp: {
      auto lhs = get_value(fs, i->u.intcmp.lhs);
      auto rhs = get_value(fs, i->u.intcmp.rhs);
      auto op = convert_intcmp(i->u.intcmp.op);
      v = b.CreateICmp(op, lhs, rhs);
      break;
    }
    case FFpCmp: {
      auto lhs = get_value(fs, i->u.fpcmp.lhs);
      auto rhs = get_value(fs, i->u.fpcmp.rhs);
      auto op = convert_fpcmp(i->u.fpcmp.op);
      v = b.CreateFCmp(op, lhs, rhs);
      break;
    }
    case FJmpIf: {
      auto cond = get_value(fs, i->u.jmpif.cond);
      auto truebr = fs.bblocks[i->u.jmpif.truebr];
      auto falsebr = fs.bblocks[i->u.jmpif.falsebr];
      v = b.CreateCondBr(cond, truebr, falsebr);
      break;
    }
    case FJmp: {
      auto dest = fs.bblocks[i->u.jmp.dest];
      v = b.CreateBr(dest);
      break;
    }
    case FSelect: {
      auto cond = get_value(fs, i->u.select.cond);
      auto truev = get_value(fs, i->u.select.truev);
      auto falsev = get_value(fs, i->u.select.falsev);
      v = b.CreateSelect(cond, truev, falsev);
      break;
    }
    case FRet: {
      auto retv = i->u.ret.val;
      if(f_null(retv)) {
        v = b.CreateRetVoid();
      }
      else {
        v = b.CreateRet(get_value(fs, retv));
      }
      break;
    }
    case FCall: {
      std::vector<llvm::Value*> args;
      for(int a = 0; a < i->u.call.nargs; ++a) {
        args.push_back(get_value(fs, i->u.call.args[a]));
      }
      v = b.CreateCall(ms.functions[i->u.call.function], args);
      break;
    }
    case FPhi: {
      auto type = convert_type(i->type);
      v = b.CreatePHI(type, vec_size(i->u.phi.inc));
      break;
    }
  }
}

/* Link the phi values in the function */
void link_phi_values(ModuleState &ms, FunctionState &fs) {
  auto f = f_get_function(ms.irmodule, fs.function);
  vec_for(f->u.bblocks, b, {
    auto bblock = f_get_bblock(ms.irmodule, fs.function, b);
    vec_for(*bblock, i, {
      auto instr = f_instr(ms.irmodule, fs.function, f_value(b, i));
      if (instr->tag == FPhi) {
        auto v = static_cast<llvm::PHINode*>(fs.values[b][i]);
        vec_foreach(instr->u.phi.inc, inc, {
          auto inc_value = fs.values[inc->value.bblock][inc->value.instr];
          auto inc_bb = fs.bblocks[inc->bb];
          v->addIncoming(inc_value, inc_bb);
        });
      }
    });
  });
}

/* Compile a function */
void compile_function(ModuleState &ms, int function) {
  FunctionState fs{function};
  auto f = f_get_function(ms.irmodule, function);
  if (f->tag != FModFunc) return;
  /* Create basic the blocks */
  fs.bblocks.reserve(vec_size(f->u.bblocks));
  vec_for(f->u.bblocks, bb, {
    fs.bblocks.push_back(
      llvm::BasicBlock::Create(TheContext, "", ms.functions[function]));
  });
  /* Compile the instructions */
  fs.values.resize(fs.bblocks.size());
  vec_for(f->u.bblocks, b, {
    auto bblock = f_get_bblock(ms.irmodule, function, b);
    fs.values[b].resize(vec_size(*bblock), nullptr);
    vec_for(*bblock, i, {
      compile_instruction(ms, fs, f_value(b, i));
    });
  });
  link_phi_values(ms, fs);
}

}

void f_init_engine(FEngine *e) {
  e->data = nullptr;
  e->nfuncs = 0;
  e->funcs = nullptr;
}

void f_close_engine(FEngine *e) {
  auto data = reinterpret_cast<FEngineData *>(e->data);
  if (data) data->ee.reset(nullptr);
  f_init_engine(e);
}

int f_compile(FEngine *e, FModule *m) {
  /* llvm initialization */
  static bool init = true;
  if (init) {
    llvm::InitializeNativeTarget();
    llvm::InitializeNativeTargetAsmPrinter();
    llvm::InitializeNativeTargetAsmParser();
    init = false;
  }
  /* Generate IR */
  std::unique_ptr<FEngineData> data(new FEngineData());
  ModuleState ms(*data, m);
  vec_for(m->functions, i, declare_function(ms, i));
  vec_for(m->functions, i, compile_function(ms, i));
  /* Verify */
  std::string error;
  llvm::raw_string_ostream error_os(error);
  if (llvm::verifyModule(*ms.module, &error_os)) {
    fprintf(stderr, "%s\n", error.c_str());
    ms.module->dump();
    return 1;
  }
  /* Compile */
  data->ee.reset(llvm::EngineBuilder(std::move(ms.module))
    .setErrorStr(&error)
    .setOptLevel(llvm::CodeGenOpt::Aggressive)
    .setEngineKind(llvm::EngineKind::JIT)
    .create());
  if (!data->ee) {
    fprintf(stderr, "%s\n", error.c_str());
    return 1;
  }
  data->ee->finalizeObject();
  /* Get functions */
  data->functions.resize(ms.functions.size());
  vec_for(m->functions, i, {
    auto f = data->ee->getPointerToFunction(ms.functions[i]);
    data->functions[i] = reinterpret_cast<FJitFunc>(f);
  });
  /* Return */
  e->funcs = data->functions.data();
  e->nfuncs = data->functions.size();
  e->data = data.release();
  return 0;
}

