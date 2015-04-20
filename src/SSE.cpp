/// Emil Hedemalm
/// 2015-01-25
/// Re-defines macros and even new and delete if activated? D:


#include "SSE.h"

#include <cstdlib> // for malloc() and free()
#include <exception> // for std::bad_alloc

// Visual C++ fix of operator new
#ifdef WINDOWS
void* operator new (size_t size)
{
	void *p = _aligned_malloc(size, 16); 
	if (p==0) // did malloc succeed?
		throw std::bad_alloc(); // ANSI/ISO compliant behavior
	return p;
}

void operator delete(void * p) throw()
{
	_aligned_free(p);
}


void *operator new[](std::size_t size) throw(std::bad_alloc)
{
	void *p = _aligned_malloc(size, 16); 
	if (p==0) // did malloc succeed?
		throw std::bad_alloc(); // ANSI/ISO compliant behavior
	return p;
}
void operator delete[](void *p) throw()
{
	_aligned_free(p);
}
#endif

#include <new>

#include <xmmintrin.h>
//#include <intrin.h>

#include <iostream>

#include "MathLib.h"
#include "Timer/Timer.h"

const char* szFeatures[] =
{
    "x87 FPU On Chip",
    "Virtual-8086 Mode Enhancement",
    "Debugging Extensions",
    "Page Size Extensions",
    "Time Stamp Counter",
    "RDMSR and WRMSR Support",
    "Physical Address Extensions",
    "Machine Check Exception",
    "CMPXCHG8B Instruction",
    "APIC On Chip",
    "Unknown1",
    "SYSENTER and SYSEXIT",
    "Memory Type Range Registers",
    "PTE Global Bit",
    "Machine Check Architecture",
    "Conditional Move/Compare Instruction",
    "Page Attribute Table",
    "36-bit Page Size Extension",
    "Processor Serial Number",
    "CFLUSH Extension",
    "Unknown2",
    "Debug Store",
    "Thermal Monitor and Clock Ctrl",
    "MMX Technology",
    "FXSAVE/FXRSTOR",
    "SSE Extensions",
    "SSE2 Extensions",
    "Self Snoop",
    "Multithreading Technology",
    "Thermal Monitor",
    "Unknown4",
    "Pend. Brk. EN."
};

// http://www.codeproject.com/Articles/4522/Introduction-to-SSE-Programming
// https://msdn.microsoft.com/de-de/library/hskdteyh%28v=vs.90%29.aspx
// https://msdn.microsoft.com/en-us/library/hskdteyh.aspx
void SIMDTest()
{
	char CPUString[0x20];
    char CPUBrandString[0x40];
    int CPUInfo[4] = {-1};
    int nSteppingID = 0;
    int nModel = 0;
    int nFamily = 0;
    int nProcessorType = 0;
    int nExtendedmodel = 0;
    int nExtendedfamily = 0;
    int nBrandIndex = 0;
    int nCLFLUSHcachelinesize = 0;
    int nLogicalProcessors = 0;
    int nAPICPhysicalID = 0;
    int nFeatureInfo = 0;
    int nCacheLineSize = 0;
    int nL2Associativity = 0;
    int nCacheSizeK = 0;
    int nPhysicalAddress = 0;
    int nVirtualAddress = 0;
    int nRet = 0;
    unsigned    nIds, nExIds, i;

    bool    bSSE3Instructions = false;
    bool    bMONITOR_MWAIT = false;
    bool    bCPLQualifiedDebugStore = false;
    bool    bVirtualMachineExtensions = false;
    bool    bEnhancedIntelSpeedStepTechnology = false;
    bool    bThermalMonitor2 = false;
    bool    bSupplementalSSE3 = false;
    bool    bL1ContextID = false;
    bool    bCMPXCHG16B = false;
    bool    bxTPRUpdateControl = false;
    bool    bPerfDebugCapabilityMSR = false;
    bool    bSSE41Extensions = false;
    bool    bSSE42Extensions = false;
    bool    bPOPCNT = false;

    bool    bMultithreading = false;

    bool    bLAHF_SAHFAvailable = false;
    bool    bCmpLegacy = false;
    bool    bSVM = false;
    bool    bExtApicSpace = false;
    bool    bAltMovCr8 = false;
    bool    bLZCNT = false;
    bool    bSSE4A = false;
    bool    bMisalignedSSE = false;
    bool    bPREFETCH = false;
    bool    bSKINITandDEV = false;
    bool    bSYSCALL_SYSRETAvailable = false;
    bool    bExecuteDisableBitAvailable = false;
    bool    bMMXExtensions = false;
    bool    bFFXSR = false;
    bool    b1GBSupport = false;
    bool    bRDTSCP = false;
    bool    b64Available = false;
    bool    b3DNowExt = false;
    bool    b3DNow = false;
    bool    bNestedPaging = false;
    bool    bLBRVisualization = false;
    bool    bFP128 = false;
    bool    bMOVOptimization = false;

#ifdef WEWEWINDOWS & 0

	// __cpuid with an InfoType argument of 0 returns the number of
    // valid Ids in CPUInfo[0] and the CPU identification string in
    // the other three array elements. The CPU identification string is
    // not in linear order. The code below arranges the information 
    // in a human readable form.
    __cpuid(CPUInfo, 0);
	nIds = CPUInfo[0];
    memset(CPUString, 0, sizeof(CPUString));
    *((int*)CPUString) = CPUInfo[1];
    *((int*)(CPUString+4)) = CPUInfo[3];
    *((int*)(CPUString+8)) = CPUInfo[2];

 // Get the information associated with each valid Id
    for (i=0; i<=nIds; ++i)
    {
        __cpuid(CPUInfo, i);
        printf_s("\nFor InfoType %d\n", i); 
        printf_s("CPUInfo[0] = 0x%x\n", CPUInfo[0]);
        printf_s("CPUInfo[1] = 0x%x\n", CPUInfo[1]);
        printf_s("CPUInfo[2] = 0x%x\n", CPUInfo[2]);
        printf_s("CPUInfo[3] = 0x%x\n", CPUInfo[3]);

        // Interpret CPU feature information.
        if  (i == 1)
        {
            nSteppingID = CPUInfo[0] & 0xf;
            nModel = (CPUInfo[0] >> 4) & 0xf;
            nFamily = (CPUInfo[0] >> 8) & 0xf;
            nProcessorType = (CPUInfo[0] >> 12) & 0x3;
            nExtendedmodel = (CPUInfo[0] >> 16) & 0xf;
            nExtendedfamily = (CPUInfo[0] >> 20) & 0xff;
            nBrandIndex = CPUInfo[1] & 0xff;
            nCLFLUSHcachelinesize = ((CPUInfo[1] >> 8) & 0xff) * 8;
            nLogicalProcessors = ((CPUInfo[1] >> 16) & 0xff);
            nAPICPhysicalID = (CPUInfo[1] >> 24) & 0xff;
            bSSE3Instructions = (CPUInfo[2] & 0x1) || false;
            bMONITOR_MWAIT = (CPUInfo[2] & 0x8) || false;
            bCPLQualifiedDebugStore = (CPUInfo[2] & 0x10) || false;
            bVirtualMachineExtensions = (CPUInfo[2] & 0x20) || false;
            bEnhancedIntelSpeedStepTechnology = (CPUInfo[2] & 0x80) || false;
            bThermalMonitor2 = (CPUInfo[2] & 0x100) || false;
            bSupplementalSSE3 = (CPUInfo[2] & 0x200) || false;
            bL1ContextID = (CPUInfo[2] & 0x300) || false;
            bCMPXCHG16B= (CPUInfo[2] & 0x2000) || false;
            bxTPRUpdateControl = (CPUInfo[2] & 0x4000) || false;
            bPerfDebugCapabilityMSR = (CPUInfo[2] & 0x8000) || false;
            bSSE41Extensions = (CPUInfo[2] & 0x80000) || false;
            bSSE42Extensions = (CPUInfo[2] & 0x100000) || false;
            bPOPCNT= (CPUInfo[2] & 0x800000) || false;
            nFeatureInfo = CPUInfo[3];
            bMultithreading = (nFeatureInfo & (1 << 28)) || false;
        }
    }

    // Calling __cpuid with 0x80000000 as the InfoType argument
    // gets the number of valid extended IDs.
    __cpuid(CPUInfo, 0x80000000);
    nExIds = CPUInfo[0];
    memset(CPUBrandString, 0, sizeof(CPUBrandString));

    // Get the information associated with each extended ID.
    for (i=0x80000000; i<=nExIds; ++i)
    {
        __cpuid(CPUInfo, i);
        printf_s("\nFor InfoType %x\n", i); 
        printf_s("CPUInfo[0] = 0x%x\n", CPUInfo[0]);
        printf_s("CPUInfo[1] = 0x%x\n", CPUInfo[1]);
        printf_s("CPUInfo[2] = 0x%x\n", CPUInfo[2]);
        printf_s("CPUInfo[3] = 0x%x\n", CPUInfo[3]);

        if  (i == 0x80000001)
        {
            bLAHF_SAHFAvailable = (CPUInfo[2] & 0x1) || false;
            bCmpLegacy = (CPUInfo[2] & 0x2) || false;
            bSVM = (CPUInfo[2] & 0x4) || false;
            bExtApicSpace = (CPUInfo[2] & 0x8) || false;
            bAltMovCr8 = (CPUInfo[2] & 0x10) || false;
            bLZCNT = (CPUInfo[2] & 0x20) || false;
            bSSE4A = (CPUInfo[2] & 0x40) || false;
            bMisalignedSSE = (CPUInfo[2] & 0x80) || false;
            bPREFETCH = (CPUInfo[2] & 0x100) || false;
            bSKINITandDEV = (CPUInfo[2] & 0x1000) || false;
            bSYSCALL_SYSRETAvailable = (CPUInfo[3] & 0x800) || false;
            bExecuteDisableBitAvailable = (CPUInfo[3] & 0x10000) || false;
            bMMXExtensions = (CPUInfo[3] & 0x40000) || false;
            bFFXSR = (CPUInfo[3] & 0x200000) || false;
            b1GBSupport = (CPUInfo[3] & 0x400000) || false;
            bRDTSCP = (CPUInfo[3] & 0x8000000) || false;
            b64Available = (CPUInfo[3] & 0x20000000) || false;
            b3DNowExt = (CPUInfo[3] & 0x40000000) || false;
            b3DNow = (CPUInfo[3] & 0x80000000) || false;
        }

        // Interpret CPU brand string and cache information.
        if  (i == 0x80000002)
            memcpy(CPUBrandString, CPUInfo, sizeof(CPUInfo));
        else if  (i == 0x80000003)
            memcpy(CPUBrandString + 16, CPUInfo, sizeof(CPUInfo));
        else if  (i == 0x80000004)
            memcpy(CPUBrandString + 32, CPUInfo, sizeof(CPUInfo));
        else if  (i == 0x80000006)
        {
            nCacheLineSize = CPUInfo[2] & 0xff;
            nL2Associativity = (CPUInfo[2] >> 12) & 0xf;
            nCacheSizeK = (CPUInfo[2] >> 16) & 0xffff;
        }
        else if  (i == 0x80000008)
        {
           nPhysicalAddress = CPUInfo[0] & 0xff;
           nVirtualAddress = (CPUInfo[0] >> 8) & 0xff;
        }
        else if  (i == 0x8000000A)
        {
            bNestedPaging = (CPUInfo[3] & 0x1) || false;
            bLBRVisualization = (CPUInfo[3] & 0x2) || false;
        }
        else if  (i == 0x8000001A)
        {
            bFP128 = (CPUInfo[0] & 0x1) || false;
            bMOVOptimization = (CPUInfo[0] & 0x2) || false;
        }
    }

    // Display all the information in user-friendly format.

    printf_s("\n\nCPU String: %s\n", CPUString);

    if  (nIds >= 1)
    {
        if  (nSteppingID)
            printf_s("Stepping ID = %d\n", nSteppingID);
        if  (nModel)
            printf_s("Model = %d\n", nModel);
        if  (nFamily)
            printf_s("Family = %d\n", nFamily);
        if  (nProcessorType)
            printf_s("Processor Type = %d\n", nProcessorType);
        if  (nExtendedmodel)
            printf_s("Extended model = %d\n", nExtendedmodel);
        if  (nExtendedfamily)
            printf_s("Extended family = %d\n", nExtendedfamily);
        if  (nBrandIndex)
            printf_s("Brand Index = %d\n", nBrandIndex);
        if  (nCLFLUSHcachelinesize)
            printf_s("CLFLUSH cache line size = %d\n",
                     nCLFLUSHcachelinesize);
        if (bMultithreading && (nLogicalProcessors > 0))
           printf_s("Logical Processor Count = %d\n", nLogicalProcessors);
        if  (nAPICPhysicalID)
            printf_s("APIC Physical ID = %d\n", nAPICPhysicalID);

        if  (nFeatureInfo || bSSE3Instructions ||
             bMONITOR_MWAIT || bCPLQualifiedDebugStore ||
             bVirtualMachineExtensions || bEnhancedIntelSpeedStepTechnology ||
             bThermalMonitor2 || bSupplementalSSE3 || bL1ContextID || 
             bCMPXCHG16B || bxTPRUpdateControl || bPerfDebugCapabilityMSR || 
             bSSE41Extensions || bSSE42Extensions || bPOPCNT || 
             bLAHF_SAHFAvailable || bCmpLegacy || bSVM ||
             bExtApicSpace || bAltMovCr8 ||
             bLZCNT || bSSE4A || bMisalignedSSE ||
             bPREFETCH || bSKINITandDEV || bSYSCALL_SYSRETAvailable || 
             bExecuteDisableBitAvailable || bMMXExtensions || bFFXSR || b1GBSupport ||
             bRDTSCP || b64Available || b3DNowExt || b3DNow || bNestedPaging || 
             bLBRVisualization || bFP128 || bMOVOptimization )
        {
            printf_s("\nThe following features are supported:\n");

            if  (bSSE3Instructions)
                printf_s("\tSSE3\n");
            if  (bMONITOR_MWAIT)
                printf_s("\tMONITOR/MWAIT\n");
            if  (bCPLQualifiedDebugStore)
                printf_s("\tCPL Qualified Debug Store\n");
            if  (bVirtualMachineExtensions)
                printf_s("\tVirtual Machine Extensions\n");
            if  (bEnhancedIntelSpeedStepTechnology)
                printf_s("\tEnhanced Intel SpeedStep Technology\n");
            if  (bThermalMonitor2)
                printf_s("\tThermal Monitor 2\n");
            if  (bSupplementalSSE3)
                printf_s("\tSupplemental Streaming SIMD Extensions 3\n");
            if  (bL1ContextID)
                printf_s("\tL1 Context ID\n");
            if  (bCMPXCHG16B)
                printf_s("\tCMPXCHG16B Instruction\n");
            if  (bxTPRUpdateControl)
                printf_s("\txTPR Update Control\n");
            if  (bPerfDebugCapabilityMSR)
                printf_s("\tPerf\\Debug Capability MSR\n");
            if  (bSSE41Extensions)
                printf_s("\tSSE4.1 Extensions\n");
            if  (bSSE42Extensions)
                printf_s("\tSSE4.2 Extensions\n");
            if  (bPOPCNT)
                printf_s("\tPPOPCNT Instruction\n");

            i = 0;
            nIds = 1;
            while (i < (sizeof(szFeatures)/sizeof(const char*)))
            {
                if  (nFeatureInfo & nIds)
                {
                    printf_s("\t");
                    printf_s(szFeatures[i]);
                    printf_s("\n");
                }

                nIds <<= 1;
                ++i;
            }
            if (bLAHF_SAHFAvailable)
                printf_s("\tLAHF/SAHF in 64-bit mode\n");
            if (bCmpLegacy)
                printf_s("\tCore multi-processing legacy mode\n");
            if (bSVM)
                printf_s("\tSecure Virtual Machine\n");
            if (bExtApicSpace)
                printf_s("\tExtended APIC Register Space\n");
            if (bAltMovCr8)
                printf_s("\tAltMovCr8\n");
            if (bLZCNT)
                printf_s("\tLZCNT instruction\n");
            if (bSSE4A)
                printf_s("\tSSE4A (EXTRQ, INSERTQ, MOVNTSD, MOVNTSS)\n");
            if (bMisalignedSSE)
                printf_s("\tMisaligned SSE mode\n");
            if (bPREFETCH)
                printf_s("\tPREFETCH and PREFETCHW Instructions\n");
            if (bSKINITandDEV)
                printf_s("\tSKINIT and DEV support\n");
            if (bSYSCALL_SYSRETAvailable)
                printf_s("\tSYSCALL/SYSRET in 64-bit mode\n");
            if (bExecuteDisableBitAvailable)
                printf_s("\tExecute Disable Bit\n");
            if (bMMXExtensions)
                printf_s("\tExtensions to MMX Instructions\n");
            if (bFFXSR)
                printf_s("\tFFXSR\n");
            if (b1GBSupport)
                printf_s("\t1GB page support\n");
            if (bRDTSCP)
                printf_s("\tRDTSCP instruction\n");
            if (b64Available)
                printf_s("\t64 bit Technology\n");
            if (b3DNowExt)
                printf_s("\t3Dnow Ext\n");
            if (b3DNow)
                printf_s("\t3Dnow! instructions\n");
            if (bNestedPaging)
                printf_s("\tNested Paging\n");
            if (bLBRVisualization)
                printf_s("\tLBR Visualization\n");
            if (bFP128)
                printf_s("\tFP128 optimization\n");
            if (bMOVOptimization)
                printf_s("\tMOVU Optimization\n");
        }
    }

    if  (nExIds >= 0x80000004)
        printf_s("\nCPU Brand String: %s\n", CPUBrandString);

    if  (nExIds >= 0x80000006)
    {
        printf_s("Cache Line Size = %d\n", nCacheLineSize);
        printf_s("L2 Associativity = %d\n", nL2Associativity);
        printf_s("Cache Size = %dK\n", nCacheSizeK);
    }

    if  (nExIds >= 0x80000008)
    {
        printf_s("Physical Address = %d\n", nPhysicalAddress);
        printf_s("Virtual Address = %d\n", nVirtualAddress);
    }

	#define ARRAY_SIZE (4096 * 1024)

	/*

	// Stuff.
	std::cout<<"\nTesting division using regular floating point and SSE SIMD extensions.";
	std::cout<<"\nTest array size: "<<ARRAY_SIZE;
	float * arr1 = new float[ARRAY_SIZE];
	float * arr2 = new float[ARRAY_SIZE];
	std::cout<<"\nFirst numbers";
	// Fill with random values.
	for (int i = 0; i < ARRAY_SIZE; ++i)
	{
		arr1[i] = (float)rand();
		arr2[i] = (float)rand();
		if (i < 8)
		{
			std::cout<<"\n"<<i<<". "<<arr1[i]<<" op "<<arr2[i];
		}
	}

	float * _arr1, * _arr2;
	_arr1 = (float*) _aligned_malloc(ARRAY_SIZE * sizeof(float), 16);
	_arr2 = (float*) _aligned_malloc(ARRAY_SIZE * sizeof(float), 16);
// Copy values so they can be cross-examined.
	for (int i = 0; i < ARRAY_SIZE; ++i)
	{
		_arr1[i] = arr1[i];
		_arr2[i] = arr2[i];
	}
	

	// Do division.
	Timer timer;
	timer.Start();
	for (int i = 0; i < ARRAY_SIZE; ++i)
	{
		arr1[i] = arr1[i] / arr2[i];
//		arr1[i] = arr1[i] - arr2[i];
//		arr1[i] = arr1[i] + arr2[i];
//		arr1[i] = arr1[i] * arr2[i];
	}
	timer.Stop();
	int ms = timer.GetMs();

	// Do division.
	// https://msdn.microsoft.com/en-us/library/c9848chc%28v=vs.90%29.aspx Arithmetic operations! o.o
	int loop = ARRAY_SIZE / 4;
	__m128 * left = (__m128*) _arr1;
	__m128 * right = (__m128*) _arr2;
	__m128 * result = (__m128*)_arr1;
	timer.Start();
	for (int i = 0; i < loop; ++i)
	{
		*result = _mm_div_ps(*left, *right);
//		*result = _mm_sub_ps(*left, *right);
//		*result = _mm_add_ps(*left, *right);
//		*result = _mm_mul_ps(*left, *right);
		++left;
		++right;
		++result;
	}
	timer.Stop();
	int ms2 = timer.GetMs();
	std::cout<<"\nTime taken, regular: "<<ms<<" SSE:"<<ms2;

	// Print results of both to ensure validity.
	for (int i = 0; i < 8; ++i)
	{
		std::cout<<"\nStandard: "<<arr1[i]<<" SSE: "<<_arr1[i];
	}

	for (int i = ARRAY_SIZE - 8; i < ARRAY_SIZE; ++i)
	{
		std::cout<<"\nStandard: "<<arr1[i]<<" SSE: "<<_arr1[i];
	}


	delete[] arr1;
	delete[] arr2;

	_aligned_free(_arr1);
	_aligned_free(_arr2);

*/


	/// Testing vectors..!
	Vector4f one(1,0,-1,0.2), two(0.1,-0.2,1.7,1.0);
#ifdef USE_SSE
	one.PrepareForSIMD();
	two.PrepareForSIMD();
#endif
//	Vector4f::useSSE = false;
	Timer timer;
	timer.Start();
	for (int i = 0; i < ARRAY_SIZE; ++i)
	{
		one = one + two;
	}
	timer.Stop();
	int ms = timer.GetMs();
	one = Vector4f(1,0,-1,0.2), two = Vector4f(0.1,-0.2,1.7,1.0);
#ifdef USE_SSE
	one.PrepareForSIMD();
	two.PrepareForSIMD();
#endif
//	Vector4f::useSSE = true;
	timer.Start();
	for (int i = 0; i < ARRAY_SIZE; ++i)
	{
		one = one + two;
		if (i < 4 || i > ARRAY_SIZE - 4)
#ifdef USE_SSE
			std::cout<<"\ni: "<<one.data.m128_f32[0]<<" "<<one.data.m128_f32[1]<<" "<<one.data.m128_f32[2]<<" "<<one.data.m128_f32[3];
#else
			std::cout<<"\ni: "<<one[0]<<" "<<one[1]<<" "<<one[2]<<" "<<one[3];
#endif
	}
	timer.Stop();
	int ms2 = timer.GetMs();
	std::cout<<"\nVector addition, standard: "<<ms<<" SSE: "<<ms2;
#endif // WINDOWS
}

