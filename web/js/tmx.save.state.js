var startSaveStateTimer = null;

$(document).ready(function () {
    $("#saveStateBtn").on("mousedown", function (e) {
        $("#saveStateBtn").attr("disabled", "true"); // disable to prevent double clicks
        showSaveStateFeedback("Starting database backup...");
        // Send command over WebSocket
        generateAndSendCommandMessage("savestate", []);
        startSaveStateProgressTimer();
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