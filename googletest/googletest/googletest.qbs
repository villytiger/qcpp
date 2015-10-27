import qbs

Project {
    StaticLibrary {
        name: "gtest"
        files: ["src/gtest-all.cc"]

        Depends { name: "cpp" }

        cpp.systemIncludePaths: ["include", "."]

        Export {
            Depends { name: "cpp" }
            cpp.systemIncludePaths: ["include"]
        }
    }

    StaticLibrary {
        name: "gtest-main"
        files: ["src/gtest_main.cc"]

        Depends { name: "cpp" }
        Depends { name: "gtest" }

        cpp.systemIncludePaths: ["include"]
    }
}
