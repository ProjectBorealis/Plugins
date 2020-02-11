// Copyright © 2018-2019 Praxinos, Inc. All Rights Reserved.
// IDDN FR.001.250001.002.S.P.2019.000.00000

/**
 * @file        ULIS.Config.h
 * @author      Clement Berthaud
 * @copyright   Copyright © 2018-2019 Praxinos, Inc. All Rights Reserved.
 * @license     Please refer to LICENSE.md
 */

#pragma once

/////////////////////////////////////////////////////
// Detect Config
#ifdef NDEBUG
    #define ULIS_RELEASE
#else // !NDEBUG
    #define ULIS_DEBUG
#endif // !NDEBUG

/////////////////////////////////////////////////////
// Detect Compiler
#if defined(__clang__)
    #define ULIS_CLANG
#elif defined(__GNUC__) || defined(__GNUG__)
    #define ULIS_GCC
#elif defined(_MSC_VER)
    #define ULIS_MSVC
#else
    #define ULIS_UNKNOWN_COMPILER
#endif


/////////////////////////////////////////////////////
// Detect Platform
#ifdef _WIN32
    #define ULIS_WIN
   #ifdef _WIN64
      #define ULIS_WIN64
   #else
      #define ULIS_WIN32
   #endif
#elif __APPLE__
    #include "TargetConditionals.h"
    #if TARGET_IPHONE_SIMULATOR
         #define ULIS_IOS_SIMULATOR
    #elif TARGET_OS_IPHONE
        #define ULIS_IOS_DEVICE
    #elif TARGET_OS_MAC
        #define ULIS_MACOS
    #else
        #error "Unknown Apple platform"
    #endif
#elif __linux__
    #define ULIS_LINUX
#elif __unix__
    #define ULIS_UNIX
#elif defined(_POSIX_VERSION)
    #define ULIS_POSIX
#else
    #error "Unknown Platform"
#endif

/////////////////////////////////////////////////////
// Force Inline
#define ULIS_ENABLE_FORCEINLINE
#ifdef ULIS_ENABLE_FORCEINLINE
    #if defined(__clang__)
    #define ULIS_FORCEINLINE inline __attribute__ ((always_inline))
    #elif defined(__GNUC__) || defined(__GNUG__)
    #define ULIS_FORCEINLINE inline __attribute__ ((always_inline))
    #elif defined(_MSC_VER)
    #define ULIS_FORCEINLINE __forceinline
    #else
    #define ULIS_FORCEINLINE inline
    #endif
#else // ULIS_ENABLE_FORCEINLINE
    #define ULIS_FORCEINLINE inline
#endif // ULIS_ENABLE_FORCEINLINE


/////////////////////////////////////////////////////
// Define Namespaces
#define ULIS_NAMESPACE_NAME     ULIS
#define ULIS_NAMESPACE_BEGIN    namespace ULIS_NAMESPACE_NAME {
#define ULIS_NAMESPACE_END      }
#define ULIS_CLASS_FORWARD_DECLARATION( i ) ULIS_NAMESPACE_BEGIN class i ; ULIS_NAMESPACE_END

/////////////////////////////////////////////////////
// Version Specification
#define ULIS_VERSION_MAJOR      0
#define ULIS_VERSION_MINOR      1
#define ULIS_VERSION_MAJOR_STR  "0"
#define ULIS_VERSION_MINOR_STR  "1"

/////////////////////////////////////////////////////
// Crash Behaviours
#define ULIS_CRASH              *((int*)0) = 0
#define ULIS_CRASH_DELIBERATE   ULIS_CRASH
#define ULIS_CRASH_TEMPORARY    ULIS_CRASH
#define ULIS_CRASH_TODO         ULIS_CRASH
#define ULIS_CRASH_CHECK        ULIS_CRASH

/////////////////////////////////////////////////////
// Debug Behaviours
#ifdef ULIS_DEBUG
    // Warning ! This flag can cause execution to become very slow !
    // Enable only for debugging small parts.
    //#define ULIS_DEBUG_TYPE_STR_SYMBOL_ENABLED
#endif // ULIS_DEBUG

/////////////////////////////////////////////////////
// GLM BEHAVIOUR
// User defines: GLM_FORCE_PURE GLM_FORCE_INTRINSICS GLM_FORCE_SSE2 GLM_FORCE_SSE3 GLM_FORCE_AVX GLM_FORCE_AVX2 GLM_FORCE_AVX2
#define GLM_FORCE_SSE42
#define GLM_FORCE_SSE41
#define GLM_FORCE_SSSE3
#define GLM_FORCE_SSE3
#define GLM_FORCE_SSE2
#define GLM_FORCE_SSE
#define GLM_FORCE_INTRINSICS
#include <glm/glm.hpp>
//#define GLM_FORCE_SSE2
