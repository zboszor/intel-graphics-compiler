/*========================== begin_copyright_notice ============================

Copyright (c) 2000-2021 Intel Corporation

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"),
to deal in the Software without restriction, including without limitation
the rights to use, copy, modify, merge, publish, distribute, sublicense,
and/or sell copies of the Software, and to permit persons to whom
the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included
in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
IN THE SOFTWARE.

============================= end_copyright_notice ===========================*/

#ifndef __GEN_UPDATE_CB__
#define __GEN_UPDATE_CB__

#include "Compiler/CodeGenPublic.h"
#include "common/LLVMWarningsPush.hpp"
#include <llvm/IR/Function.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/Intrinsics.h>
#include <llvm/IR/InstIterator.h>
#include <llvm/ADT/SetVector.h>
#include <llvm/Transforms/Utils/Local.h>
#include <llvm/Transforms/Utils/BasicBlockUtils.h>
#include <llvm/Transforms/Utils/ValueMapper.h>
#include "common/LLVMWarningsPop.hpp"

void initializeGenUpdateCBPass(llvm::PassRegistry&);

namespace IGC
{

    class GenUpdateCB : public llvm::FunctionPass
    {
    public:
        static char ID;
        GenUpdateCB() : llvm::FunctionPass(ID)
        {
            initializeGenUpdateCBPass(*llvm::PassRegistry::getPassRegistry());
        }
        virtual llvm::StringRef getPassName() const { return "GenUpdateCB"; }
        virtual bool runOnFunction(llvm::Function& F);
        virtual void getAnalysisUsage(llvm::AnalysisUsage& AU) const
        {
            AU.setPreservesCFG();
            AU.addRequired<CodeGenContextWrapper>();
            AU.addRequired<llvm::DominatorTreeWrapperPass>();
        }
    private:
        bool isConstantBufferLoad(llvm::LoadInst* inst, unsigned& bufId);

        bool allSrcConstantOrImm(llvm::Instruction* inst);
        bool updateCbAllowedInst(llvm::Instruction* inst);
        void InsertInstTree(llvm::Instruction* inst, llvm::Instruction* pos);
        llvm::Instruction* CreateModule(llvm::Module* newModule);

        const unsigned FLAG_LOAD = 1;
        const unsigned FLAG_RESINFO = 2;

        CodeGenContext* m_ctx;

        // For each instruction in m_CbUpadteMap, we record whether it's
        // coming from a load or resinfo.
        llvm::DenseMap<llvm::Value*, unsigned> m_CbUpdateMap;
        llvm::ValueToValueMapTy vmap;
        llvm::Module* m_ConstantBufferReplaceShaderPatterns = nullptr;
        uint m_ConstantBufferReplaceShaderPatternsSize = 0;
        uint m_ConstantBufferUsageMask = 0;
        uint m_maxCBcases = 128;
    };
}

#endif
