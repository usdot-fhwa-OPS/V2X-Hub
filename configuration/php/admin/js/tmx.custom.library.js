"use strict";
var deviceName = "V2I";
var customLibraryJsVersion = "0.3.0";

var displayEnabled = true;
var displayDisabled = false;
var displayExternal = false;
var msgShowKeepAliveMsg = false;


// Event listener from common.library 
eventElement.addEventListener("newMessage", newMessageHandler, false);

function newMessageHandler(evt) {

}

function updatePluginFilter()
{
	$("[id=\"enableButton\"]").attr("data-state", displayEnabled);
	$("[id=\"disableButton\"]").attr("data-state", displayDisabled);
	$("[id=\"externalButton\"]").attr("data-state", displayExternal);
	/*$("[id=\"sslButton\"]").attr("data-state", useSSL);*/

	if (displayEnabled) {
	    $(".plugin > [id^=\"pluginHeader\"][class*=\"pluginHeaderEnabled\"]").parent().css("display", "");
	} else {
	    $(".plugin > [id^=\"pluginHeader\"][class*=\"pluginHeaderEnabled\"]").parent().css("display", "none");
	}

	if (displayDisabled) {
	    $(".plugin > [id^=\"pluginHeader\"][class*=\"pluginHeaderDisabled\"]").parent().css("display", "");
	} else {
	    $(".plugin > [id^=\"pluginHeader\"][class*=\"pluginHeaderDisabled\"]").parent().css("display", "none");
	}

	if (displayExternal) {
	    $(".plugin > [id^=\"pluginHeader\"][class*=\"pluginHeaderExternal\"]").parent().css("display", "");
	} else {
	    $(".plugin > [id^=\"pluginHeader\"][class*=\"pluginHeaderExternal\"]").parent().css("display", "none");
	}

	//if(useSSL) $("[id=\"sslButton\"]").html("SSL");
	//else $("[id=\"sslButton\"]").html("No SSL");
}

function resetPluginFilterButtons() {
    displayEnabled = true;
    displayDisabled = false;
    displayExternal = false;
    $("[id=\"enableButton\"]").attr("data-state", displayEnabled);
    $("[id=\"disableButton\"]").attr("data-state", displayDisabled);
    $("[id=\"externalButton\"]").attr("data-state", displayExternal);
}

$(document).ready(function() {
	$("[id=\"enableButton\"]").on("mousedown", function () {
		displayEnabled = !displayEnabled;
		updatePluginFilter();
		$("[id=\"enableButton\"]").attr("data-state", displayEnabled);
	});

	$("[id=\"disableButton\"]").on("mousedown", function () {
		displayDisabled = !displayDisabled;
		updatePluginFilter();
		$("[id=\"disableButton\"]").attr("data-state", displayDisabled);
	});

	$("[id=\"externalButton\"]").on("mousedown", function () {
		displayExternal = !displayExternal;
		updatePluginFilter();
		$("[id=\"externalButton\"]").attr("data-state", displayExternal);
	});

    $("[id=\"showKeepAliveButton\"]").on("mousedown", function () {
        msgShowKeepAliveMsg = !msgShowKeepAliveMsg;
        $("[id=\"showKeepAliveButton\"]").attr("data-state", msgShowKeepAliveMsg);
        $("[id=messagesTable]").DataTable().draw();
    });
    
    $("[id=\"showKeepAliveButton\"]").attr("data-state", msgShowKeepAliveMsg);

	$("[id=\"clearLogButton\"]").on("mousedown", function () {
		sendClearLog();
	});

	/*$("[id=\"sslButton\"]").on("mousedown", function () {
		useSSL = !useSSL;
		$("[id=\"sslButton\"]").attr("data-state", useSSL);
		if(useSSL) $("[id=\"sslButton\"]").html("SSL");
		else $("[id=\"sslButton\"]").html("No SSL");
		ssl_changed();
		changeIPAddress();
	});*/
	setInterval(function() {
	    var timeDisplay = document.getElementById("timeDisplay");
    	if (timeDisplay != null && timeDisplay != undefined) {
    		var n = new Date().getTime();
    		var d = new Date(n - timeOffsetMs);
			timeDisplay.innerHTML = d.toUTCString();
    	}
	}, 1000);

	updatePluginFilter();
});