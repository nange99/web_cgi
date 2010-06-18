/*!
 * PD3 Router JavaScript Library
 * Depends on jQuery and jQuery UI
 * http://jquery.com/
 *
 * Copyright 2010, PD3 Tecnologia
 */

/* Left menu sliding function */
$(function () {
	$(".lvl1").click( function() {
		$(this).children().slideToggle();
	});
});

/* Reboot confirmation */
function reboot_me() {
    var rtn = confirm ("Reboot now?");

    return rtn;
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

/* Route deletion */
function deleteRoute(hash) {
	
	if (confirm("Delete this route?"))
		$.ajax({url: "/app/config_static_routes?del=" + hash, success: function(data) { $("#table_wrapper").html(data)}});
}

