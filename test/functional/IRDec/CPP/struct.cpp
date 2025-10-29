//
// RUN: clang++ -std=c++20 -g -c -emit-llvm %s -o %t.bc
// RUN: gluc %t.bc -print-interface | FileCheck -v %s
//

#include <chrono>
#include <format>

struct Person {
    char const *name;
    unsigned birthYear;

    static Person getEmil();
    unsigned getAgeInYear(unsigned year) const;
    unsigned getCurrentAge() const;
};

// CHECK: public func getEmil() -> Person;
Person Person::getEmil()
{
    return { "Emil", 2003 };
}

// CHECK: public func getAgeInYear({{.*}}: *Person, {{.*}}: UInt32) -> UInt32;
unsigned Person::getAgeInYear(unsigned year) const
{
    return year - birthYear;
}

// CHECK: public func getCurrentAge({{.*}}: *Person) -> UInt32;
unsigned Person::getCurrentAge() const
{
    using namespace std::chrono;
    auto now = system_clock::now();
    auto currentYear = year_month_day(time_point_cast<days>(now)).year();
    return getAgeInYear(static_cast<int>(currentYear));
}

// CHECK: public struct Person {
// CHECK-NEXT:    public name: *Char,
// CHECK-NEXT:    public birthYear: UInt32,
// CHECK-NEXT:}
