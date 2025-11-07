
#ifndef GLU_GIL_INSTRUCTIONS_HPP
#define GLU_GIL_INSTRUCTIONS_HPP

#include "Instructions/AllocaInst.hpp"
#include "Instructions/BrInst.hpp"
#include "Instructions/CallInst.hpp"
#include "Instructions/CondBrInst.hpp"
#include "Instructions/DebugInst.hpp"
#include "Instructions/EnumVariantInst.hpp"
#include "Instructions/FloatLiteralInst.hpp"
#include "Instructions/FunctionPtrInst.hpp"
#include "Instructions/GlobalPtrInst.hpp"
#include "Instructions/IntegerLiteralInst.hpp"
#include "Instructions/LoadInst.hpp"
#include "Instructions/PtrOffsetInst.hpp"
#include "Instructions/ReturnInst.hpp"
#include "Instructions/StoreInst.hpp"
#include "Instructions/StringLiteralInst.hpp"
#include "Instructions/StructCreateInst.hpp"
#include "Instructions/StructExtractInst.hpp"
#include "Instructions/StructFieldPtrInst.hpp"
#include "Instructions/UnreachableInst.hpp"

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
#include "Instructions/DropInst.hpp"

// Needed by InstBase
#include "BasicBlock.hpp"

#endif // GLU_GIL_INSTRUCTIONS_HPP
