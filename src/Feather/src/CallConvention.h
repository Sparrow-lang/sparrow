#pragma once

namespace Feather
{
    /// The possible calling conventions
    enum CallConvention
    {
        ccC,        ///< The default llvm calling convention, compatible with C
        ccFast,     ///< This calling convention attempts to make calls as fast as possible (e.g. by passing things in registers)
        ccCold,     ///< Makes the caller as efficient as possible, under the assumption that the call is not commonly executed
        ccGHC,      ///< Calling convention used by the Glasgow Haskell Compiler (GHC) - only registers

        ccX86Std,       ///< stdcall is the calling conventions mostly used by the Win32 API
        ccX86Fast,      ///< 'fast' analog of ccX86Std
        ccX86ThisCall,  ///< Similar to X86 StdCall. Passes first argument in ECX, others via stack
        ccARMAPCS,      ///< ARM Procedure Calling Standard calling convention
        ccARMAAPCS,     ///< ARM Architecture Procedure Calling Standard calling convention (aka EABI)
        ccARMAAPCSVFP,  ///< Same as ARM_AAPCS, but uses hard floating point ABI
        ccMSP430Intr,   ///< Calling convention used for MSP430 interrupt routines
        ccPTXKernel,    ///< Call to a PTX kernel. Passes all arguments in parameter space
        ccPTXDevice,    ///< Call to a PTX device function. Passes all arguments in register or parameter space
        ccMBLAZEIntr,   ///< Calling convention used for MBlaze interrupt routines
        ccMBLAZESVol,   ///< Calling convention used for MBlaze interrupt support routines
    };
}
