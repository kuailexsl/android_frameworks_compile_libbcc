/*
 * Copyright 2010-2012, The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef BCC_COMPILER_H
#define BCC_COMPILER_H

namespace llvm {

class raw_ostream;
class DataLayout;
class TargetMachine;

namespace legacy {
class PassManager;
} // end namespace legacy

using legacy::PassManager;

} // end namespace llvm

namespace bcc {

class CompilerConfig;
class OutputFile;
class Script;

//===----------------------------------------------------------------------===//
// Design of Compiler
//===----------------------------------------------------------------------===//
// 1. A compiler instance can be constructed provided an "initial config."
// 2. A compiler can later be re-configured using config().
// 3. Once config() is invoked, it'll re-create TargetMachine instance (i.e.,
//    mTarget) according to the configuration supplied. TargetMachine instance
//    is *shared* across the different calls to compile() before the next call
//    to config().
// 4. Once a compiler instance is created, you can use the compile() service
//    to compile the file over and over again. Each call uses TargetMachine
//    instance to construct the compilation passes.
class Compiler {
public:
  enum ErrorCode {
    kSuccess,

    kInvalidConfigNoTarget,
    kErrCreateTargetMachine,
    kErrSwitchTargetMachine,
    kErrNoTargetMachine,
    kErrMaterialization,
    kErrInvalidOutputFileState,
    kErrPrepareOutput,
    kPrepareCodeGenPass,

    kErrCustomPasses,

    kErrInvalidSource,

    kIllegalGlobalFunction
  };

  static const char *GetErrorString(enum ErrorCode pErrCode);

private:
  llvm::TargetMachine *mTarget;
  // Optimization is enabled by default.
  bool mEnableOpt;

  enum ErrorCode runPasses(Script &pScript, llvm::raw_ostream &pResult);
  enum ErrorCode screenGlobals(Script &pScript);

  bool addCustomPasses(Script &pScript, llvm::legacy::PassManager &pPM);
  bool addInternalizeSymbolsPass(Script &pScript, llvm::legacy::PassManager &pPM);
  bool addExpandForEachPass(Script &pScript, llvm::legacy::PassManager &pPM);
  bool addInvariantPass(llvm::legacy::PassManager &pPM);
  bool addInvokeHelperPass(llvm::legacy::PassManager &pPM);
  bool addPostLTOCustomPasses(llvm::legacy::PassManager &pPM);

public:
  Compiler();
  Compiler(const CompilerConfig &pConfig);

  enum ErrorCode config(const CompilerConfig &pConfig);

  // Compile a script and output the result to a LLVM stream.
  //
  // @param IRStream If not NULL, the LLVM-IR that is fed to code generation
  //                 will be written to IRStream.
  enum ErrorCode compile(Script &pScript, llvm::raw_ostream &pResult,
                         llvm::raw_ostream *IRStream);

  // Compile a script and output the result to a file.
  enum ErrorCode compile(Script &pScript, OutputFile &pResult,
                         llvm::raw_ostream *IRStream = 0);

  const llvm::TargetMachine& getTargetMachine() const
  { return *mTarget; }

  void enableOpt(bool pEnable = true)
  { mEnableOpt = pEnable; }

  ~Compiler();
};

} // end namespace bcc

#endif // BCC_COMPILER_H
