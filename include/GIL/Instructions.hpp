
#ifndef GLU_GIL_INSTRUCTIONS_HPP
#define GLU_GIL_INSTRUCTIONS_HPP

// Terminator Instructions
#include "Instructions/Terminator/BrInst.hpp"
#include "Instructions/Terminator/CondBrInst.hpp"
#include "Instructions/Terminator/ReturnInst.hpp"
#include "Instructions/Terminator/UnreachableInst.hpp"

// Constant Instructions
#include "Instructions/Constants/EnumVariantInst.hpp"
#include "Instructions/Constants/FloatLiteralInst.hpp"
#include "Instructions/Constants/FunctionPtrInst.hpp"
#include "Instructions/Constants/GlobalPtrInst.hpp"
#include "Instructions/Constants/IntegerLiteralInst.hpp"
#include "Instructions/Constants/StringLiteralInst.hpp"

// Memory Instructions
#include "Instructions/Memory/AllocaInst.hpp"
#include "Instructions/Memory/LoadInst.hpp"
#include "Instructions/Memory/StoreInst.hpp"

// Aggregate Instructions
#include "Instructions/Aggregates/ArrayCreateInst.hpp"
#include "Instructions/Aggregates/PtrOffsetInst.hpp"
#include "Instructions/Aggregates/StructCreateInst.hpp"
#include "Instructions/Aggregates/StructExtractInst.hpp"
#include "Instructions/Aggregates/StructFieldPtrInst.hpp"

// Call Instructions
#include "Instructions/CallInst.hpp"

// Debug Instructions
#include "Instructions/DebugInst.hpp"

// Conversion Instructions
#include "Instructions/Conversions/BitcastInst.hpp"
#include "Instructions/Conversions/CastIntToPtrInst.hpp"
#include "Instructions/Conversions/CastPtrToIntInst.hpp"
#include "Instructions/Conversions/ConversionInst.hpp"
#include "Instructions/Conversions/FloatExtInst.hpp"
#include "Instructions/Conversions/FloatToIntInst.hpp"
#include "Instructions/Conversions/FloatTruncInst.hpp"
#include "Instructions/Conversions/IntSextInst.hpp"
#include "Instructions/Conversions/IntToFloatInst.hpp"
#include "Instructions/Conversions/IntTruncInst.hpp"
#include "Instructions/Conversions/IntZextInst.hpp"

// OSSA Instructions
#include "Instructions/OSSA/DropInst.hpp"

// Needed by InstBase
#include "BasicBlock.hpp"

#endif // GLU_GIL_INSTRUCTIONS_HPP
