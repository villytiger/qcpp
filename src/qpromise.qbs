import qbs

StaticLibrary {
    name: "qpromise"

    files: ["*.cpp", "*.h", publicIncludePath + "/**/*.h"]

    Depends { name: "cpp" }
    Depends { name: "Qt.core" }

    cpp.cxxLanguageVersion: "c++14"
    cpp.warningLevel: "all"
    cpp.treatWarningsAsErrors: true

    cpp.includePaths: [publicIncludePath]

    Export {
        Depends { name: "cpp" }
        Depends { name: "Qt.core" }
        cpp.includePaths: [project.publicIncludePath]
    }
}
