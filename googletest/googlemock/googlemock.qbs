import qbs

Project {
    StaticLibrary {
        name: "gmock"
        files: ["src/gmock-all.cc"]

        Depends { name: "cpp" }
        Depends { name: "gtest" }

        cpp.systemIncludePaths: ["include", "."]

        Export {
            Depends { name: "cpp" }
            cpp.systemIncludePaths: ["include"]
        }
    }

    StaticLibrary {
        name: "gmock-main"
        files: ["src/gmock_main.cc"]

        Depends { name: "cpp" }
        Depends { name: "gtest" }
        Depends { name: "gmock" }

        cpp.systemIncludePaths: ["include"]
    }
}
