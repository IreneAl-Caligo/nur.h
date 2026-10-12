from conan import ConanFile
from conan.tools.files import copy
import os

class NurConan(ConanFile):
    name = "nur"
    version = "1.2.0"
    license = "MIT"
    author = "Irene Al-Caligo"
    url = "https://github.com/IreneAl-Caligo/nur.h"
    description = "Zero-allocation, header-only Result<T> and Option<T> library for ISO C99, C11, C17, C23 and C++11"
    topics = ("c", "error-handling", "result", "option", "header-only", "zero-allocation")
    package_type = "header-library"
    no_copy_source = True

    def layout(self):
        pass

    def package(self):
        copy(self, "nur.h", src=os.path.join(self.source_folder, "src"), dst=os.path.join(self.package_folder, "include"))
        copy(self, "LICENSE", src=self.source_folder, dst=os.path.join(self.package_folder, "licenses"))

    def package_info(self):
        self.cpp_info.bindirs = []
        self.cpp_info.libdirs = []
        self.cpp_info.includedirs = ["include"]
