import qbs

CppApplication {
    name: "test"
    type: ["application", "autotest"]

    files: ["*.cpp"]

    Depends { name: "qpromise" }

    cpp.cxxLanguageVersion: "c++14"
    cpp.warningLevel: "all"
    cpp.treatWarningsAsErrors: true

    cpp.dynamicLibraries: ["gtest"]
}
