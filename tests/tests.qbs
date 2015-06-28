import qbs

CppApplication {
    name: "test"
    type: ["application", "autotest"]

    files: ["*.cpp", "*.h"]

    Depends { name: "cpp" }

    cpp.cxxLanguageVersion: "c++14"
    cpp.warningLevel: "all"
    cpp.treatWarningsAsErrors: true

    cpp.includePaths: [publicIncludePath]

    cpp.dynamicLibraries: ["gtest", "gtest_main"]
}
