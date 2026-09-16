/* -------------------------------------------
   FMOD Studio Perforce Integration:
   Script-Based Source Control Provider Plugin
   -------------------------------------------
 */

"use strict";

var ENABLE_PERFORCE_PROVIDER = true; // Set the true to enable

////////////////////////////////////// Implementation of Helper Functions ///////////////////////////////////////////////////

var providerName = "Perforce";

var p4Settings = {
    p4port: "",
    p4host: "",
    p4encoding: "",
    p4user: "",
    p4password: "",
    p4client: "",
    p4Executable: "",
};

var hasValidTicket = false;
var invalidPassword = false;
var logErrorsToConsole = true;

var projectRoot = "";
var notOpenedFilter = /(not opened on this client.)$/i;
var syncFilter = /^(.*) ((- file\(s\) up-to-date.)|(- no such file\(s\).))$/i;

function resetTicketState() {
    hasValidTicket = false;
    invalidPassword = false;
}

function initSettingsFromEnvironmentVariables() {
    var regex = /^(P4.*)\=(.*) \(set\)$/gmi;

    tryRunP4("set", [], function(result) {
        var envVarMap = {};
        envVarMap.P4PORT = "perforce:1666";
        envVarMap.P4CHARSET = "none";
        var tokens;
        while((tokens = regex.exec(result.standardOutput)) !== null) {
            envVarMap[tokens[1]] = tokens[2];
        }

        p4Settings.p4port = envVarMap.P4PORT;
        p4Settings.p4host = envVarMap.P4HOST ? envVarMap.P4HOST : "";
        p4Settings.p4encoding = envVarMap.P4CHARSET;
        p4Settings.p4user = envVarMap.P4USER ? envVarMap.P4USER : "";
        p4Settings.p4password = envVarMap.P4PASSWD ? envVarMap.P4PASSWD : "";
        p4Settings.p4client = envVarMap.P4CLIENT ? envVarMap.P4CLIENT : "";
    });
    resetTicketState();
}

function acquireSessionTicket() {
    if(!hasValidTicket && p4Settings.p4user) {
        var result = runP4Impl("login -s");
        var dict = createKeyValuePairs(result.standardOutput);
        hasValidTicket = dict.TicketExpiration !== undefined && dict.User !== undefined && dict.User === p4Settings.p4user;
        if(!hasValidTicket && !invalidPassword) {
            var noLoginRequiredRegEx = /^'login' not necessary, no password set for this user\./i;
            var loginSucceedRegEx = /^User .* logged in.$/gmi;
            var result = runP4Impl("login", null, p4Settings.p4password);
            hasValidTicket = loginSucceedRegEx.exec(result.standardOutput) !== null || noLoginRequiredRegEx.exec(result.standardOutput) !== null;
        }
    }
}

function setProviderStatus(status) {
    studio.sourceControl.setProviderStatus(providerName, status);
}

function newLineFixup(text) {
    return text.replace(/\r\n/g, "\n");
}

function hidePassword(text) {
    var pattern = /-P \S+/g;
    return text.replace(pattern, "-P *****");
}

function runP4AndMergeResult(processArgs, stdInput, result) {
    var options = { timeout: -1, args: processArgs, standardInput: stdInput, useANSICodePage: true };
    var executable = p4Settings.p4Executable ? p4Settings.p4Executable : (studio.os.platform == "mac" ? "/usr/local/bin/p4" : "p4");
    var newResult = studio.system.start(executable, options);
    result.exitCode = newResult.exitCode;
    result.standardOutput += newLineFixup(newResult.standardOutput);
    result.standardError += newLineFixup(newResult.standardError);
    var p4ConnectionTimeoutRegex = /(exceeded maximum configured duration of|Connect to server failed)/i;
    if(p4ConnectionTimeoutRegex.exec(result.standardError)) {
        throw "The Perforce operation has timed out due to extended period of inactivity. Please make sure the server is online and accessible via your network.";
    }

    if(result.exitCode !== 0) {
        if(logErrorsToConsole) {
            console.error("runP4: error running command: " + hidePassword(processArgs.join(" ")));
            console.error("runP4: " + result.standardError);
        }
        var connectionErrorRegex = /Connect to server failed/gmi;
        var loginRequiredErrorRegex = /^(Perforce password \(P4PASSWD\) invalid or unset\.|Password invalid\.)/gmi;
        if(connectionErrorRegex.exec(result.standardError)) {
            throw result.standardError;
        }
        if(loginRequiredErrorRegex.exec(result.standardError)) {
            invalidPassword = true;
        }
        else {
            hasValidTicket = false;
        }
    }
    return result;
}

function filterError(result, filter) {
    var filteredErrors = [];
    result.standardError.trim().split("\n").map(function(block) {
        var tokens = filter.exec(block);
        if(tokens === null)
            filteredErrors.push(block);
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

function runP4Impl(command, additionalArgs, stdInput) {
    var base = "-z tag -r0 -vnet.maxwait=10"; // output result of command in fstat format and let perforce handle network inactivity
    if(p4Settings.p4port)
        base += " -p " + p4Settings.p4port;
    if(p4Settings.p4host)
        base += " -H " + p4Settings.p4host;
    if(p4Settings.p4encoding)
        base += " -C " + p4Settings.p4encoding;
    if(p4Settings.p4user)
        base += " -u " + p4Settings.p4user;
    if(p4Settings.p4password)
        base += " -P " + p4Settings.p4password;
    if(p4Settings.p4client)
        base += " -c " + p4Settings.p4client;
    base += " " + command;

    if(base.length > 30000) { // limit of commandline minus the terminateing null character plus some for good measure
        throw "runP4: command line exceeds the character limit: " + hidePassword(base);
    }

    if(stdInput && additionalArgs) {
        throw "runP4: batching of command with standard input is not supported";
    }

    var result = { exitCode: 0, standardOutput: "", standardError: "" };
    var processArgs = base.split(" ");
    var argumentLength = base.length;

    for(var index = 0; additionalArgs && index < additionalArgs.length; index++) {
        var argStr = additionalArgs[index].toString();
        if(argumentLength + argStr.length > 30000) {
            if(argumentLength == base.length) {
                throw "runP4: command line exceeds the character limit: " + hidePassword(processArgs.join(" ") + " " + argStr);
            }
            if(command == "move") { // batch operation is not support for move
                throw "runP4: source and target filename exceed the character limits for move";
            }
            result = runP4AndMergeResult(processArgs, stdInput, result);
            if(result.exitCode !== 0) {
                return result;
            }
            processArgs = base.split(" ");
            argumentLength = base.length;
        }
        processArgs.push(argStr);
        argumentLength += argStr.length;
    }

    return runP4AndMergeResult(processArgs, stdInput, result);
}

function runP4(command, additionalArgs, stdInput) {
    var canRunCommand = command == "users" || command == "set" || hasValidTicket;
    if(!canRunCommand) {
        acquireSessionTicket();
        canRunCommand = hasValidTicket;
    }

    return canRunCommand ? runP4Impl(command, additionalArgs, stdInput) : { exitCode: -1, standardOutput: "", standardError: "Failed to connect to Perforce server. Please ensure the settings are correct." };
}

function tryRunP4(command, additionalArgs, callback) {
    var standardError = undefined;
    logErrorsToConsole = false;
    try {
        var result = runP4(command, additionalArgs);
        if(result.standardError.length === 0) {
            callback(result);
        }
        else {
            standardError = result.standardError;
        }
    }
    catch(err) {
        if(err.errorCode === studio.system.processError.FailedToStart) {
            standardError = "Failed to start the Perforce command-line client. Please make sure the Perforce command-line client has been installed on your system.";
        }
        else {
            standardError = err.errorString;
        }
    }
    finally {
        logErrorsToConsole = true;
    }

    if(standardError !== undefined) {
        setProviderStatus(standardError);
    }
    return standardError;
}

function throwOnError(result) {
    if(result.exitCode !== 0 || result.standardError.length) {
        throw result.standardError;
    }
}

function p4StringToAction(str) {
    var mapping = {
        "edit": studio.sourceControl.action.Edit,
        "add": studio.sourceControl.action.Add,
        "delete": studio.sourceControl.action.Delete,
        "move/add": studio.sourceControl.action.MovedTo,
        "move/delete": studio.sourceControl.action.MovedFrom,
        "branch": studio.sourceControl.action.None,
        "integrate": studio.sourceControl.action.None
    };

    var result = mapping[str];
    if(typeof result === 'number')
        return result;
    else
        throw "Unknown p4 action " + str;
}

function createKeyValuePairs(input) {
    var result = {};

    var regex = /^(\.{3}) (\S+) (.*)$/gmi;
    var tokens;
    while((tokens = regex.exec(input)) !== null) {
        if(tokens.index === regex.lastIndex) {
            regex.lastIndex++;
        }

        var key = tokens[2];
        var value = tokens[3];
        result[key] = value;
    }

    return result;
}

function searchStrings(clientDescription, filter) {
    var result = [];

    Object.keys(clientDescription).map(function(property) {
        if(property.search(/^View\d+$/) === 0) {
            var regex = /^(\/\/.*) (\/\/.*)$/gmi;
            var tokens;
            if((tokens = regex.exec(clientDescription[property])) !== null) {
                var depotPath = tokens[1];
                result.push(depotPath + "/" + filter);
            }
        }
    });

    return result;
}

function pathsToPerforceStylePaths(pathsToConvert) {
    var perforceStylePaths = [];
    pathsToConvert.map(function(path) {
        var stringPath = path.toString();
        if(stringPath.charAt(stringPath.length - 1) == '/') {
            perforceStylePaths.push(stringPath + "...");
        }
        else if(stringPath.length) {
            perforceStylePaths.push(stringPath);
        }
    });

    return perforceStylePaths;
}

function createFstat(input) {
    var result = studio.sourceControl.fileStat.createFileStat();
    var otherUserStats = [];

    var regex = /^(\.{3} \.{3}|\.{3}) (\S+)(( .*)?)$/gmi; // group the space with tokens[3] since ourLock doesn't come with a space
    var tokens;
    while((tokens = regex.exec(input)) !== null) {
        if(tokens.index === regex.lastIndex) {
            regex.lastIndex++;
        }

        var dots = tokens[1];
        var key = tokens[2];
        var value = tokens[3].trim();

        if(dots === "...") {
            switch(key) {
                case "depotFile": result.depotFile = value; break;
                case "movedFile": result.movedFile = value; break;
                case "clientFile": result.clientFile = value.replace(/\\/gi, "/"); break;
                case "headRev": result.headRevision = value; break;
                case "haveRev":
                case "movedRev":
                    result.haveRevision = value; break;
                case "resolveBaseRev0": result.baseRevision = value; break;
                case "headAction": result.headAction = p4StringToAction(value); break;
                case "action": result.action = p4StringToAction(value); break;
                case "headChange": result.headChange = value; break;
                case "ourLock": result.locked = true; break;
                case "isMapped": result.isMapped = true; break;
                case "unresolved": result.hasUnresolvedConflicts = true; break;
                default:
                    break; // unused
            }
        }
        else if(dots === "... ...") {
            if(key.search(/^otherOpen\d+$/) === 0)
                otherUserStats.push({ otherUser: value });
            else if(key.search(/^otherAction\d+$/) === 0)
                otherUserStats[otherUserStats.length - 1].otherAction = p4StringToAction(value);
            else if(key.search(/^otherLock\d+$/) === 0)
                otherUserStats[otherUserStats.length - 1].otherLocked = value;
            else if(key.search(/^otherChange\d+$/) === 0)
                otherUserStats[otherUserStats.length - 1].otherChangelist = value;
            else
                ; // unused
        }
    }

    result.otherUserStats = otherUserStats;
    return result;
}

////////////////////////////////////// Implementation of Source Control Functionality ///////////////////////////////////////////////////

function fstat(files) {
    var fstats = [];
    var standardError = tryRunP4("fstat -Or", pathsToPerforceStylePaths(files), function(result) {
        if(result.standardOutput) {
            result.standardOutput.trim().split("\n\n").forEach(function(block) {
                fstats.push(createFstat(block));
            });
        }
    });
    if(standardError) {
        standardError.trim().split("\n").forEach(function(block) {
            var noSuchFileRegex = /^(.*) (- no such file\(s\).)$/gmi;
            var notUnderClientsRootRegex = /^Path '.*' is not under client's root/gmi;
            var tokens = noSuchFileRegex.exec(block);
            if(tokens !== null) {
                var whereResult = runP4("where", [tokens[1]]);
                // convert location from local disk path to server path
                whereResult.standardOutput.trim().split("\n\n").forEach(function(block) {
                    var dict = createKeyValuePairs(block);
                    if(dict.unmap === undefined && dict.depotFile && dict.path) {
                        fstats.push(createFstat("... clientFile " + dict.path.replace(/\\/gi, "/") + "\n" + "... depotFile " + dict.depotFile));
                    }
                    else {
                        fstats.push(createFstat("... clientFile " + tokens[1]));
                    }
                });
            }
            else if((tokens = notUnderClientsRootRegex.exec(block)) !== null) {
                fstats.push(createFstat("... clientFile " + tokens[1]));
            }
        });
        if(fstats.length === 0) {
            files.forEach(function(path) {
                var stringPath = path.toString();
                if(stringPath.charAt(stringPath.length - 1) !== '/') {
                    fstats.push(createFstat("... clientFile " + path));
                }
            });
        }
    }

    return fstats;
}

function openedFiles(pathsToCheck) {
    var openedFilter = /^(.*) (is not under client's root)/i;
    var result = filterError(runP4("opened", pathsToPerforceStylePaths(pathsToCheck)), openedFilter);
    throwOnError(result);

    var realMerge = function(to, from) {
        var n;
        for(n in from) {
            if(typeof to[n] != 'object') {
                to[n] = from[n];
            } else if(typeof from[n] == 'object') {
                to[n] = realMerge(to[n], from[n]);
            }
        }

        return to;
    };
    var fstats = [];
    result.standardOutput.trim().split("\n\n").map(function(block) {
        var stat = createFstat(block);
        if(stat.depotFile) {
            fstats.push(realMerge(studio.sourceControl.fileStat.createFileStat(), stat));
        }
    });

    return fstats;
}

function checkout(paths, addIfNotInDepot) {
    var files = [];
    paths.forEach(function(path) {
        var stringPath = path.toString();
        if(stringPath.charAt(stringPath.length - 1) != '/') {
            files.push(path);
        }
    });
    var result = runP4("edit", files);

    if(!addIfNotInDepot && result.standardError.length) {
        throw result.standardError;
    }

    var notOnClientFiles = [];
    result.standardError.trim().split("\n").map(function(block) {
        var regex = /^(.*) (- file\(s\) not on client.)$/gmi;
        var tokens = regex.exec(block);
        if(tokens !== null) {
            notOnClientFiles.push(tokens[1]);
        }
    });

    if(addIfNotInDepot && notOnClientFiles.length) {
        var addResult = runP4("add", notOnClientFiles);
        throwOnError(addResult);
    }

    return fstat(paths);
}

function rename(fromFiles, toFiles) {
    if(!fromFiles.length || fromFiles.length === 0 || !toFiles.length || toFiles.length === 0) {
        throw "No files to rename";
    }

    if(fromFiles.length != toFiles.length) {
        throw "Source and destination file list length mismatch.";
    }

    var index = 0;
    fromFiles.forEach(function(file) {
        var result = runP4("edit", [file]);
        throwOnError(result);
        result = runP4("move", [file.toString(), toFiles[index].toString()]);
        throwOnError(result);
        index++;
    });

    return fstat(fromFiles.concat(toFiles));
}

function remove(files) {
    var result = runP4("delete", files);
    throwOnError(result);
    return fstat(files);
}

function revert(files, revertIfUnchanged) {
    var command = "revert";
    if(revertIfUnchanged) {
        command += " -a";
    }
    var depotPathOfOpenedFiles = [];
    openedFiles(files).map(function(stat) {
        // if revert if unchanged, ignore files marked for add, Perforce treats that as an error which for Studio is not
        if(!revertIfUnchanged || stat.action == studio.sourceControl.action.Edit) {
            depotPathOfOpenedFiles.push(stat.depotFile);
        }
    });
    var filesToRevert = [];
    if(depotPathOfOpenedFiles.length) {
        fstat(depotPathOfOpenedFiles).map(function(stat) {
            filesToRevert.push(stat.clientFile);
        });
    }

    if(filesToRevert.length) {
        var result = filterError(runP4(command, filesToRevert), notOpenedFilter);
        throwOnError(result);
        return fstat(filesToRevert);
    }
    return [];
}

function lock(files) {
    // in Perforce you can only lock files that are checked out
    var stats = fstat(files);
    var filesToCheckout = [];
    stats.map(function(fs) {
        if(fs.action == studio.sourceControl.action.None) {
            filesToCheckout.push(fs.clientFile);
        }
    });
    if(filesToCheckout.length) {
        checkout(filesToCheckout, false);
    }

    var result = runP4("lock", files);
    throwOnError(result);
    var errorMessages = [];
    fstat(files).map(function(stat) {
        if(!stat.locked) {
            errorMessages.push("Failed to lock " + stat.clientFile);
        }
    });
    if(errorMessages.length) {
        unlock(files);
        if(filesToCheckout.length) {
            revert(filesToCheckout, true);
        }
        throw errorMessages.join("\n");
    }

    return fstat(files);
}

function unlock(files) {
    var result = filterError(runP4("unlock", files), notOpenedFilter);
    throwOnError(result);
    revert(files, true); // Perforce specific, since checkout is required for locking
    return fstat(files);
}

function print(fileToPrint, revisionToPrint, outputFilePath) {
    var result = runP4("print -q -o", [outputFilePath, revisionToPrint > 0 ? fileToPrint + "#" + revisionToPrint : fileToPrint]);
    throwOnError(result);
}

function pendingChanges(paths) {
    var depotPathOfOpenedFiles = [];

    openedFiles(paths).map(function(stat) {
        depotPathOfOpenedFiles.push(stat.depotFile);
    });

    if(depotPathOfOpenedFiles.length) {
        return fstat(depotPathOfOpenedFiles);
    }
}

function syncPreview(paths) {
    var result = filterError(runP4("sync -n", pathsToPerforceStylePaths(paths)), syncFilter);
    throwOnError(result);

    var items = [];
    if(result.standardOutput && result.standardOutput.length) {
        result.standardOutput.trim().split("\n\n").map(function(block) {
            var dict = createKeyValuePairs(block);
            if(dict.depotFile && dict.depotFile.length) {
                items.push(dict.depotFile);
            }
        });
        result.standardOutput.trim().split("\n").map(function(block) {
            var regex = /^(.+)#\d+ - is opened/gmi;
            var tokens = regex.exec(block);
            if(tokens !== null) {
                items.push(tokens[1]);
            }
        });
    }

    return items.length ? fstat(items) : [];
}

function sync(fileList) {
    var result = filterError(runP4("sync", pathsToPerforceStylePaths(fileList)), syncFilter);
    throwOnError(result);
    var items = [];
    if(result.standardOutput && result.standardOutput.length) {
        result.standardOutput.trim().split("\n\n").map(function(block) {
            var dict = createKeyValuePairs(block);
            if(dict.depotFile && dict.depotFile.length) {
                items.push(dict.depotFile);
            }
        });
        result.standardOutput.trim().split("\n").map(function(block) {
            var regex = /^(.+)#\d+ - is opened/gmi;
            var tokens = regex.exec(block);
            if(tokens !== null) {
                items.push(tokens[1]);
            }
        });
    }

    return items.length ? fstat(items) : [];
}

function resolve(acceptYours, acceptTheirs, acceptMerge) {
    var stats = [];
    if(acceptYours.length) {
        throwOnError(runP4("resolve -ay", acceptYours));
        throwOnError(runP4("add -d", acceptYours));
        stats.concat(fstat(acceptYours));
    }
    if(acceptTheirs.length) {
        stats.concat(revert(acceptTheirs, false));
    }
    if(acceptMerge.length) {
        throwOnError(runP4("resolve -ay", acceptMerge));
        stats.concat(fstat(acceptMerge));
    }

    return stats;
}

function identifyLocalChanges(directoryList, fileList) {
    var result = [];
    // Studio always passes path that is directory with terminating "/". Perforce doesn't like that as input for its where command so we have to strip it here
    var dirList = directoryList.map(function(directory) { return directory.toString().replace(/^\/?|\/?$/g, ""); });
    var whereResult = runP4("where", dirList);
    throwOnError(whereResult);
    var mappedDirectories = [];
    var filesToReconcile = fileList.map(function(file) { return file.toString(); });

    // convert location from local disk path to server path
    whereResult.standardOutput.trim().split("\n\n").forEach(function(block) {
        var dict = createKeyValuePairs(block);
        if(dict.unmap === undefined && dict.depotFile) {
            mappedDirectories.push(dict.depotFile + "/...");
        }
    });

    // find all the files in the various directories
    mappedDirectories.forEach(function(directory) {
        var filesResult = runP4("files", [directory]);
        // ignore no such file(s) on Perforce because the directory might be empty or is not under source control
        if(filesResult.standardError.length && filesResult.standardError.indexOf("- no such file(s).") == -1) {
            throwOnError(filesResult);
        }
        filesResult.standardOutput.trim().split("\n\n").forEach(function(block) {
            var dict = createKeyValuePairs(block);
            if(dict.action != studio.sourceControl.action.Delete && dict.depotFile && dict.depotFile.length) {
                filesToReconcile.push(dict.depotFile);
            }
        });
    });

    if(filesToReconcile.length) {
        var fileStats = fstat(filesToReconcile);
        var filesToAddOrCheckout = [];
        var filesToDelete = [];
        fileStats.forEach(function(stat) {
            if(stat.headRevision > 0 && stat.headAction !== studio.sourceControl.action.Delete) {
                // perform a diff using perforce
                var diffResult = runP4("diff -f -sl", ["{0}#{1}".format(stat.clientFile, stat.haveRevision)]);
                var diffStatus = createKeyValuePairs(diffResult.standardOutput).status;
                if(diffStatus == "diff") { // the file is difference from server, add it to checkout list
                    filesToAddOrCheckout.push(stat.clientFile);
                }
                else if(diffStatus == "missing") { // file is not on local disk, mark it for delete
                    filesToDelete.push(stat.clientFile);
                }
            }
            else { // file is not in server, mark it for add
                filesToAddOrCheckout.push(stat.clientFile);
            }
        });

        if(filesToAddOrCheckout.length) {
            result = result + checkout(filesToAddOrCheckout, true); // checkout or add all the files
        }
        if(filesToDelete.length) {
            result = result + remove(filesToDelete); // remove the files
        }
    }

    return result;
}

function checkin(changeDescription, filesToCheckin, checkinKeyFile) {
    // create a new perforce changelist and passing in the changeDescription via standardInput
    var result = runP4("change -i", null, "Change: new\nDescription: " + changeDescription.replace(/\n/g, "\n\t") + "\n");
    throwOnError(result);

    // extract the changelist number and move filesToCheckin to the new changelist
    var regex = /^Change (.*) created.$/gmi;
    var changelistNumber = regex.exec(result.standardOutput)[1];
    throwOnError(runP4("reopen -c " + changelistNumber, filesToCheckin));

    // revert submit key file if it is unmodified because Perforce does not allow locking without checking out
    var stats = fstat([checkinKeyFile]);
    if(stats[0].action == studio.sourceControl.action.Edit)
        revert([checkinKeyFile], true);

    // submit the changelist
    throwOnError(runP4("submit -c " + changelistNumber));

    return fstat(filesToCheckin);
}

function findFiles(filter) {
    var result = runP4("client -o");
    var clientDescription = createKeyValuePairs(result.standardOutput);

    var fstats = [];
    var result = runP4("fstat -Or", searchStrings(clientDescription, filter));
    if(result.standardOutput) {
        result.standardOutput.trim().split("\n\n").forEach(function(block) {
            fstats.push(createFstat(block));
        });
    }

    return fstats;
}

function availableUsers() {
    var users = [];
    var index = 0;
    var currentIndex = -1;
    tryRunP4("users", [], function(result) {
        result.standardOutput.trim().split("\n\n").map(function(block) {
            var dict = createKeyValuePairs(block);
            if(dict.User) {
                users.push({ text: dict.User });
            }
            if(p4Settings.p4user && dict.User == p4Settings.p4user) {
                currentIndex = index;
            }
            index++;
        });

        if(currentIndex == -1) {
            // provide user some feedback on how to set up the connection
            if(users.length) {
                setProviderStatus("Please select an appropriate user from the drop down.");
            }
            else {
                setProviderStatus("There is no available user for the current connection settings.");
            }
        }
    });
    return { items: users, currentIndex: currentIndex };
}

function availableClients() {
    var clients = [];
    var index = 0;
    var currentIndex = -1;
    if(p4Settings.p4user && p4Settings.p4user.length > 0) {
        tryRunP4("clients", ["-u", p4Settings.p4user], function(result) {
            result.standardOutput.trim().split("\n\n").map(function(block) {
                var dict = createKeyValuePairs(block);
                if(!p4Settings.p4host || !dict.Host || p4Settings.p4host == dict.Host) {
                    if(dict.client) {
                        clients.push({ text: dict.client });
                    }
                    if(p4Settings.p4client && dict.client == p4Settings.p4client) {
                        currentIndex = index;
                    }
                    index++;
                }
            });

            if(currentIndex == -1) {
                // provide user some feedback on how to set up the connection
                if(clients.length) {
                    setProviderStatus("Please select an appropriate existing workspace from the drop down.");
                }
                else if(p4Settings.p4host) {
                    setProviderStatus("There is no available workspace for the host \"" + p4Settings.p4host + "\".");
                }
                else {
                    setProviderStatus("There is no available workspace for the user \"" + p4Settings.p4user + "\".");
                }
            }
            else {
                setProviderStatus(undefined);
            }
        });
    }
    return { items: clients, currentIndex: currentIndex };
}

function refreshSettings(widget) {
    var userCombo = widget.findWidget("m_userCombo");
    var workspaceCombo = widget.findWidget("m_workspaceCombo");

    p4Settings.p4port = widget.findWidget("m_serverText").text();
    p4Settings.p4host = widget.findWidget("m_hostText").text();
    p4Settings.p4password = widget.findWidget("m_passwordText").text();
    p4Settings.p4encoding = widget.findWidget("m_encodingCombo").currentUserData();
    p4Settings.p4user = userCombo.currentText();
    p4Settings.p4Executable = widget.findWidget("m_executablePathLineEdit").text();

    if(widget.widgetId() != userCombo.widgetId() && widget.widgetId() != workspaceCombo.widgetId()) {
        var usersAndCurrentIndex = availableUsers();
        userCombo.setItems(usersAndCurrentIndex.items);
        if(usersAndCurrentIndex.currentIndex >= 0) {
            userCombo.setCurrentIndex(usersAndCurrentIndex.currentIndex);
        }
        else if(p4Settings.p4user && p4Settings.p4user.length) {
            userCombo.setEditText(p4Settings.p4user);
        }
    }

    p4Settings.p4user = userCombo.currentText();

    resetTicketState();

    if(widget.widgetId() != workspaceCombo.widgetId()) {
        var availableClientsAndCurrentIndex = availableClients();
        workspaceCombo.setItems(availableClientsAndCurrentIndex.items);
        workspaceCombo.setCurrentIndex(availableClientsAndCurrentIndex.currentIndex);
    }
}

function verifyWorkspaceSelection(widget) {
    p4Settings.p4client = widget.currentText();
    if(projectRoot && projectRoot.length) {
        // check if the project path is within the workspace view
        tryRunP4("where", [projectRoot], function(result) { setProviderStatus(""); });
    }
    else if(p4Settings.p4client && p4Settings.p4client.length) {
        setProviderStatus(undefined);
    }
}

function settingsUIDefinition() {
    var usersAndCurrentIndex = availableUsers();
    var availableClientsAndCurrentIndex = availableClients();
    var encodingOptions = [
                        { text: "Auto", userData: "auto" },
                        { text: "None", userData: "none" },
                        { text: "Unicode (UTF-8)", userData: "utf8-bom" },
                        { text: "Unicode (UTF-8, no BOM)", userData: "utf8" },
                        { text: "Unicode (UTF-16)", userData: "utf16" },
                        { text: "Unicode (UTF-16, no BOM)", userData: "utf16-nobom" },
                        { text: "Unicode (UTF-16, Big-Endian)", userData: "utf16be" },
						{ text: "Unicode (UTF-16, Big-Endian, no BOM)", userData: "utf16be-nobom" },
                        { text: "Unicode (UTF-32)", userData: "utf32" },
                        { text: "Unicode (UTF-32, no BOM)", userData: "utf32-nobom" },
						{ text: "Unicode (UTF-32, Big-Endian)", userData: "utf32be" },
                        { text: "Unicode (UTF-32, Big-Endian, no BOM)", userData: "utf32be-nobom" },
                        { text: "Western European (ISO 8895-1)", userData: "iso8859-1" },
						{ text: "Western European (WinAnsi 1252)", userData: "winansi" },
                        { text: "Western European (Mac OS Roman)", userData: "macosroman" },
                        { text: "Western European (ISO 8895-15)", userData: "iso8895-15" },
                        { text: "Japanese (Shift-JIS)", userData: "shiftjis" },
						{ text: "Japanese (EUC-JP)", userData: "eucjp" },
                        { text: "Cyrillic (ISO 8895-5)", userData: "iso8859-5" },
                        { text: "Cyrillic (KOI8-R)", userData: "koi-r" },
						{ text: "Cyrillic (WinAnsi 1251)", userData: "cp1251" },
                        { text: "Korean (CP949)", userData: "cp949" },
                        { text: "Chinese (CP936 GBK)", userData: "cp936" },
						{ text: "Chinese (CP950 Big5)", userData: "cp950" },
    ];
    var currentEncodingIndex = -1;
    for(var index = 0; index < encodingOptions.length && currentEncodingIndex < 0; index++) {
        if(encodingOptions[index].userData == p4Settings.p4encoding) {
            currentEncodingIndex = index;
        }
    }
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
                text: p4Settings.p4port,
                row: 1,
                column: 2,
                onEditingFinished: function() { refreshSettings(this); },
            },
            { widgetType: studio.ui.widgetType.Label, text: "Host:", alignment: studio.ui.alignment.AlignRight | studio.ui.alignment.AlignVCenter },
            {
                widgetType: studio.ui.widgetType.LineEdit,
                widgetId: "m_hostText",
                text: p4Settings.p4host,
                row: 2,
                column: 2,
                onEditingFinished: function() { refreshSettings(this); },
            },
            { widgetType: studio.ui.widgetType.Label, text: "Character Encoding:", alignment: studio.ui.alignment.AlignRight | studio.ui.alignment.AlignVCenter },
            {
                widgetType: studio.ui.widgetType.ComboBox,
                widgetId: "m_encodingCombo",
                items: encodingOptions,
                currentIndex: currentEncodingIndex,
                row: 3,
                column: 2,
                onCurrentIndexChanged: function() { refreshSettings(this); },
            },
            { widgetType: studio.ui.widgetType.Label, text: "User:", alignment: studio.ui.alignment.AlignRight | studio.ui.alignment.AlignVCenter },
            {
                widgetType: studio.ui.widgetType.ComboBox,
                widgetId: "m_userCombo",
                items: usersAndCurrentIndex.items,
                currentIndex: usersAndCurrentIndex.currentIndex,
                isEditable: true,
                row: 4,
                column: 2,
                onCurrentIndexChanged: function() { refreshSettings(this); },
                onEditingFinished: function() { refreshSettings(this); },
            },
            { widgetType: studio.ui.widgetType.Label, text: "Password (Optional):", alignment: studio.ui.alignment.AlignRight | studio.ui.alignment.AlignVCenter },
			{
			    widgetType: studio.ui.widgetType.LineEdit,
			    widgetId: "m_passwordText",
			    text: p4Settings.p4password,
			    echoMode: studio.ui.echoMode.Password,
			    row: 5,
			    column: 2,
			    onEditingFinished: function() { refreshSettings(this); },
			},
            { widgetType: studio.ui.widgetType.Label, text: "Client Workspace:", alignment: studio.ui.alignment.AlignRight | studio.ui.alignment.AlignVCenter },
			{
			    widgetType: studio.ui.widgetType.ComboBox,
			    widgetId: "m_workspaceCombo",
			    items: availableClientsAndCurrentIndex.items,
			    currentIndex: availableClientsAndCurrentIndex.currentIndex,
			    row: 6,
			    column: 2,
			    onCurrentIndexChanged: function() { verifyWorkspaceSelection(this); },
			},
            { widgetType: studio.ui.widgetType.Label, text: "P4 Executable (Optional):", alignment: studio.ui.alignment.AlignRight | studio.ui.alignment.AlignVCenter },
            {
                widgetType: studio.ui.widgetType.PathLineEdit,
                widgetId: "m_executablePathLineEdit",
                caption: "Select path of P4 executable",
                pathType: studio.ui.pathType.OpenFile,
                text: p4Settings.p4Executable ? p4Settings.p4Executable : "",
                row: 7,
                column: 2,
                onEditingFinished: function() { refreshSettings(this); },
            },
        ],
    }
}

function deserializeSettings(settings) {
    initSettingsFromEnvironmentVariables();
    if(settings && settings.length) {
        p4Settings = JSON.parse(settings);
        resetTicketState();
    }
}

function serializeSettings() {
    return JSON.stringify(p4Settings);
}

function configurationAsString() {
    return p4Settings.p4client + ", " + p4Settings.p4port + ", " + p4Settings.p4user;
}

if(ENABLE_PERFORCE_PROVIDER) {
    studio.sourceControl.registerProvider(providerName, {
        separator: function() { return "/"; },
        repositoryPathPrefix: function() { return "//"; },
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

if(0) // To enable menu test, you will need to rename script file to extension .js instead of .sc.js
{
    studio.menu.addMenuItem({
        name: "Perforce Settings UI Test", execute: function() {
            studio.ui.showModalDialog(settingsUIDefinition());
        }
    });
}
