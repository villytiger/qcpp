import qbs

Project {
    qbsSearchPaths: ["qbs"]

    AutotestRunner {
        arguments: ["--gtest_color=yes"]
    }

    references: [
        "googletest/googletest.qbs",
        "src/qpromise.qbs",
        "tests/tests.qbs"
    ]
}
