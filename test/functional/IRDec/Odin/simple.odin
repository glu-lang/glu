//
// RUN: mkdir -p %t && cd %t && odin build %s -file -debug -build-mode:llvm-ir
// RUN: gluc -print-interface %t/simple-main.ll | FileCheck %s
//

package main

import "core:fmt"
import "core:runtime"

// CHECK: func `main::getPizzaCount`() -> Int64
@(require=true)
getPizzaCount :: proc "contextless" () -> int {
    return 123
}

// CHECK: func `main::getCC`() -> string
@(require=true)
getCC :: proc "contextless" () -> string {
    return "Odin"
}

// CHECK: func `main::showInfo`();
@(require=true)
showInfo :: proc "contextless" () {
    context = runtime.default_context()
    fmt.println("Hello from ", getCC(), "!")
}
