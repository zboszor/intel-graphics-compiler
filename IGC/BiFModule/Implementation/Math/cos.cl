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

#include "../include/BiF_Definitions.cl"
#include "../../Headers/spirv.h"
#include "../ExternalLibraries/libclc/trig.cl"
#include "../IMF/FP32/cos_s_la.cl"
#include "../IMF/FP32/cos_s_noLUT.cl"

static INLINE float __intel_cos_f32( float x, bool doFast )
{
    if(__FastRelaxedMath && (!__APIRS) && doFast)
    {
        return __builtin_spirv_OpenCL_native_cos_f32(x);
    }
    else
    {
        if(__UseMathWithLUT)
        {
            return __ocl_svml_cosf(x);
        }
        else
        {
            float abs_float = __builtin_spirv_OpenCL_fabs_f32(x);
            if( abs_float > 10000.0f )
            {
                return libclc_cos_f32(x);
            }
            else
            {
                return __ocl_svml_cosf_noLUT(x);
            }
        }
    }
}

INLINE float __builtin_spirv_OpenCL_cos_f32( float x )
{
    return __intel_cos_f32(x, true);
}

GENERATE_VECTOR_FUNCTIONS_1ARG_LOOP( __builtin_spirv_OpenCL_cos, float, float, f32 )

#if defined(cl_khr_fp16)

INLINE half __builtin_spirv_OpenCL_cos_f16( half x )
{
    return (half)__builtin_spirv_OpenCL_cos_f32((float)x);
}

GENERATE_VECTOR_FUNCTIONS_1ARG_LOOP( __builtin_spirv_OpenCL_cos, half, half, f16 )

#endif // defined(cl_khr_fp16)
