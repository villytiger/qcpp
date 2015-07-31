import qbs

StaticLibrary {
    name: "qpromise"

    files: ["*.cpp", "*.h"]

    Depends { name: "cpp" }
    Depends { name: "Qt.core" }
    Depends { name: "ExportHeaders" }

    ExportHeaders.headers: [{module: "QPromise", file: "qpromise.h", alias: "QPromise"}]

    cpp.cxxLanguageVersion: "c++14"
    cpp.warningLevel: "all"
    cpp.treatWarningsAsErrors: true

    cpp.includePaths: [ExportHeaders.includePath]

    Export {
        Depends { name: "cpp" }
        Depends { name: "Qt.core" }
        cpp.includePaths: [project.publicIncludePath]
    }
}
