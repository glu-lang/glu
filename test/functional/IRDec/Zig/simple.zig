//
// RUN: zig build-obj -fllvm -fno-strip %s -femit-llvm-ir=%t.ll
// RUN: gluc %t.ll -print-interface | FileCheck -v %s
//
const std = @import("std");

// CHECK: func sayHello();
export fn sayHello() void {
    std.fs.File.stdout().writeAll("Hello, World!\n") catch {};
}

// CHECK: func square(x: Int32) -> Int32;
export fn square(x: i32) i32 {
    return x * x;
}

// CHECK: func getCC() -> *UInt8;
export fn getCC() [*:0]const u8 {
    return "Zig";
}
