/*!
 * PD3 Router JavaScript Library
 * Depends on jQuery and jQuery UI
 * http://jquery.com/
 *
 * Copyright 2010, PD3 Tecnologia
 */

/* Global objects */
var ethernet = new RegExp("^eth*");
var loopback = new RegExp("^lo*");
var m3g = new RegExp("^m3[Gg]*");
var intf_type = {ethernet:1, loopback:2, m3g:3};

/* Left menu sliding function */
$(function () {
	$(".lvl1").click( function() {
		$(this).children().children().slideToggle();
	});
});

/* Reboot confirmation */
function reboot_me() {
    var rtn = confirm ("Reboot now?");

    return rtn;
}

function verifyInterfaceType(name) {
	
	if (ethernet.test(name))
		return intf_type.ethernet;
	else if (loopback.test(name))
		return intf_type.loopback;
	else if (m3g.test(name))
		return intf_type.m3g;
	
	return 0;
}

/**
 * isTextFieldEmpty		Check if a text field is empty
 * 
 * @param obj
 * @return true if empty, false otherwise
 */
function isTextFieldEmpty(obj) {
	var x = obj.val();
	
	if (x.length == 0)
		return true;
	else
		return false;
}

function isText(obj) {
	var x = /\w*/;
	
	return (x.test(obj.val()));	
}

function isNaturalValue(obj) {
	var x = /\d*/;
	
	return (x.test(obj.val()));
}

/* IP address validation */
function verifyIPAddress(obj) {
	var x = obj.val();
	var splitted = x.split(".");
	var valid = true;
	
	if (splitted.length != 4)
		valid = false;
	
	for ( var i = 0; i < splitted.length; i++) {
		if ((splitted[i] < 0)  || (splitted[i] > 255))
			valid = false;
	}	
		
	if (!valid) {
		obj.addClass("ui-state-error");
	} else {
		obj.removeClass("ui-state-error");
	}
	
	return valid;
}
