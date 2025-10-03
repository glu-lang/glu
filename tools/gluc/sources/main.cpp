#include "CompilerDriver.hpp"

int main(int argc, char **argv)
{
    glu::driver::CompilerDriver driver;
    return driver.run(argc, argv);
}
