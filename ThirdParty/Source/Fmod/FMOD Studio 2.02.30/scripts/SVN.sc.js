/* -------------------------------------------
   FMOD Studio SVN Integration:
   Script-Based Source Control Provider Plugin
   -------------------------------------------
 */

var ENABLE_SVN_PROVIDER = true; // Set the true to enable

////////////////////////////////////// Implementation of Helper Functions ///////////////////////////////////////////////////

var providerName = "SVN";

var svnSettings = {
    svnserver: "",
    svnusername: "",
    svnpassword: "",
    svnExecutable: "",
};

var projectRoot = "";
var globalErrorFilter = null;

function setProviderStatus(status) {
    studio.sourceControl.setProviderStatus(providerName, status);
}

function newLineFixup(text) {
    return text.replace(/\r\n/g, "\n");
}

function runWithGlobalErrorFilter(commands, filter) {
    try {
        globalErrorFilter = filter;
        return commands();
    }
    finally {
        globalErrorFilter = null;
    }
}

function hidePassword(text) {
    var pattern = /--password \S+/g;
    return text.replace(pattern, "--password *****");
}

function runSVNAndMergeResult(processArgs, stdInput, result) {
    var options = { timeout: 100000, args: processArgs, standardInput: stdInput }; // timeout is arbitary, depending on your server, you might need longer timeout period
    var executable = svnSettings.svnExecutable ? svnSettings.svnExecutable : (studio.os.platform == "mac" ? "/usr/bin/svn" : "svn");
    try {
        var newResult = studio.system.start(executable, options);
        result.exitCode = newResult.exitCode;
        result.standardOutput += newLineFixup(newResult.standardOutput);
        result.standardError += newLineFixup(newResult.standardError);
    }
    catch(err) {
        result.exitCode = -1;
        if(err.errorCode === studio.system.processError.FailedToStart) {
            result.standardError = "Failed to start the SVN command-line client. Please make sure the SVN command-line client has been installed on your system.";
        }
        else {
            result.standardError = err.errorString;
        }
    }

    if(globalErrorFilter) {
        result = filterError(result, globalErrorFilter);
    }
    if(result.exitCode !== 0) {
        console.error("runSVN: error running command: " + hidePassword(processArgs.join(" ")));
        console.error("runSVN: " + result.standardError);
    }
    return result;
}

function runSVN(command, additionalArgs, stdInput) {
    var base = "--non-interactive"; // disable all interactive prompting
    if(svnSettings.svnusername) {
        base += " --username " + svnSettings.svnusername;
        base += " --password " + svnSettings.svnpassword;
    }
    base += " " + command;

    if(base.length > 30000) { // limit of commandline minus the terminateing null character plus some for good measure
        throw "runSVN: command line exceeds the character limit: " + hidePassword(base);
    }

    if(stdInput && additionalArgs) {
        throw "runSVN: batching of command with standard input is not supported";
    }

    var result = { exitCode: 0, standardOutput: "", standardError: "" };
    var processArgs = base.split(" ");
    var argumentLength = base.length;

    for(var index = 0; additionalArgs && index < additionalArgs.length; index++) {
        var argStr = additionalArgs[index].toString();
        if(argumentLength + argStr.length > 30000) {
            if(argumentLength == base.length) {
                throw "runSVN: command line exceeds the character limit: " + hidePassword(processArgs.join(" ") + " " + argStr);
            }
            if(command == "move" || command == "commit") { // batch operation is not support for move and commit
                throw "runSVN: source and target filename exceed the character limits for move";
            }
            result = runSVNAndMergeResult(processArgs, stdInput, result);
            if(result.exitCode !== 0) {
                return result;
            }
            processArgs = base.split(" ");
            argumentLength = base.length;
        }
        processArgs.push(argStr);
        argumentLength += argStr.length;
    }

    return runSVNAndMergeResult(processArgs, stdInput, result);
}

function throwOnError(result) {
    if(result.exitCode !== 0 || result.standardError.length) {
        throw result.standardError;
    }
}

function filterError(result, filter) {
    var filteredErrors = [];
    result.standardError.trim().split("\n").map(function(block) {
        var tokens = filter.exec(block);
        if(tokens === null) {
            filteredErrors.push(block);
        }
    });
    if(filteredErrors.length) {
        result.standardError = filteredErrors.join("\n");
    }
    else {
        result.exitCode = 0;
        result.standardError = "";
    }

    return result;
}

function createKeyValuePairs(input) {
    var result = {};

    var regex = /^(.*): (.*)$/gmi;
    var tokens;
    while((tokens = regex.exec(input)) !== null) {
        if(tokens.index === regex.lastIndex) {
            regex.lastIndex++;
        }

        var key = tokens[1];
        var value = tokens[2];
        result[key] = value;
    }

    return result;
}

function fullPathFromRelative(referencePath, relativePath) {
    // easier if we expose some helper in Studio....
    return relativePath;
}

function createFstat(input) {
    var result = studio.sourceControl.fileStat.createFileStat();
    var otherUserStats = [];
    var userAction = {
        ' ': studio.sourceControl.action.None,
        'A': studio.sourceControl.action.Add,
        'D': studio.sourceControl.action.Delete,
        'M': studio.sourceControl.action.Edit,
        'R': studio.sourceControl.action.Add, // for delete and readd?
        'C': studio.sourceControl.action.Edit, // conflicting content
        'X': studio.sourceControl.action.None, // item is present because of an external definitions
        'I': studio.sourceControl.action.None, // item is being ignored
        '?': studio.sourceControl.action.None, // not under source control
        '!': studio.sourceControl.action.None, // item has been moved/delete without svn
    };
    // 1. action, 2. property, 3. svnlock, 4. history, 5. switched, 6. userlock, 7. conflict, 8. uptodate, 9. working revision, 10. last committed revision, 11. last committed author, 12. working copy path
    var statusRegex = /^(.)(.)(.)(.)(.)(.)(.)(.)(.)\s*(\S*)\s*(\S*)\s*(\S*)\s*(.*)$/mi;
    var tokens;
    if((tokens = statusRegex.exec(input)) !== null) {
        result.action = userAction[tokens[1]];
        result.isMapped = tokens[1] != '?';
        result.locked = tokens[6] == 'K';
        result.hasUnresolvedConflicts = tokens[7] == 'C';
        result.headChange = tokens[11];
        result.haveRevision = tokens[11];
        result.clientFile = tokens[13];
    }
    var movedFromRegex = /(\> moved from (.*))/mi;
    if((tokens = movedFromRegex.exec(input)) !== null) {
        result.action = studio.sourceControl.action.MovedTo;
        result.movedFile = fullPathFromRelative(result.clientFile, tokens[1]);
    }
    var movedToRegex = /(\> moved to (.*))/mi;
    if((tokens = movedToRegex.exec(input)) !== null) {
        result.action = studio.sourceControl.action.MovedFrom;
    }

    result.otherUserStats = otherUserStats;
    return result;
}

////////////////////////////////////// Implementation of Source Control Functionality ///////////////////////////////////////////////////

function infoFromBlock(block) {
    var dict = createKeyValuePairs(block);
    var stat = studio.sourceControl.fileStat.createFileStat();
    if(dict['Node Kind'] == "file") {
        stat.otherUserStats = [];
        stat.depotFile = dict.URL;
        stat.headRevision = dict['Last Changed Rev'];
        if(dict['Working Copy Root Path'] && dict['Working Copy Root Path'].length && dict.Path && dict.Path.length) {
            stat.clientFile = dict.Path;
            stat.isMapped = true;
            stat.haveRevision = stat.headRevision;
        }

        if(dict['Lock Owner'] && dict['Lock Owner'] != svnSettings.svnusername) {
            stat.otherUserStats.push({ otherUser: dict['Lock Owner'], otherLocked: true });
        }
        if(dict['Moved From'] !== undefined) {
            stat.movedFile = dict['Moved From'];
        }
    }

    return stat;
}

function info(files) {
    var stats = [];
    var statsInDepot = {};
    var filesToQuery = [];
    files.forEach(function(file) {
        var stringPath = file.toString();
        if(stringPath.indexOf('/', stringPath.length) === -1) {
            filesToQuery.push(stringPath);
        }
    });
    if(filesToQuery.length) {
        var result = runSVN("info", filesToQuery);
        result.standardOutput.trim().split("\n\n").forEach(function(block) {
            var fileStat = infoFromBlock(block);
            if(fileStat.clientFile.length > 0) {
                fileStat.clientFile = fileStat.clientFile.replace(/\\/g, '/');
            }
            if(fileStat.depotFile.length > 0) {
                if(fileStat.depotFile.indexOf(svnSettings.svnserver) === 0) {
                    statsInDepot[fileStat.depotFile] = fileStat;
                }
                else {
                    var notInServerStat = studio.sourceControl.fileStat.createFileStat();
                    notInServerStat.clientFile = fileStat.clientFile;
                    stats.push(notInServerStat);
                }
            }
        });
        result.standardError.trim().split("\n\n").forEach(function(block) {
            if(block.length) {
                var fileStat = studio.sourceControl.fileStat.createFileStat();
                var errorRegex = /'(.*)'/g;
                var tokens;
                if((tokens = errorRegex.exec(block)) !== null) {
                    var path = tokens[1];
                    if(path.indexOf(svnSettings.svnserver) === 0) {
                        fileStat.depotFile = path;
                    }
                    else {
                        fileStat.clientFile = path.replace(/\\/g, '/');
                    }
                    stats.push(fileStat);
                }
            }
        });
        // run info another time to retrieve the head revision
        result = runSVN("info -r HEAD", filesToQuery);
        result.standardOutput.trim().split("\n\n").forEach(function(block) {
            if(block.length) {
                var fileStat = infoFromBlock(block);
                if(fileStat.depotFile && fileStat.depotFile.length) {
                    if(fileStat.depotFile in statsInDepot) {
                        statsInDepot[fileStat.depotFile].headRevision = fileStat.headRevision;
                    }
                }
            }
        });
        result.standardError.trim().split("\n\n").forEach(function(block) {
            if(block.length) {
                var incorrectPasswordRegex = /Password incorrect/gmi;
                if(incorrectPasswordRegex.exec(block) !== null) {
                    filesToQuery.forEach(function(file) {
                        var stat = studio.sourceControl.fileStat.createFileStat();
                        stat.clientFile = file;
                        stats.push(stat);
                    });
                    statsInDepot = {};
                }
            }
        });
    }

    Object.keys(statsInDepot).forEach(function(key) {
        stats.push(statsInDepot[key]);
    });
    return stats;
}

function fstat(files) {
    var fstats = info(files);
    var filesToQuery = [];
    fstats.forEach(function(stat) {
        if(stat.isMapped) {
            filesToQuery.push(stat.clientFile);
        }
    });
    var statusFileStat = {};
    if(filesToQuery.length) {
        var result = runSVN("status --show-updates --verbose", filesToQuery);
        result.standardOutput.split("\n").forEach(function(block) {
            var stat = createFstat(block);
            if(stat.clientFile && stat.clientFile.length) {
                stat.clientFile = stat.clientFile.replace(/\\/g, '/');
                statusFileStat[stat.clientFile] = stat;
            }
        });
    }
    fstats = fstats.map(function(stat) {
        if(stat.isMapped) {
            if(stat.clientFile in statusFileStat) {
                var statusStat = statusFileStat[stat.clientFile];
                statusStat.depotFile = stat.depotFile;
                statusStat.otherUsersStats = stat.otherUserStats;
                statusStat.movedFile = stat.movedFile;
                statusStat.headRevision = stat.headRevision;
                if(statusStat.movedFile.length) {
                    statusStat.haveRevision = stat.haveRevision;
                }
                return statusStat;
            }
            else {
                return stat;
            }
        }
        else {
            return stat;
        }
    });

    return fstats;
}

function openedFiles(pathsToCheck) {
    var result = runSVN("status --show-updates --verbose", pathsToCheck); // show only versioned items
    throwOnError(result);
    var changedItems = [];
    // this will return a list of items included folders
    result.standardOutput.split("\n").forEach(function(block) {
        if(block.indexOf("> moved ") == -1) {
            var stat = createFstat(block);
            if(stat.isMapped && stat.action != studio.sourceControl.action.None && stat.clientFile.length) {
                changedItems.push(stat.clientFile);
            }
        }
    });

    return fstat(changedItems); // run fstat again to strip out folders
}

function checkout(files, addIfNotInDepot) {
    var stats = fstat(files);
    // SVN has no concept of check out, just fake it
    var notOnClientFiles = [];
    stats.forEach(function(stat) {
        if(stat.depotFile && stat.depotFile.length) {
            stat.action = studio.sourceControl.action.Edit;
        }
        else {
            notOnClientFiles.push(stat.clientFile);
            if(stat.clientFile.length === 0) {
                throw "Attempting to push empty client file while checking out " + files.join(", ");
            }
        }
    });

    if(notOnClientFiles.length === 0) {
        return stats;
    }

    if(!addIfNotInDepot && notOnClientFiles.length) {
        throw notOnClientFiles.join(", ") + "not found in server.";
    }

    throwOnError(runSVN("add --parents", notOnClientFiles));

    return fstat(files);
}

function rename(fromFiles, toFiles) {
    var currentStats = fstat(fromFiles);
    var newFiles = [];
    var index = 0;
    currentStats.map(function(stat) {
        if(stat.action == studio.sourceControl.action.Add) {
            throwOnError(runSVN("revert", [fromFiles[index]]));
            throwOnError(runSVN("add --parents", [toFiles[index]]));
        }
        else {
            throwOnError(runSVN("move --parents", [fromFiles[index], toFiles[index]]));
        }
        index++;
    });

    return fstat(fromFiles.concat(toFiles));
}

function remove(files) {
    var result = runSVN("delete", files);
    throwOnError(result);
    return fstat(files);
}

function revert(files, revertIfUnchanged) {
    var filesToRevert = [];
    openedFiles(files).map(function(stat) {
        if(!revertIfUnchanged) {
            filesToRevert.push(stat.clientFile);
        }
    });
    if(filesToRevert.length) {
        var result = runSVN("revert", filesToRevert);
        throwOnError(result);
    }

    return fstat(filesToRevert);
}

function lock(files) {
    var stats = fstat(files);
    var filesToCheckout = [];
    var filesToLock = [];
    stats.map(function(fs) {
        if(fs.action == studio.sourceControl.action.None) {
            filesToCheckout.push(fs.clientFile);
        }
        if(fs.action != studio.sourceControl.action.Add && fs.action != studio.sourceControl.action.MovedTo) {
            filesToLock.push(fs.clientFile);
        }
    });
    if(filesToCheckout.length) {
        checkout(filesToCheckout, false);
    }

    var result = runWithGlobalErrorFilter(function() {
        return runSVN("lock", filesToLock);
    }, new RegExp("^(((.*) (is already locked by user '" + svnSettings.svnusername + "'))|((.*) (One or more locks could not be obtained)))", "i"));
    throwOnError(result);
    var errorMessages = [];
    fstat(filesToLock).map(function(stat) {
        if(!stat.locked) {
            errorMessages.push("Failed to lock " + stat.clientFile);
        }
    });
    if(errorMessages.length) {
        unlock(filesToLock);
        if(filesToCheckout.length) {
            revert(filesToCheckout, true);
        }
        throw errorMessages.join("\n");
    }

    return fstat(files);
}

function unlock(files) {
    var result = runWithGlobalErrorFilter(function() {
        return runSVN("unlock", files);
    }, /is not locked in this working copy/);
    throwOnError(result);
    revert(files, true); // Perforce specific, since checkout is required for locking
    return fstat(files);
}

function print(fileToPrint, revisionToPrint, outputFilePath) {
    var cmdArguments = [];
    if(revisionToPrint > 0) {
        cmdArguments.push("-r");
        cmdArguments.push(revisionToPrint);
    }
    cmdArguments.push(fileToPrint);
    var result = runSVN("cat", cmdArguments);
    throwOnError(result);
    var file = studio.system.getFile(outputFilePath);
    if(file.open(studio.system.openMode.WriteOnly)) {
        file.writeText(result.standardOutput);
        file.close();
    }
}

function pendingChanges(paths) {
    var clientPathOfOpenedFiles = [];
    var stats = [];
    openedFiles(paths).map(function(stat) {
        if(stat.clientFile.length === 0) {
            throw "openedFiles return a stat with no clientFile. depotFile=" + stat.depotFile;
        }
        clientPathOfOpenedFiles.push(stat.clientFile);
    });

    if(clientPathOfOpenedFiles.length) {
        stats = fstat(clientPathOfOpenedFiles);
    }
    return stats;
}


function syncPreview(paths) {
    var result = runSVN("status -u", paths);
    throwOnError(result);

    var items = [];
    result.standardOutput.split("\n").map(function(block) {
        var dict = createKeyValuePairs(block);
        if(dict.depotFile && dict.depotFile.length) {
            items.push(dict.depotFile);
        }
    });

    return items.length ? fstat(items) : [];
}

function sync(fileList) {
    var items = [];
    var result = runSVN("status", [projectRoot]);
    if(result.standardError.match('not a working copy')) {
        result = runSVN("checkout", fileList.concat([projectRoot]));
        throwOnError(result);
        var regex = /^A\s+(.+)$/gmi;
        var tokens;
        while((tokens = regex.exec(result.standardOutput)) !== null) {
            items.push(tokens[1]);
        }
    }
    else {
        result = runSVN("update", fileList);
        throwOnError(result);
        result.standardOutput.trim().split("\n").forEach(function(line) {
        });
    }

    return items.length ? fstat(items) : [];
}

function resolve(acceptYours, acceptTheirs, acceptMerge) {
    var stats = [];
    if(acceptYours.length) {
        throwOnError(runSVN("resolve --accept mine-full", acceptYours));
        stats.concat(fstat(acceptYours));
    }
    if(acceptTheirs.length) {
        throwOnError(revert(acceptTheirs, false));
        stats.concat(fstat(acceptTheirs));
    }
    if(acceptMerge.length) {
        var cmdArguments = [];
        throwOnError(runSVN("resolve --accept mine-full", acceptMerge));
        stats.concat(fstat(acceptMerge));
    }

    return stats;
}

function identifyLocalChanges(directoryList, fileList) {
    throw "Identifying Local Changes is currently not supported for SVN.";
    /*
        var result = runSVN("where", directoryList);
        throwOnError(result);
        var mappedDirectories = [];
        var filesToReconcile = [];
        result.standardOutput.trim().split("\n\n").map(function(block) {
            var dict = createKeyValuePairs(block);
            if(dict.unmap == null && dict.depotFile) {
                mappedDirectories.push(dict.depotFile + "/*");
            }
        });
        
        if(mappedDirectories.length) {
            var filesResult = runSVN("files", mappedDirectories);
            throwOnError(filesResult);
            filesResult.standardOutput.trim().split("\n\n").map(function(block) {
                var dict = createKeyValuePairs(block);
                if(dict.change != studio.sourceControl.action.Delete) {
                    filesToReconcile.push(dict.depotFile);
                }
            });
        }
    
        if(filesToReconcile.length) {
            var fileStats = fstat(filesToReconcile);
            var fileStatsMap = [];
            var filesToAdd = [];
            var filesToCheckout = [];
            for(var index=0; index<fileStats.length; index++) {
                fileStatsMap[fileStats[index].clientFile] = fileStats[index];
            }
            for(var index=0; index<filesToReconcile.length; index++) {
                if(filesToReconcile[index] in fileStatsMap) {
                    if(fileStatsMap[filesToReconcile[index]].mDepotFile.length == 0) {
                        filesToAdd.push(filesToReconcile[index]);
                    }
                    else {
                        var diffResult = runSVN("diff", ["-f", "-s1", fileStatsMap[filesToReconcile[index]].clientFile + "#" + fileStatsMap[filesToReconcile[index]].haveRevision]);
                        if(createKeyValuePairs(diffResult.standardOutput).status == "diff") {
                            filesToCheckout.push(filesToReconcile[index]);
                        }
                    }
                }
            }
        }
        */
}

function checkin(changeDescription, filesToCheckin, checkinKeyFile) {
    var changelistName = "FMOD_Studio_Changelist";
    var result = runSVN("status --verbose --show-updates --ignore-externals --changelist " + changelistName);
    result.standardOutput.split("\n").map(function(block) {
        var stat = createFstat(block);
        if(stat.clientFile && stat.clientFile.length) {
            throwOnError(runSVN("changelist --remove", [stat.clientFile]));
        }
    });

    // recurse through the files to find all the directory that are marked for add
    var directoriesToCheckIn = [projectRoot];
    var newlyAddedDirectories = [];
    filesToCheckin.forEach(function(file) {
        var directory = file.toString().replace(/\/[^\/]*$/, '');
        if(directory.indexOf(projectRoot) == -1) {
            directoriesToCheckIn.push(directory);
        }
        while(newlyAddedDirectories.indexOf(directory) == -1 && directory.length && directory.indexOf('/') != directory.lastIndexOf('/')) {
            var result = runSVN("status --depth empty", [directory]); // just retrieve status for the directory
            if(result.standardOutput.charAt(0) == 'A') {
                newlyAddedDirectories.push(directory);
                directory = directory.replace(/\/[^\/]*$/, '');
            }
            else {
                directory = '';
            }
        }
    });

    // move items into new changelist
    throwOnError(runSVN("changelist " + changelistName, filesToCheckin));

    // revert key file
    var stats = fstat([checkinKeyFile]);
    if(stats[0].action == studio.sourceControl.action.Edit)
        revert([checkinKeyFile], true);

    var commitArgs;
    if(newlyAddedDirectories.length) {
        console.log("Committing newly added directories...");
        commitArgs = ["--depth", "empty", "--message", changeDescription];
        throwOnError(runSVN("commit", commitArgs.concat(newlyAddedDirectories)));
    }

    // submit the changelist
    commitArgs = ["--message", changeDescription, "--changelist", changelistName];
    throwOnError(runSVN("commit", commitArgs.concat(directoriesToCheckIn)));

    return fstat(filesToCheckin);
}

function findFiles(filter) {
    var result = runSVN("list", [svnSettings.svnserver, "-R"]);
    var regex = filter;
    regex = regex.replace('.', '\\.');
    regex = regex.replace('?', '.');
    regex = regex.replace('*', '.*');
    var filterResult = [];
    result.standardOutput.trim().split("\n").forEach(function(line) {
        if((new RegExp(regex)).test(line)) {
            filterResult.push(svnSettings.svnserver + "/" + line);
        }
    });

    return info(filterResult);
}

function refreshSettings(widget) {
    svnSettings.svnserver = widget.findWidget("m_serverText").text();
    svnSettings.svnusername = widget.findWidget("m_usernameText").text();
    svnSettings.svnpassword = widget.findWidget("m_passwordText").text();
    svnSettings.svnExecutable = widget.findWidget("m_executablePathLineEdit").text();

    if(!svnSettings.svnserver.length) {
        setProviderStatus("Please enter a valid server address.");
    }
    else if(!svnSettings.svnusername.length) {
        setProviderStatus("Please enter a valid username.");
    }
    else {
        var executableResult = runSVN("list", [svnSettings.svnserver]); // execute the list command to test the username and password
        setProviderStatus(executableResult.standardError.length ? executableResult.standardError : undefined);
    }
}

function settingsUIDefinition() {
    return {
        widgetType: studio.ui.widgetType.Layout,
        widgetId: "m_layout",
        layout: studio.ui.layoutType.GridLayout,
        onConstructed: function() { refreshSettings(this); },
        items: [
            { widgetType: studio.ui.widgetType.Label, text: "Connection Settings", columnSpan: 4 },
            { widgetType: studio.ui.widgetType.Label, text: "Server:", alignment: studio.ui.alignment.AlignRight | studio.ui.alignment.AlignVCenter },
            {
                widgetType: studio.ui.widgetType.LineEdit,
                widgetId: "m_serverText",
                text: svnSettings.svnserver,
                row: 1,
                column: 2,
                onEditingFinished: function() { refreshSettings(this); },
            },
            { widgetType: studio.ui.widgetType.Label, text: "Username:", alignment: studio.ui.alignment.AlignRight | studio.ui.alignment.AlignVCenter },
            {
                widgetType: studio.ui.widgetType.LineEdit,
                widgetId: "m_usernameText",
                text: svnSettings.svnusername,
                row: 2,
                column: 2,
                onEditingFinished: function() { refreshSettings(this); },
            },
            { widgetType: studio.ui.widgetType.Label, text: "Password:", alignment: studio.ui.alignment.AlignRight | studio.ui.alignment.AlignVCenter },
            {
                widgetType: studio.ui.widgetType.LineEdit,
                widgetId: "m_passwordText",
                text: svnSettings.svnpassword,
                echoMode: studio.ui.echoMode.Password,
                row: 3,
                column: 2,
                onEditingFinished: function() { refreshSettings(this); },
            },
            { widgetType: studio.ui.widgetType.Label, text: "SVN Executable (Optional):", alignment: studio.ui.alignment.AlignRight | studio.ui.alignment.AlignVCenter },
            {
                widgetType: studio.ui.widgetType.PathLineEdit,
                widgetId: "m_executablePathLineEdit",
                caption: "Select path of SVN executable",
                pathType: studio.ui.pathType.OpenFile,
                text: svnSettings.svnExecutable ? svnSettings.svnExecutable : "",
                row: 4,
                column: 2,
                onEditingFinished: function() { refreshSettings(this); },
            },
        ],
    }
}

function deserializeSettings(settings) {
    if(settings && settings.length) {
        svnSettings = JSON.parse(settings);
    }
}

function configurationAsString() {
    return svnSettings.svnserver + ", " + svnSettings.svnusername;
}

function serializeSettings() {
    return JSON.stringify(svnSettings);
}

if(ENABLE_SVN_PROVIDER) {
    studio.sourceControl.registerProvider(providerName, {
        separator: function() { return "/"; },
        repositoryPathPrefix: function() { return svnSettings.svnserver; },
        setProjectRoot: function(root) { projectRoot = root; },
        fstat: fstat,
        checkout: checkout,
        rename: rename,
        remove: remove,
        revert: revert,
        lock: lock,
        unlock: unlock,
        print: print,
        pendingChanges: pendingChanges,
        syncPreview: syncPreview,
        sync: sync,
        resolve: resolve,
        identifyLocalChanges: identifyLocalChanges,
        checkin: checkin,
        findFiles: findFiles,
        settingsUIDefinition: settingsUIDefinition,
        deserializeSettings: deserializeSettings,
        serializeSettings: serializeSettings,
        configurationAsString : configurationAsString
    });
}
