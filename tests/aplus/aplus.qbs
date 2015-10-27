import qbs

CppApplication {
    name: "aplus-tests"
    type: ["application", "autotest"]

    files: ["*.cpp"]

    Depends { name: "qpromise" }
    Depends { name: "gmock" }
    Depends { name: "gtest" }

    cpp.cxxLanguageVersion: "c++14"
    cpp.warningLevel: "all"
    cpp.treatWarningsAsErrors: true

    cpp.includePaths: ["aplusplus-tests/include"]
}
