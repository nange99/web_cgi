/*!
 * PD3 Router JavaScript Library
 * Depends on jQuery and jQuery UI
 * http://jquery.com/
 *
 * Copyright 2010, PD3 Tecnologia
 */

/**
 * jQuery handlers at document ready
 */
$(function () {
	$(".lvl1").click( function() {
		$(this).children().children().slideToggle();
	});
	
	/* Set maximum length for text inputs */
	$(':text').attr('maxlength','50');

	/* Don't accept spaces in text boxes */
	$(':text').keypress(function (event) {
		if (event.which == 32) {
			event.preventDefault();
		}
	});
	
	/* Use tabs if any */
	if ($("#pageTabs").length)
		$("#pageTabs").tabs();
	
	/* UI Buttons */
	if ($(":button").length)
		$(':button').button();
	
});

/**
 * reboot_me	Ask for reboot confirmation
 * 
 * @return true if yes, false otherwise
 */
function reboot_me() {
    var rtn = confirm ("Reboot now?");

    return rtn;
}

/**
 * verifyInterfaceType		Get interface type from name
 * 
 * @param name
 * @return interface type
 */
function verifyInterfaceType(name) {
	var ethernet = new RegExp("^eth*");
	var loopback = new RegExp("^lo*");
	var m3g = new RegExp("^m3[Gg]*");
	var intf_type = {ethernet:1, loopback:2, m3g:3};
	
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
		return  true;
	else
		return false;
}

/**
 * checkForWhiteSpaces		Check if jQuery object value begins with a white space
 * 
 * @param obj
 * @return false if white space is first character
 */
function checkForWhiteSpaces(obj) {
	var x = obj.val();
	var y = new RegExp("^ ");
	
	if (y.test(x)) {
		obj.addClass("ui-state-error");
		return false;
	}
	return true;
}


/**
 * set_ui_state_error_onObj		Add or remove ui-state-error for object     
 * 
 * @param obj
 * @param flag
 * @return true if ok
 */
function set_ui_state_error_onObj(obj,flag){
	
	if (flag)
		obj.addClass("ui-state-error");
	else 
		obj.removeClass("ui-state-error");
	
	return true;
}


/**
 * 
 * inRange		Check if jQuery object has value between min and max
 * 
 * @param obj
 * @param min
 * @param max
 * @return true if in range, false otherwise
 */
function inRange(obj, min, max) {
	var ok = true;
	var value = obj.val();
	
	if (isNaN(value)) /* Test if entry is a valid number */
		ok = false;
	
	if (parseInt(value) < min)
		ok = false;
	
	if (parseInt(value) > max)
		ok = false;
	
	if (ok == false) {
		obj.addClass("ui-state-error");
	} else {
		obj.removeClass("ui-state-error");
	}
	
	return ok;
}

/**
 * verifyDate	Validate jQuery object with date value
 * 
 * Time is valid when in format dd/mm/yyyy
 * 
 * @param obj
 * @return true if valid, false otherwise
 */
function verifyDate(obj) {
	var x = obj.val();
	var splitted = x.split("/");
	var valid = true;
	
	if (splitted.length != 3)
		valid = false;
	
	var day = splitted[0];
	var mon = splitted[1];
	var year = splitted[2];
	
	if ((day <= 0) || (day > 31))
		valid = false;
	
	if ((mon <= 0) || (mon > 12))
		valid = false;
	
	if ((year < 1970) || (year > 2037))
		valid = false;
	
	if ((mon == 4) || (mon == 6) || (mon == 9) || (mon == 11)) {
		if (day > 30)
			valid = false;
	}
	
	if (mon == 2) {
		if ((year % 4) == 0) {
			if (day > 29)
				valid = false;
		} else {
			if (day > 28)
				valid = false;
		}
	}
			
	if (!valid) {
		obj.addClass("ui-state-error");
	} else {
		obj.removeClass("ui-state-error");
	}
	
	return valid;
}

/**
 * verifyTime	Validate jQuery object with time value
 * 
 * @param obj
 * @return true if valid, false otherwise
 */
function verifyTime(obj) {
	var x = obj.val();
	var splitted = x.split(":");
	var valid = true;
	
	if (splitted.length != 3)
		valid = false;
	
	if (splitted[0] < 0 || splitted[0] > 23)
		valid = false;
	
	if (splitted[1] < 0 || splitted[1] > 59)
		valid = false;
	
	if (splitted[2] < 0 || splitted[2] > 59)
		valid = false;
		
	if (!valid) {
		obj.addClass("ui-state-error");
	} else {
		obj.removeClass("ui-state-error");
	}
	
	return valid;
}

/**
 * verifyIPAddress_and_Mask		Check if jQuery object has a valid IP address and mask values
 * 
 * @param obj
 * @return true if valid, false otherwise
 */
function verify_IPAddress_Mask_AnyAddr(obj) {
	var x = obj.val();
	var valid = true;

	if (x != "*"){
			
		var splitteddot = x.split(".");
		var splittedslash = x.split("/");
				
		if (splittedslash.length == 2){
			var splittedip = splittedslash[0].split(".");
			var splittedmask = splittedslash[1].split(".");
			
			for ( var i = 0; i < splittedip.length; i++) {
				if ((splittedip[i] < 0)  || (splittedip[i] > 255))
					valid = false;
			}
			
			for ( var i = 0; i < splittedmask.length; i++) {
				if ((splittedmask[i] < 0)  || (splittedmask[i] > 255))
					valid = false;
			}
		}
		else { 
			if (splitteddot.length != 4)
				valid = false;
			
			for ( var i = 0; i < splitteddot.length; i++) {
				if ((splitteddot[i] < 0)  || (splitteddot[i] > 255))
					valid = false;
			}	
		
		}
	}
	
	if (!valid) {
		obj.addClass("ui-state-error");
	} else {
		obj.removeClass("ui-state-error");
	}
	
	return valid;
}


/**
 * verifyIPAddress		Check if jQuery object has a valid IP address value
 * 
 * @param obj
 * @return true if valid, false otherwise
 */
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

/**
 * stringMatch	Check if two jQuery objects have the same string value 
 * 
 * When not the same, set the ui-state-error class
 * to both objects. 
 * 
 * @param str1
 * @param str2
 * @return true if match, false otherwise
 */
function stringMatch(str1, str2) {
	var x = str1.val();
	var y = str2.val();
	
	if (x == y) {
		str1.removeClass("ui-state-error");
		str2.removeClass("ui-state-error");
		return true;
	} else {
		str1.addClass("ui-state-error");
		str2.addClass("ui-state-error");
		return false;
	}
}

/**
 * authServerVerify		Validate authentication server information
 * 
 * @param ip
 * @param key
 * @param timeout
 * @return true if ok, false if not ok
 */
function authServerVerify(ip, key, timeout) {
	var ok = true;
	var empty_ip = isTextFieldEmpty(ip);
	var empty_key = isTextFieldEmpty(key);
	var empty_timeout = isTextFieldEmpty(timeout);
	
	if (empty_ip == false)
		ok = verifyIPAddress(ip) && ok;
	
	if (empty_key == false)
		ok = checkForWhiteSpaces(key) && ok;
	
	if (empty_timeout == false)
		ok = inRange(timeout, 1, 1000) && ok;
	
	/* If ip field is blank and others are field, show error */
	if (empty_ip) {
		if (!empty_key || !empty_timeout) {
			ip.addClass("ui-state-error");
			ok = false;
		}
	/* else if empty key, no timeout is accepted */
	} else if (empty_key) {
		if (!empty_timeout) {
			key.addClass("ui-state-error");
			ok = false;
		}
	}

	return ok;
}
