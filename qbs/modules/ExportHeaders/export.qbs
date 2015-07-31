import qbs
import qbs.File
import qbs.FileInfo
import qbs.TextFile

Module {
    property path includePath: FileInfo.joinPaths(buildDirectory, "include")
    property var headers: [] //: [{module: "QWamp", file: "qwamprawsession.h", alias: "QWampRawsession"}]

    readonly property var headersMap: {
        var m = {};
        headers.forEach(function (h) {
            if (h.file in m) {
                throw "Duplicated file in headers property: " + h.file;
            }
            m[h.file] = h;
        });
        return m;
    }

    FileTagger {
        fileTags: ["export_hpp"]
        patterns: headers.map(function (header) {return header.file;});
    }

    Depends { name: "cpp" }

    Rule {
        inputs: ["export_hpp"]

        outputFileTags: ["hpp"]
        outputArtifacts: {
            var outputs = [];
            var includePath = product.moduleProperty("ExportHeaders", "includePath");

            inputs.forEach(function (h) {
                outputs.push({
                    filePath: FileInfo.joinPaths(includePath, h.module, h.file),
                    fileTags: ["hpp"]
                });

                if (h.alias) {
                    outputs.push({
                        filePath: FileInfo.joinPaths(includePath, h.module, h.alias),
                        fileTags: ["hpp"]
                    });
                }
            });

            outputs.forEach(function (o) {print(o);});

            return outputs;
        }

        prepare: {
            throw "stop";
            var cmd = new JavaScriptCommand();
            cmd.description = "creating export header " + input.fileName;
            cmd.highlight = "filegen";
            cmd.sourceCode = function () {
                var dst = FileInfo.joinPaths(product.buildDirectory, "include/QWamp", input.fileName);
                File.copy(input.filePath, dst);

                if (!(input.fileName in product.headersMap)) {
                    return;
                }

                var fileName = product.headersMap[input.fileName];
                var f = new TextFile(FileInfo.joinPaths(product.buildDirectory, "include/QWamp", fileName), TextFile.WriteOnly);
                f.writeLine("#include \"" + input.fileName + "\"");
                f.close();
            }

            return cmd;
        }
    }
}
