"use strict"; 

var fileUploadLibraryJsVersion = "0.0.1";

var clearUploadProgressTimer = null;
var uploadFileType = null;
var uploadFile = null;
var defaultFileDestPath = "/var/www/download/";

/**
*   Print text to console with a timestamp.
*   @private
*   @param {string} msg - Text to be displayed on console
**/
function log(msg) {
    console.log(timeStamp() + " " + msg);
}

function closeFileUploadDialog() {
    $("#fileButtonOptions > button").removeClass("activeFileOption");
    $("#fileDestPathLabel").css("display", "none");
    $("#fileDestPathInput").css("display", "none");
    $("#fileUploadErrorFeedback").html("");
    clearFileUploadProgress();
    $("#uploadFiles").val(null);
    $("#fileUploadDialog").dialog("close");
    $("#fileUploadForm").css("display", "none");
}

function clearFileUploadProgress() {
    $("#fileUploadProgress").html("");
    $("#fileUploadProgressFeedback").html("");
}

function clearUploadFile() {
    $("#uploadFiles").val(null);
    uploadFile = null;
    uploadFileType = null;
}

function readyFormForSubmitting() {
    $("#fileUploadErrorFeedback").html("");
    $(".ui-dialog-buttonset > .ui-button:contains(\"Submit\")").button("enable");
}

function updateFileUploadFormAction(ip, port) {
    $("#fileUploadForm").attr("action", "https://" + ip + ":" + port + "/upload?upload_progress_id_=12344");
}

function populateFileUploadDialogBasedOnAccessLevel() {
    var accessLevel = $(".permissionsLabel").html();
    if (accessLevel == "(Application Administrator)") {
        $(".fileButtonOptions > button[data-type=\"plugin\"]").css("display", "");
        $(".fileButtonOptions > button[data-type=\"map\"]").css("display", "");
    } else if (accessLevel == "(System Administrator)") {
        $(".fileButtonOptions > button[data-type=\"plugin\"]").css("display", "");
        $(".fileButtonOptions > button[data-type=\"map\"]").css("display", "");
        $(".fileButtonOptions > button[data-type=\"other\"]").css("display", "");
    }
}

function setFileTypesForAndOpenFileUploadForm(type, accessLevel, fileTypeStringList) {
    $(this).addClass("activeFileOption");
    $("#fileButtonOptions > button[data-type=\"" + type + "\"]").addClass("activeFileOption");
    $("#fileButtonOptions > button:not([data-type=\"" + type + "\"])").removeClass("activeFileOption");
    $("#uploadFiles").attr("data-acceptedFileTypes", fileTypeStringList);
    if (type == "other") {
        $("#fileDestPathLabel").css("display", "");
        $("#fileDestPathInput").css("display", "");
    } else {
        $("#fileDestPathLabel").css("display", "none");
        $("#fileDestPathInput").css("display", "none");
    }
    $("#fileUploadErrorFeedback").html("");
    $(".ui-dialog-buttonset > .ui-button:contains(\"Submit\")").button("disable");
    clearUploadFile();
    $("#fileUploadForm").css("display", "");
}

function createFileUploadDialog(accessLevel) {
    var dialog = document.getElementById("fileUploadDialog");
    if (dialog != null && dialog != undefined) {
        if (accessLevel == "2") {
            dialog.innerHTML =  "<div id=\"fileButtonOptions\"><button data-type=\"plugin\" >Upload Plugin</button><button data-type=\"map\">Upload Map</button></div>" + dialog.innerHTML;
        } else if (accessLevel == "3") {
            dialog.innerHTML = "<div id=\"fileButtonOptions\"><button data-type=\"plugin\">Upload Plugin</button><button data-type=\"map\">Upload Map</button><button data-type=\"other\">Upload Other</button></div>" + dialog.innerHTML;
        }

        if (accessLevel == "2") {
            $("#fileButtonOptions > button[data-type=\"plugin\"]").on("mousedown", function () { setFileTypesForAndOpenFileUploadForm("plugin", accessLevel, ".deb,.zip,.tar.gz,.tgz") });
            $("#fileButtonOptions > button[data-type=\"map\"]").on("mousedown", function () { setFileTypesForAndOpenFileUploadForm("map", accessLevel, ".xml,.json,.txt") });
        } else if (accessLevel == "3") {
            $("#fileButtonOptions > button[data-type=\"plugin\"]").on("mousedown", function () { setFileTypesForAndOpenFileUploadForm("plugin", accessLevel, ".deb,.zip,.tar.gz,.tgz") });
            $("#fileButtonOptions > button[data-type=\"map\"]").on("mousedown", function () { setFileTypesForAndOpenFileUploadForm("map", accessLevel, ".xml,.json,.txt") });
            $("#fileButtonOptions > button[data-type=\"other\"]").on("mousedown", function () { setFileTypesForAndOpenFileUploadForm("other", accessLevel, null) });
        }
    }

    $("#uploadFiles").on("change", function (e) {
        var fileName = $(this).val();
        var message = "";
        var validFile = true;
        // Check if filename exceeds 63 characters
        if (fileName.length > 63) {
            var message = "<span style='font-weight:bold;'>Error: </span>Filename cannot exceed 63 characters.";
            message += "<br />The length of " + fileName.substring(fileName.lastIndexOf("\\") + 1) + " is " + fileName.length + ".";
            validFile = false;
        }

        // Check if file is of one of the accepted types.
        var acceptedFileTypesList = $(this).attr("data-acceptedFileTypes");
        if (acceptedFileTypesList != null && acceptedFileTypesList != undefined) {
            var fileTypes = acceptedFileTypesList.split(",");
            if (fileTypes.length > 0) {
                var validFileTypeStatus = false;
                for (var i = 0; i < fileTypes.length; i++) {
                    if (fileName.endsWith(fileTypes[i])) {
                        validFileTypeStatus = true;
                        break;
                    }
                }
                if (!validFileTypeStatus) {
                    if (message.length > 0) {
                        message += "<br /><br />";
                    }
                    message += "<span style='font-weight:bold;'>Error: </span>" + fileName.substring(fileName.lastIndexOf("\\") + 1) + " does not have an accepted file type. The file extension must be ";
                    for (var i = 1; i <= fileTypes.length; i++) {
                        message += fileTypes[i - 1];
                        if (i == fileTypes.length - 1) {
                            message += " or ";
                        } else if (i < fileTypes.length - 1) {
                            message += ", ";
                        }
                    }
                    message += ".";
                }
                validFile = validFile && validFileTypeStatus;
            }
        }
        
        var fileSize = document.getElementById("uploadFiles").files[0].size;
        // Check if file size exceeds maximum accepted file size of 100MB.
        if (fileSize > 100000000) {
            if (message.length > 0) {
                message += "<br /><br />";
            }
            message += "<span style='font-weight:bold;'>Error: </span> File size cannot exceed 100MB. " + fileName.substring(fileName.lastIndexOf("\\") + 1) + " is " + (fileSize / 1000000.0) + "MB.";
            validFile = false;
        }
        
        if (!validFile) {
            $(this).val(null);
            $("#fileUploadErrorFeedback").html(message);
            $(".ui-dialog-buttonset > .ui-button:contains(\"Submit\")").button("disable");
        } else {
            readyFormForSubmitting();
        }
    });

}

function checkIfPluginUploadAndInstall() {
    if (uploadFileType == "plugin") {
        sendInstallCommand(uploadFile.name);
        return true;
    }
    return false;
}

function sendInstallCommand(filename) {
    generateAndSendCommandMessage("plugininstall", [{ name: "pluginfile", value: filename }]);
}

function sendUninstallCommand(plugin) {
    generateAndSendCommandMessage("pluginuninstall", [{ name: "plugin", value: plugin }])
}

function sendFileUploadInitialRequest(uploadFilename, destinationFilename, destinationPath, fileSize) {
    generateAndSendCommandMessage("uploadfile", [{ name: "uploadfilename", value: uploadFilename }, { name: "destinationfilename", value: destinationFilename }, { name: "destinationpath", value: destinationPath }, { name: "filesize", value: fileSize } ]);
}

function stopClearFileUploadProgressTimer() {
    clearTimeout(clearUploadProgressTimer);
    clearUploadProgressTimer = null;
}

function startClearFileUploadProgressTimer() {
    if (clearUploadProgressTimer != null) {
        stopClearFileUploadProgressTimer();
    }
    clearUploadProgressTimer = setTimeout(function () {
        clearFileUploadProgress();
        clearUploadFile();
        $("#uploadFileBtn").removeAttr("disabled");
    }, 3000);
}

$(document).ready(function () {
    // Initialize File Upload Dialog
    $("#fileUploadDialog").dialog({
        title: "File Upload",
        autoOpen: false,
        modal: true,
        width: 800,
        draggable: true,
        resizable: true,
        buttons: {
            "Submit": function () {
                var uploadFiles = document.getElementById("uploadFiles");
                if (uploadFiles.files.length != 1) {
                    // TODO: output message
                    return;
                }
                uploadFile = uploadFiles.files[0];
                var destPath = "";
                var type = $(".activeFileOption").attr("data-type");
                uploadFileType = type;
                if (type == "map") {
                    destPath = defaultFileDestPath + "MAP/";
                } else if (type == "other") {
                    var usrPath = $("#fileDestPathInput").val();
                    if (usrPath.length > 0) {
                        destPath = usrPath;
                    }
                }
                var copyName = "";
                if (destPath != defaultFileDestPath && destPath != "") {
                    copyName = uploadFiles.files[0].name;
                }
                sendFileUploadInitialRequest(uploadFiles.files[0].name, copyName, destPath, uploadFiles.files[0].size);
                $(".ui-dialog-buttonset > .ui-button:contains(\"Submit\")").button("disable");
            },
            "Cancel": function () {
                $("#uploadFileBtn").removeAttr("disabled");
                closeFileUploadDialog();
                clearUploadFile();
            }
        }
    });

    $("#uploadFileBtn").on("mousedown", function (e) {
        $(this).attr("disabled", "true");
        $("#fileDestPathInput").val(defaultFileDestPath);
        populateFileUploadDialogBasedOnAccessLevel();
        $("#fileUploadDialog").dialog("open");
        resizeDialogWindow($(".ui-dialog[aria-describedby=\"fileUploadDialog\"]"));
        $(".ui-dialog-buttonset > .ui-button:contains(\"Submit\")").button("disable");
        e.stopPropagation();
    });

    $("div[aria-describedby=\"fileUploadDialog\"] > div > button[title=\"Close\"]").on("mousedown", function () {
        $("#uploadFileBtn").removeAttr("disabled");
        closeFileUploadDialog();
        clearUploadFile();
    });

});
