import qbs

Project {
    qbsSearchPaths: ["qbs"]

    AutotestRunner {
        arguments: ["--gtest_color=yes"]
    }

    references: [
        "src/qpromise.qbs",
        "tests/tests.qbs"
    ]
}
