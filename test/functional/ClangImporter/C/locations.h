// RUN: gluc %s -print-ast | FileCheck -v %s

struct LocStruct {
    int value;
};

struct Forward;
struct Forward *useForward(struct Forward *input);

struct Forward {
    int value;
};

int locationFunc(int arg);

// CHECK: ModuleDecl {{.*}}locations.h, line:1:1>

// CHECK: StructDecl {{.*}} <line:3:8>
// CHECK-NEXT: -->Name: LocStruct
// CHECK-NEXT: -->Fields:
// CHECK-NEXT: FieldDecl {{.*}} <line:4:9>
// CHECK-NEXT: -->Name: value
// CHECK-NEXT: -->Type: Int32

// CHECK: StructDecl {{.*}} <line:10:8>
// CHECK-NEXT: -->Name: Forward
// CHECK-NEXT: -->Fields:
// CHECK-NEXT: FieldDecl {{.*}} <line:11:9>
// CHECK-NEXT: -->Name: value
// CHECK-NEXT: -->Type: Int32

// CHECK: FunctionDecl {{.*}} <line:8:17>
// CHECK-NEXT: -->Name: useForward
// CHECK-NEXT: -->Type: (*Forward) -> *Forward
// CHECK: -->Params:
// CHECK-NEXT: ParamDecl {{.*}} <line:8:44>
// CHECK-NEXT: -->input : *Forward

// CHECK: FunctionDecl {{.*}} <line:14:5>
// CHECK-NEXT: -->Name: locationFunc
// CHECK-NEXT: -->Type: (Int32) -> Int32
// CHECK: -->Params:
// CHECK-NEXT: ParamDecl {{.*}} <line:14:22>
// CHECK-NEXT: -->arg : Int32
