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
llvm::Type *convert_type(ModuleState &ms, enum FType type) {
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

/* Declare a function */
void declare_function(ModuleState &ms, int function) {
  auto ftype = f_get_ftype_by_function(ms.irmodule, function);
  auto ret = convert_type(ms, ftype->ret);
  std::vector<llvm::Type*> args;
  for (int i = 0; i < ftype->nargs; ++i)
    args.push_back(convert_type(ms, ftype->args[i]));
  auto type = llvm::FunctionType::get(ret, args, false);
  auto f = llvm::Function::Create(type, llvm::Function::ExternalLinkage,
    "f" + std::to_string(function), ms.module.get());
  ms.functions.push_back(f);
}

/* Obtain a llvm value given the ir value */
llvm::Value *get_value(FunctionState &fs, FValue irvalue) {
  return fs.values[irvalue.bblock][irvalue.instr];
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
      auto ktype = convert_type(ms, i->type);
      if(f_is_int(i->type)) {
        v = llvm::ConstantInt::get(ktype, i->u.konst.i);
      }
      else if(f_is_float(i->type)) {
        v = llvm::ConstantFP::get(ktype, i->u.konst.f);
      }
      else {
        auto intptrt = llvm::IntegerType::get(TheContext, 8 * sizeof(void *));
        auto intptr = llvm::ConstantInt::get(intptrt,
          (uintptr_t)i->u.konst.p);
        v = b.CreateIntToPtr(intptr, convert_type(ms, FPointer), "");
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
      auto addr = get_value(fs, i->u.load.addr);
      auto addrtype = convert_type(ms, i->type);
      addrtype = llvm::PointerType::get(addrtype, 0);
      addr = b.CreateBitCast(addr, addrtype, "");
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
      break;
    }
    case FBinop: {
      break;
    }
    case FCmp: {
      break;
    }
    case FJmpIf: {
      break;
    }
    case FJmp: {
      break;
    }
    case FSelect: {
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
      break;
    }
    case FPhi: {
      break;
    }
  }
}

/* Compile a function */
void compile_function(ModuleState &ms, int function) {
  FunctionState fs{function};
  auto f = f_get_function(ms.irmodule, function);
  /* TODO check if func is external */
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

