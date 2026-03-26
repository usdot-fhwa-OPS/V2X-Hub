$(document).ready(function () {
    
    let saveStateTimeout = null;

    $("#saveStateBtn").on("mousedown", function (e) {
        const $btn = $(this);
         $btn.attr("disabled", "true"); // disable to prevent double clicks
        $("#saveStateFeedback").html("Starting database backup...");
        // Send command over WebSocket
        generateAndSendCommandMessage("savestate", []);

        // Set a timeout to handle case if DB is not downloaded
        saveStateTimeout = setTimeout(() => {
            $("#saveStateFeedback").html("Backup timed out. Please try again.");
            $btn.removeAttr("disabled"); // re-enable button
        }, 5000);

        e.stopPropagation();
    });
});