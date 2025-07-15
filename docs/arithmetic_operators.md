# Intrinsic Arithmetic Operators System

This document describes how basic arithmetic operators (+, -, *, /) are implemented in the Glu compiler as intrinsics, avoiding the need to explicitly import C functions.

## Architecture

### 1. AST Level
- Operators are represented by `BinaryOpExpr` with the operator stored as `RefExpr`
- Operator tokens are defined in `TokenKind.def`

### 2. GIL Level (Glu Intermediate Language)
- **Arithmetic instructions**: `AddInst`, `SubInst`, `MulInst`, `DivInst` for integers
- **Floating-point instructions**: `FAddInst`, `FSubInst`, `FMulInst`, `FDivInst` for floats
- **Base class**: `ArithmeticInst` for all arithmetic instructions

### 3. Code Generation
- **GILGen**: Detects intrinsic operators and generates appropriate GIL instructions
- **IRGen**: Translates GIL instructions to LLVM IR instructions

## Implementation

### Main Files

1. **`include/GIL/InstKind.def`**: Definition of arithmetic instruction types
2. **`include/GIL/Instructions/ArithmeticInst.hpp`**: Base class for arithmetic instructions
3. **`include/GIL/Instructions/AddInst.hpp`**: Specific arithmetic instruction implementations
4. **`lib/GILGen/Intrinsics.hpp`**: Intrinsic generator for arithmetic operators
5. **`lib/GILGen/GILGenExpr.hpp`**: GIL generation for binary expressions (modified)
6. **`lib/IRGen/IRGen.cpp`**: LLVM IR generation for arithmetic instructions (modified)

### Compilation Flow

1. **Parsing**: `a + b` → `BinaryOpExpr(RefExpr("+"), a, b)`
2. **GILGen**: Detects that "+" is an intrinsic operator → generates `AddInst` or `FAddInst`
3. **IRGen**: Translates `AddInst` → `llvm::IRBuilder::CreateAdd()`

### Supported Types

- **Integers**: `Int` (32 bits by default)
- **Floats**: `Float` (32 bits by default)  
- **Automatic conversions**: `Int + Float` → `Float`

## Usage

```glu
func main() {
    let a: Int = 10;
    let b: Int = 20;
    let sum = a + b;    // Generates AddInst
    
    let x: Float = 3.14;
    let y: Float = 2.71;
    let fsum = x + y;   // Generates FAddInst
    
    let mixed = a + x;  // Generates FAddInst with conversion
}
```

## Advantages

1. **Performance**: No function calls, direct instructions
2. **Optimization**: LLVM can optimize arithmetic operations
3. **Simplicity**: No need to import C functions
4. **Type Safety**: Automatic type checking and conversions

## Extension

To add new intrinsic operators:

1. Add the instruction in `InstKind.def`
2. Create the instruction class in `Instructions/`
3. Add support in `Intrinsics.hpp`
4. Implement LLVM generation in `IRGen.cpp`

## Tests

Tests are in `test/GILGen/ArithmeticIntrinsicsTest.cpp` and cover:
- Instruction generation for different types
- Intrinsic operator detection
- Automatic type conversion
