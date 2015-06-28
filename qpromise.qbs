import qbs

Project {
    id: qpromise
    property path publicIncludePath: qpromise.sourceDirectory + "/include"

    AutotestRunner {
        arguments: ["--gtest_color=yes"]
    }

    references: [
        "src/qpromise.qbs",
        "tests/tests.qbs"
    ]
}
