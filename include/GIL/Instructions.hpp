
#ifndef GLU_GIL_INSTRUCTIONS_HPP
#define GLU_GIL_INSTRUCTIONS_HPP

#include "Instructions/AddInst.hpp"
#include "Instructions/AllocaInst.hpp"
#include "Instructions/ArithmeticInst.hpp"
#include "Instructions/BitcastInst.hpp"
#include "Instructions/BrInst.hpp"
#include "Instructions/CallInst.hpp"
#include "Instructions/CastIntToPtrInst.hpp"
#include "Instructions/CastPtrToIntInst.hpp"
#include "Instructions/CondBrInst.hpp"
#include "Instructions/DebugInst.hpp"
#include "Instructions/EnumVariantInst.hpp"
#include "Instructions/FloatExtInst.hpp"
#include "Instructions/FloatLiteralInst.hpp"
#include "Instructions/FloatTruncInst.hpp"
#include "Instructions/FunctionPtrInst.hpp"
#include "Instructions/IntSextInst.hpp"
#include "Instructions/IntTruncInst.hpp"
#include "Instructions/IntZextInst.hpp"
#include "Instructions/IntegerLiteralInst.hpp"
#include "Instructions/LoadInst.hpp"
#include "Instructions/PtrOffsetInst.hpp"
#include "Instructions/ReturnInst.hpp"
#include "Instructions/StoreInst.hpp"
#include "Instructions/StringLiteralInst.hpp"
#include "Instructions/StructCreateInst.hpp"
#include "Instructions/StructDestructureInst.hpp"
#include "Instructions/StructExtractInst.hpp"
#include "Instructions/StructFieldPtrInst.hpp"
#include "Instructions/UnreachableInst.hpp"

// Needed by InstBase
#include "BasicBlock.hpp"

#endif // GLU_GIL_INSTRUCTIONS_HPP
