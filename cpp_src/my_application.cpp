// cpp_src/my_application.cpp
#include "my_application.h"
#include <pybind11/embed.h>
#include <iostream>

namespace py = pybind11;

MyApplication::MyApplication() {
    // Constructor implementation
}

double MyApplication::runPythonCode(double input) {
    py::scoped_interpreter guard{};  // Start the Python interpreter

    py::module_ math = py::module_::import("math");
    double result = math.attr("sqrt")(input).cast<double>();

    std::cout << "The square root of " << input << " is " << result << std::endl;

    return result;  // Return the result to be used elsewhere in your C++ code
}
