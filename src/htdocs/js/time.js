
$(function () {
	$(".lvl1").click( function() {
		$(this).children().slideToggle();
	});
});

function verifyIPAddress(buttonID) {
	var x = $(buttonID).val();
	var splitted = x.split(".");
	var invalid = 0;
	
	if (splitted.length != 4)
		invalid = 1;
	
	
	for ( var i = 0; i < splitted.length; i++) {
		if ((splitted[i] < 0)  || (splitted[i] > 255))
			invalid = 1;
	}	
		
	if (invalid)	
		alert("Invalid IP address : " + x);
	
	return ( invalid ? -1 : 0 );
}