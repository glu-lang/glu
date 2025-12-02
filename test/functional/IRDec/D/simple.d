//
// RUN: ldc2 -c --output-ll -g %s -of=%t.ll
// RUN: gluc %t.ll -print-interface | FileCheck -v %s
//

import std.stdio;

// CHECK: namespace simple
// CHECK: func getPizzaCount() -> Int64;
long getPizzaCount() {
    return 42;
}

// CHECK: func square(x: Int32) -> Int32;
int square(int x) {
    return x * x;
}

// CHECK: func getCC() -> string;
string getCC() {
    return "LDC";
}

// CHECK: func showInfo();
void showInfo() {
    writeln("Hello from D!");
}
