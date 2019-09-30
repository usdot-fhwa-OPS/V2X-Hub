"use strict"; 

var userLibraryJsVersion = "0.0.1";
var currUser = null;

function updateUserObject(user, access) 
{
    var accessString = "Read Only";
    switch (access) {
        case "3": accessString = "System Administrator";
            break;
        case "2": accessString = "Application Administrator";
            break
    }
    var tableRow = "";
    if (currUser == user)
    {
        tableRow = "<tr><td data-type='userName'>" + user + "</td><td data-type='userAccess'>" + accessString + "</td><td data-type='userOperations'><button id='userResetPassword_" + user + "' class='userBtn' onmousedown='resetPassword(this, \"" + user + "\")'>Reset Password</button></td></tr>";
    }
    else
    {
        tableRow = "<tr><td data-type='userName'>" + user + "</td><td data-type='userAccess'>" + accessString + "</td><td data-type='userOperations'><button id='userResetPassword_" + user + "' class='userBtn' onmousedown='resetPassword(this, \"" + user + "\")'>Reset Password</button><button id='userEdit_" + user + "' class='userBtn' onmousedown='changeUserAccess(this, \"" + user + "\", \"" + access + "\")'>Change Access</button><button id='deleteUser_" + user + "' class='userBtn' onmousedown='deleteUser(this, \"" + user + "\")'>Delete User</button></td></tr>";
    }
    var table = $("[id=\"usersTable\"]");
    $("[id=\"usersTable\"]").append(tableRow);
}

function updateUsers(users)
{
    $("#usersTable tr").remove(); 
    users.forEach( function (arrayItem)
    {
        updateUserObject(arrayItem.username, arrayItem.accesslevel);
    });
}

// User Management
var managementUser = "";
var deleteTarget = "";

function createUser()
{
    console.log("Create User");
    $("#err_new_user").html("");
    $("#user_add_dialog").dialog("open");
    resizeDialogWindow($(".ui-dialog[aria-describedby=\"user_add_dialog\"]"));
    $("#new_username").val("");
    $("#new_password1").val("")
    $("#new_password2").val("")
}
function sendCreateUser()
{
    var validData = true;
    var newUserPassword = "";
    var newUsername = $("#new_username").val();
    // Do username check
    if ($("#new_password1").val() === $("#new_password2").val())
    {
        newUserPassword = $("#new_password1").val()
    }
    else 
    {
        validData = false;
        // Create error message
    }
    var newUserAccess = $("#new_user_Access").val();
//    console.log("New User:" + newUsername + " Password:" + newUserPassword + " Access:" + newUserAccess);
    if (validData)
    {
        generateAndSendCommandMessage("useradd", [{ name: "username", value: newUsername }, { name: "password", value: newUserPassword}, { name: "accesslevel", value: newUserAccess}]);
        $("#user_add_dialog").dialog("close");
    }
    else
    {
        
        $("#err_new_user").html("Passwords do not match");
        // show error message
    }
    
}

function deleteUser(buttonObject, user)
{
    managementUser = user;
    console.log("Delete User:" + user);
    $("#user_delete_dialog").dialog("open");
    resizeDialogWindow($(".ui-dialog[aria-describedby=\"user_delete_dialog\"]"));
}
function sendDeleteUser()
{
    generateAndSendCommandMessage("userdelete", [{ name: "username", value: managementUser }]);
    $("#user_delete_dialog").dialog("close");
}

function changeUserAccess(buttonObject, user, currentAccess)
{
    managementUser = user;
    $('#change_user_Access').val(currentAccess);
    $("#change_user_Access").selectmenu("refresh");
    $("#user_change_access_dialog").dialog("open");
    resizeDialogWindow($(".ui-dialog[aria-describedby=\"user_change_access_dialog\"]"));
}

function sendChangeUserAccess()
{
    generateAndSendCommandMessage("userupdate", [{ name: "username", value: managementUser }, { name: "accesslevel", value: $("#change_user_Access").val() }]);
    $("#user_change_access_dialog").dialog("close");
}
function resetPassword(buttonObject, user)
{
    $("#password1").val("");
    $("#password2").val("");
    $("#err_change_password").html("");
    managementUser = user;
    $("#confirm_reset").dialog("open");
    resizeDialogWindow($(".ui-dialog[aria-describedby=\"confirm_reset\"]"));
}

function sendResetPassword()
{
    if ($("#password1").val() === $("#password2").val())
    {
        $( "#confirm_reset" ).dialog("close");
        generateAndSendCommandMessage("userupdate", [{ name: "username", value: managementUser }, { name: "password", value: $("#password1").val() }]);
    }
    else 
    {
        // Notify User passwords do not match
        console.log("Passwords do not match");
        $("#err_change_password").html("Passwords do not match");
    }

}
// --- end User Management --------------------

function sendUserRequest()
{
    generateAndSendCommandMessage("userlist", []);
}

function SendLoginCredentials() {
    if ($("#uname").val() == "") {
        $("#uname").css("background-color", "red");
        $("#loginFeedback").html("Username required");
        return;
    }
    else  $("#uname").css("background-color", "");
    
    if ($("#upwd").val() == "") {
        $("#upwd").css("background-color", "red");
        $("#loginFeedback").html("Password required");
        return;
    }
    else $("#upwd").css("background-color", "");

    var uname = $("#uname").val().replace(/\\(.)/mg, "").replace(/&/g, "&#038;").replace(/</g, "&#060;").replace(/>/g, "&#062;").replace(/"/g, "&#034;").replace(/'/g, "&#039;").replace(/\//g, "&#047;").replace(/\\/g, "&#092;").trim();
    var d = new Date().getTime() - timeOffsetMs;
    var msg = '{"header": {"type": "Command","subtype": "Execute","encoding": "jsonstring","timestamp": "' + d + '","flags": "0"},' +
              '"payload": {"command": "login", "id": "' + cmdCntr + '", "args": {"user": "' + uname + '","password": "' + $("#upwd").val() + '"}}}';
    currUser = uname;
    $("#uname").val("");
    $("#upwd").val("");
    $("#uname").css("background-color", "");
    $("#upwd").css("background-color", "");
    $("#loginFeedback").html("");
    cmdCntr++;

    sendCommand(msg);
}

function SendLogoutCommand() {
    generateAndSendCommandMessage("logout", []);
}

$(document).ready(function () {
    // Initialize User Password Change
    $( "#confirm_reset" ).dialog({
        title: "Change User Password",
        autoOpen: false,
        modal: true,
        width: 800,
        draggable: true,
        resizable: true,
        buttons: {
            "Change": function () {
                sendResetPassword();
                sendUserRequest();
            },
            "Cancel": function () {
                $("#confirm_reset").dialog("close");
            }
        }                
    });

    $( "#user_change_access_dialog" ).dialog({
        title: "Change User Access",
        autoOpen: false,
        modal: true,
        width: 600,
        draggable: true,
        resizable: true,
        buttons: {
            "Change": function () {
                sendChangeUserAccess();
                sendUserRequest();
            },
            "Cancel": function () {
                $("#user_change_access_dialog").dialog("close");
            }
        }                
    });

    $( "#user_delete_dialog" ).dialog({
        title: "Delete User",
        autoOpen: false,
        modal: true,
        width: 600,
        draggable: true,
        resizable: true,
        buttons: {
            "Delete": function () {
                sendDeleteUser();
                sendUserRequest();
            },
            "Cancel": function () {
                $("#user_delete_dialog").dialog("close");
            }
        }                
    });
    $( "#delete_plugin_dialog" ).dialog({
        title: "Remove Plugin?",
        autoOpen: false,
        modal: true,
        width: 600,
        draggable: true,
        resizable: true,
        buttons: {
            "Remove": function () {
                sendDeletePluginCommand();
            },
            "Cancel": function () {
                $("#delete_plugin_dialog").dialog("close");
            }
        }                
    });

    $( "#user_add_dialog" ).dialog({
        title: "Add New User",
        autoOpen: false,
        modal: true,
        width: 600,
        draggable: true,
        resizable: true,
        buttons: {
            "Add": function () {
                sendCreateUser();
                sendUserRequest();
            },
            "Cancel": function () {
                $("#user_add_dialog").dialog("close");
            }
        }                
    });
    $( "#new_user_Access" ).selectmenu();
    $( "#change_user_Access" ).selectmenu();
});