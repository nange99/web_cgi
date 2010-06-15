
$(function () {
	$(".lvl1").click( function() {
		$(this).children().slideToggle();
	});
});

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
