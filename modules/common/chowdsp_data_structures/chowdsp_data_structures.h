/*
==============================================================================

BEGIN_JUCE_MODULE_DECLARATION

   ID:            chowdsp_data_structures
   vendor:        Chowdhury DSP
   version:       2.3.0
   name:          ChowDSP Data Structures
   description:   C++ data structures for other ChowDSP modules
   dependencies:  chowdsp_core

   website:       https://ccrma.stanford.edu/~jatin/chowdsp
   license:       BSD 3-Clause

   END_JUCE_MODULE_DECLARATION

==============================================================================
*/

#pragma once

#include <bitset>
#include <optional>

#include <chowdsp_core/chowdsp_core.h>

#include "third_party/short_alloc.h"

#include "Helpers/chowdsp_ArrayHelpers.h"
#include "Helpers/chowdsp_TupleHelpers.h"
#include "Helpers/chowdsp_VectorHelpers.h"
#include "Helpers/chowdsp_Iterators.h"

#include "Structures/chowdsp_DoubleBuffer.h"
#include "Structures/chowdsp_PackedPointer.h"
#include "Structures/chowdsp_DestructiblePointer.h"
#include "Structures/chowdsp_RawObject.h"
#include "Structures/chowdsp_LocalPointer.h"
#include "Structures/chowdsp_OptionalPointer.h"
#include "Structures/chowdsp_SmallVector.h"
#include "Structures/chowdsp_StringLiteral.h"
#include "Structures/chowdsp_OptionalRef.h"
#include "Structures/chowdsp_EnumMap.h"
#include "Structures/chowdsp_OptionalArray.h"

#include "Allocators/chowdsp_ArenaAllocator.h"
#include "Allocators/chowdsp_ChainedArenaAllocator.h"
#include "Allocators/chowdsp_STLArenaAllocator.h"
#include "Helpers/chowdsp_ArenaHelpers.h"

#include "Allocators/chowdsp_PoolAllocator.h"

#include "Structures/chowdsp_BucketArray.h"
#include "Structures/chowdsp_AbstractTree.h"
#include "Structures/chowdsp_SmallMap.h"
