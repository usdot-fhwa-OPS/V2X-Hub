var startSaveStateTimer = null;
var saveStatePassphrase = "";

$(document).ready(function () {
    $("#saveStateBtn").on("mousedown", function (e) {
        initializeSaveStatePasswordDialog();
        $("#saveStateBtn").attr("disabled", "true"); // disable to prevent double clicks
        // showSaveStateFeedback("Starting database backup...");
        // // Send command over WebSocket
        // generateAndSendCommandMessage("savestate", []);
        // startSaveStateProgressTimer();
        $("#saveStatePassword").val("");
        $("#saveStatePasswordConfirm").val("");
        $("#saveStatePasswordError").html("");

        $("#saveStatePasswordDialog").dialog("open");
        e.stopPropagation();
    });
});

function startSaveStateProgressTimer() {
    stopSaveStateProgressTimer();
    startSaveStateTimer = setTimeout(() => {
        $("#saveStateFeedback").html("Backup timed out. Please try again.");
        $("#saveStateBtn").removeAttr("disabled"); // re-enable button
    }, 5000); // 5 seconds timeout
}

function stopSaveStateProgressTimer() {
    if (startSaveStateTimer) {
        clearTimeout(startSaveStateTimer);
        startSaveStateTimer = null;
        $("#saveStateBtn").removeAttr("disabled");
    }
}

function showSaveStateFeedback(message) {
    $("#saveStateFeedback").text(message).css("display", "block");
}

function hideSaveStateFeedback() {
    $("#saveStateFeedback").css("display", "none");
}

function initializeSaveStatePasswordDialog() {

    $("#saveStatePasswordDialog").dialog({
        autoOpen: false,
        modal: true,
        width: 450,
        resizable: false,

        buttons: {

            "Save Backup": function () {

                var password =
                    $("#saveStatePassword").val();

                var confirmPassword =
                    $("#saveStatePasswordConfirm").val();

                if (!password) {
                    $("#saveStatePasswordError")
                        .html("Password is required.");
                    return;
                }

                if (password !== confirmPassword) {
                    $("#saveStatePasswordError")
                        .html("Passwords do not match.");
                    return;
                }

                saveStatePassphrase = password;

                $("#saveStatePasswordError").html("");

                $(this).dialog("close");

                showSaveStateFeedback(
                    "Starting database backup..."
                );

                generateAndSendCommandMessage(
                    "savestate",
                    [
                        {
                            name: "passphrase",
                            value: saveStatePassphrase
                        }
                    ]
                );

                startSaveStateProgressTimer();
            },

            "Cancel": function () {

                $("#saveStateBtn")
                    .removeAttr("disabled");

                $("#saveStatePassword").val("");
                $("#saveStatePasswordConfirm").val("");
                $("#saveStatePasswordError").html("");

                $(this).dialog("close");
            }
        }
    });
}