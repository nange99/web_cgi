$("document").ready ( function() {
		$("#toptext").html("<p>PD3 Tecnologia</p>");
});

$(function () {
	$(".lvl1").click( function() {
		$(this).children().slideToggle();
	});
});

