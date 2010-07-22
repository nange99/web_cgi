bin_PROGRAMS = \
	web_digistar

web_digistar_SOURCES = \
	main.c \
	interface.c \
	reboot.c \
	route.c \
	firmware.c \
	dhcp.c \
	firewall.c \
	nat.c \
	ipsec.c \
	logging.c \
	cpu.c \
	ntp.c

htmlfilesdir = $(datadir)
cssdir = $(datadir)/css
javascriptdir = $(datadir)/js
imagesdir = $(datadir)/images
ui_imagesdir = $(datadir)/css/images
appdir = $(datadir)/app
templatesdir = $(bindir)/templates

htmlfiles_DATA = htdocs/index.html \
		htdocs/index.wn

css_DATA = htdocs/css/digistar_styles.css \
		htdocs/css/jquery-ui-1.8.2.custom.css

javascript_DATA = htdocs/js/*

images_DATA = htdocs/images/*
ui_images_DATA = htdocs/css/images/*

app_DATA = htdocs/app/*

templates_DATA = htdocs/templates/*

