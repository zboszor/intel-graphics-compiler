/*===================== begin_copyright_notice ==================================

Copyright (c) 2017 Intel Corporation

Permission is hereby granted, free of charge, to any person obtaining a
copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be included
in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.


======================= end_copyright_notice ==================================*/

#include "Compiler/Optimizer/OpenCLPasses/ImageFuncs/ImageFuncResolution.hpp"
#include "Compiler/Optimizer/OpenCLPasses/ImageFuncs/ImageFuncsAnalysis.hpp"
#include "Compiler/Optimizer/OCLBIUtils.h"
#include "Compiler/IGCPassSupport.h"

#include "common/LLVMWarningsPush.hpp"
#include <llvmWrapper/IR/Function.h>
#include <llvm/IR/Instructions.h>
#include "common/LLVMWarningsPop.hpp"

using namespace llvm;
using namespace IGC;
using namespace IGC::IGCMD;

// Register pass to igc-opt
#define PASS_FLAG "igc-image-func-resolution"
#define PASS_DESCRIPTION "Resolves image height, width, depth functions"
#define PASS_CFG_ONLY false
#define PASS_ANALYSIS false
IGC_INITIALIZE_PASS_BEGIN(ImageFuncResolution, PASS_FLAG, PASS_DESCRIPTION, PASS_CFG_ONLY, PASS_ANALYSIS)
IGC_INITIALIZE_PASS_DEPENDENCY(MetaDataUtilsWrapper)
IGC_INITIALIZE_PASS_END(ImageFuncResolution, PASS_FLAG, PASS_DESCRIPTION, PASS_CFG_ONLY, PASS_ANALYSIS)

char ImageFuncResolution::ID = 0;

ImageFuncResolution::ImageFuncResolution() : FunctionPass(ID), m_implicitArgs()
{
    initializeImageFuncResolutionPass(*PassRegistry::getPassRegistry());
}

bool ImageFuncResolution::runOnFunction(Function &F) {
    const MetaDataUtils* pMdUtils = getAnalysis<MetaDataUtilsWrapper>().getMetaDataUtils();
    m_implicitArgs = ImplicitArgs(F, pMdUtils);
    m_InputIRVersionMD = pMdUtils->getInputIRVersionsItem(0);
    m_changed = false;
    visit(F);
    return m_changed;
}

void ImageFuncResolution::visitCallInst(CallInst &CI)
{
    if (!CI.getCalledFunction())
    {
        return;
    }

    Value* imageRes = nullptr;
    
    // Add appropriate sequence and image dimension func
    StringRef funcName = CI.getCalledFunction()->getName();

    if( funcName.equals(ImageFuncsAnalysis::GET_IMAGE_HEIGHT) )
    {
        imageRes = getImageHeight(CI);
    }
    else if( funcName.equals(ImageFuncsAnalysis::GET_IMAGE_WIDTH) )
    {
        imageRes = getImageWidth(CI);
    }
    else if( funcName.equals(ImageFuncsAnalysis::GET_IMAGE_DEPTH) )
    {
        imageRes = getImageDepth(CI);
    }
    else if (funcName.equals(ImageFuncsAnalysis::GET_IMAGE_NUM_MIP_LEVELS))
    {
        imageRes = getImageNumMipLevels(CI);
    }
    else if( funcName.equals(ImageFuncsAnalysis::GET_IMAGE_CHANNEL_DATA_TYPE) )
    {
        imageRes = getImageChannelDataType(CI);
    }
    else if( funcName.equals(ImageFuncsAnalysis::GET_IMAGE_CHANNEL_ORDER) )
    {
        imageRes = getImageChannelOrder(CI);
    }
    else if( funcName.equals(ImageFuncsAnalysis::GET_IMAGE_SRGB_CHANNEL_ORDER) )
    {
        imageRes = getImplicitImageArg(CI, ImplicitArg::IMAGE_SRGB_CHANNEL_ORDER);
    }
    else if( funcName.equals(ImageFuncsAnalysis::GET_IMAGE_ARRAY_SIZE) )
    {
        imageRes = getImageArraySize(CI);
    }
    else if (funcName.equals(ImageFuncsAnalysis::GET_IMAGE_NUM_SAMPLES))
    {
        imageRes = getImageNumSamples(CI);
    }
    else if( funcName.equals(ImageFuncsAnalysis::GET_SAMPLER_ADDRESS_MODE) )
    {
        imageRes = getSamplerAddressMode(CI);
    }
    else if( funcName.equals(ImageFuncsAnalysis::GET_SAMPLER_NORMALIZED_COORDS) )
    {
        imageRes = getSamplerNormalizedCoords(CI);
    }
    else if( funcName.equals(ImageFuncsAnalysis::GET_SAMPLER_SNAP_WA_REQUIRED) )
    {
        imageRes = getSamplerSnapWARequired(CI);
    }
    else 
    {
        // Non image function, do nothing
        return;
    }

    // Replace original image dim call instruction by the result of the appropriate sequence
    CI.replaceAllUsesWith(imageRes);
    CI.eraseFromParent();
    m_changed = true;
}

Value* ImageFuncResolution::getImageHeight(CallInst &CI) 
{
    Argument* arg = getImplicitImageArg(CI, ImplicitArg::IMAGE_HEIGHT);
    return arg;
}

Value* ImageFuncResolution::getImageWidth(CallInst &CI) 
{
    Argument* arg = getImplicitImageArg(CI, ImplicitArg::IMAGE_WIDTH);
    return arg;
}

Value* ImageFuncResolution::getImageDepth(CallInst &CI) 
{
    Argument* arg = getImplicitImageArg(CI, ImplicitArg::IMAGE_DEPTH);
    return arg;
}

Value* ImageFuncResolution::getImageNumMipLevels(CallInst &CI) 
{
    Argument* arg = getImplicitImageArg(CI, ImplicitArg::IMAGE_NUM_MIP_LEVELS);
    return arg;
}

Value* ImageFuncResolution::getImageChannelDataType(CallInst &CI) 
{
    Argument* arg = getImplicitImageArg(CI, ImplicitArg::IMAGE_CHANNEL_DATA_TYPE);
    return arg;
}

Value* ImageFuncResolution::getImageChannelOrder(CallInst &CI) 
{
    Argument* arg = getImplicitImageArg(CI, ImplicitArg::IMAGE_CHANNEL_ORDER);
    return arg;
}

Value* ImageFuncResolution::getImageArraySize(CallInst &CI) 
{
    Argument* arg = getImplicitImageArg(CI, ImplicitArg::IMAGE_ARRAY_SIZE);
    return arg;
}

Value* ImageFuncResolution::getImageNumSamples(CallInst &CI)
{
    Argument* arg = getImplicitImageArg(CI, ImplicitArg::IMAGE_NUM_SAMPLES);
    return arg;
}

Value* ImageFuncResolution::getSamplerAddressMode(CallInst &CI)
{
    MetaDataUtils* pMdUtils = getAnalysis<MetaDataUtilsWrapper>().getMetaDataUtils();
    Value* sampler = CImagesBI::CImagesUtils::traceImageOrSamplerArgument(&CI, 0, pMdUtils);
    assert(sampler != nullptr && "Sampler untraceable for ImplicitArg::SAMPLER_ADDRESS");
    if (isa<Argument>(sampler))
    {
        Argument* arg = getImplicitImageArg(CI, ImplicitArg::SAMPLER_ADDRESS);
        return arg;
    }
    else
    {
        llvm::Function* pFunc = CI.getParent()->getParent();
        FunctionInfoMetaDataHandle funcInfoMD = pMdUtils->getFunctionsInfoItem(pFunc);
        ResourceAllocMetaDataHandle resAllocMD = funcInfoMD->getResourceAlloc();

        assert(isa<ConstantInt>(sampler) && "Sampler must be a constant integer");
        InlineSamplerState samplerStateAddressMode{ cast<ConstantInt>(sampler)->getZExtValue() };
        uint64_t samplerVal = 0;
        uint samplerValue = int_cast<unsigned int>(cast<ConstantInt>(sampler)->getZExtValue());
        for (auto i = resAllocMD->begin_InlineSamplers(), e = resAllocMD->end_InlineSamplers(); i != e; ++i){
            InlineSamplerMetaDataHandle inlineSamplerMD = *i;
            if (samplerValue == inlineSamplerMD->getValue())
            {
                InlineSamplerState samplerState{ static_cast<uint64_t>(samplerValue) };
                samplerVal = inlineSamplerMD->getAddressMode();
            }
        }

        return ConstantInt::get(CI.getType(), samplerVal);
    }
}

Value* ImageFuncResolution::getSamplerNormalizedCoords(CallInst &CI)
{
    MetaDataUtils* pMdUtils = getAnalysis<MetaDataUtilsWrapper>().getMetaDataUtils();
    Value* sampler = CImagesBI::CImagesUtils::traceImageOrSamplerArgument(&CI, 0, pMdUtils);
    if (sampler == nullptr)
    {
        // TODO: For now disable WA if unable to trace sampler argument. 
        // Will need to rework WA to add support for indirect sampler case.
        return ConstantInt::get(CI.getType(), 0);
    }
    else if (isa<Argument>(sampler))
    {
        Argument* arg = getImplicitImageArg(CI, ImplicitArg::SAMPLER_NORMALIZED);
        return arg;
    }
    else
    {
        llvm::Function* pFunc = CI.getParent()->getParent();
        FunctionInfoMetaDataHandle funcInfoMD = pMdUtils->getFunctionsInfoItem(pFunc);
        ResourceAllocMetaDataHandle resAllocMD = funcInfoMD->getResourceAlloc();

        assert(isa<ConstantInt>(sampler) && "Sampler must be a constant integer");

        uint64_t samplerVal = 0;
        uint samplerValue = int_cast<unsigned int>(cast<ConstantInt>(sampler)->getZExtValue());
        for (auto i = resAllocMD->begin_InlineSamplers(), e = resAllocMD->end_InlineSamplers(); i != e; ++i){
            InlineSamplerMetaDataHandle inlineSamplerMD = *i;
            if (samplerValue == inlineSamplerMD->getValue())
            {
                InlineSamplerState samplerState{ static_cast<uint64_t>(samplerValue) };
                samplerVal = inlineSamplerMD->getNormalizedCoords();
            }
        }

        return ConstantInt::get(CI.getType(), samplerVal);
    }
}

Value* ImageFuncResolution::getSamplerSnapWARequired(CallInst &CI) 
{
    MetaDataUtils* pMdUtils = getAnalysis<MetaDataUtilsWrapper>().getMetaDataUtils();
    Value* sampler = CImagesBI::CImagesUtils::traceImageOrSamplerArgument(&CI, 0, pMdUtils);
    if (sampler == nullptr)
    {
        // TODO: For now disable WA if unable to trace sampler argument. 
        // Will need to rework WA to add support for indirect sampler case.
        return ConstantInt::get(CI.getType(), 0);
    }
    else if (isa<Argument>(sampler))
    {
        Argument* arg = getImplicitImageArg(CI, ImplicitArg::SAMPLER_SNAP_WA);
        return arg;
    }
    else
    {
        assert(isa<ConstantInt>(sampler) && "Sampler must be a constant integer");

        llvm::Function* pFunc = CI.getParent()->getParent();
        FunctionInfoMetaDataHandle funcInfoMD = pMdUtils->getFunctionsInfoItem(pFunc);
        ResourceAllocMetaDataHandle resAllocMD = funcInfoMD->getResourceAlloc();
        bool snapWARequired = false;
        uint samplerVal = int_cast<unsigned int>(cast<ConstantInt>(sampler)->getZExtValue());
        for (auto i = resAllocMD->begin_InlineSamplers(), e = resAllocMD->end_InlineSamplers(); i != e; ++i){
            InlineSamplerMetaDataHandle inlineSamplerMD = *i;
            if (samplerVal == inlineSamplerMD->getValue())
            {
                InlineSamplerState samplerState{ static_cast<uint64_t>(samplerVal) };
                bool anyAddressModeClamp = 
                    inlineSamplerMD->getTCXAddressMode() == iOpenCL::SAMPLER_TEXTURE_ADDRESS_MODE_BORDER ||
                    inlineSamplerMD->getTCYAddressMode() == iOpenCL::SAMPLER_TEXTURE_ADDRESS_MODE_BORDER ||
                    inlineSamplerMD->getTCZAddressMode() == iOpenCL::SAMPLER_TEXTURE_ADDRESS_MODE_BORDER;
                bool anyMapFilterModeNearest = 
                    inlineSamplerMD->getMagFilterType() == iOpenCL::SAMPLER_MAPFILTER_POINT ||
                    inlineSamplerMD->getMinFilterType() == iOpenCL::SAMPLER_MAPFILTER_POINT;
                snapWARequired = anyAddressModeClamp && 
                                 anyMapFilterModeNearest && 
                                 !inlineSamplerMD->getNormalizedCoords();
            }
        }

        return ConstantInt::get(CI.getType(), snapWARequired ? -1 : 0);
    }
}

Argument* ImageFuncResolution::getImplicitImageArg(CallInst &CI, ImplicitArg::ArgType argType) {
    // Only images that are arguments are supported!
    Argument* image = cast<Argument>(CImagesBI::CImagesUtils::traceImageOrSamplerArgument(&CI, 0));

    unsigned int numImplicitArgs = m_implicitArgs.size();
    unsigned int implicitArgIndex = m_implicitArgs.getImageArgIndex(argType, image);

    Function* pFunc = CI.getParent()->getParent();
    unsigned int implicitArgIndexInFunc = IGCLLVM::GetFuncArgSize(pFunc) - numImplicitArgs + implicitArgIndex;
    
    Function::arg_iterator arg = pFunc->arg_begin();
    for (unsigned int i = 0; i < implicitArgIndexInFunc; ++i,  ++arg);

    return &(*arg);
}
