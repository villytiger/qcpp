import qbs
import qbs.File
import qbs.FileInfo
import qbs.TextFile

import qbs.ModUtils

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

            inputs.export_hpp.forEach(function (header) {
                var h = product.moduleProperty("ExportHeaders", "headersMap")[header.fileName];
                if (!h) {
                    throw "No header info found for: " + header.filePath;
                }

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

            //outputs.forEach(function (o) {ModUtils.dumpObject(o);});

            return outputs;
        }

        prepare: {
            var cmd = new JavaScriptCommand();
            cmd.description = "creating export header " + input.fileName;
            cmd.highlight = "filegen";
            cmd.sourceCode = function () {
                var includePath = product.moduleProperty("ExportHeaders", "includePath");
                var headersMap = product.moduleProperty("ExportHeaders", "headersMap");

                var h = headersMap[input.fileName];
                if (!h) {
                    throw "No header info found for: " + header.filePath;
                }

                var dst = FileInfo.joinPaths(includePath, h.module, input.fileName);
                File.copy(input.filePath, dst);

                var h = headersMap[input.fileName];
                if (!h.alias) {
                    return;
                }

                var f = new TextFile(FileInfo.joinPaths(includePath, h.module, h.alias), TextFile.WriteOnly);
                f.writeLine("#include \"" + input.fileName + "\"");
                f.close();
            }

            return cmd;
        }
    }
}
