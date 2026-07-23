//
// RUN: zig build-obj -fllvm -fno-strip %s -femit-llvm-ir=%t.ll
// RUN: gluc %t.ll -print-interface | FileCheck -v %s
//
const std = @import("std");

// CHECK-DAG: func sayHello();
export fn sayHello() void {
    var threaded = std.Io.Threaded.init(std.heap.page_allocator, .{});
    defer threaded.deinit();
    std.Io.File.writeStreamingAll(
        .stdout(),
        threaded.io(),
        "Hello, World!\n"
    ) catch {};
}

// CHECK-DAG: func square(x: Int32) -> Int32;
export fn square(x: i32) i32 {
    return x * x;
}

// CHECK-DAG: func getCC() -> *UInt8;
export fn getCC() [*:0]const u8 {
    return "Zig";
}
