//
// RUN: swiftc -parse-as-library -emit-ir -g -gdwarf-types %s -o %t.ll
// RUN: gluc %t.ll -print-interface | FileCheck -v %s
//

// CHECK: public func getPizzaCount() -> Int32;
public func getPizzaCount() -> Int32 {
    return 42
}

// CHECK: public func square(x: Int32) -> Int32;
public func square(x: Int32) -> Int32 {
    return x * x
}

// CHECK: public func printCC()
public func printCC() {
    print("Swift", terminator: "")
}
