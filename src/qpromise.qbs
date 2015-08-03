import qbs

StaticLibrary {
    name: "qpromise"

    files: ["*.cpp", "*.h"]

    Depends { name: "cpp" }
    Depends { name: "Qt.core" }
    Depends { name: "ExportHeaders" }

    ExportHeaders.headers: [
        {module: "QPromise", file: "qpromiseglobal.h"},
        {module: "QPromise", file: "qpromisetraits.h"},
        {module: "QPromise", file: "qdeferred.h", alias: "QDeferred"},
        {module: "QPromise", file: "qpromise.h", alias: "QPromise"}
    ]

    cpp.cxxLanguageVersion: "c++14"
    cpp.warningLevel: "all"
    cpp.treatWarningsAsErrors: true

    cpp.includePaths: [ExportHeaders.includePath]

    Export {
        Depends { name: "cpp" }
        Depends { name: "Qt.core" }
        cpp.includePaths: [ExportHeaders.includePath]
    }
}
