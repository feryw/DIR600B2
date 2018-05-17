#
# flex
#

config ELBOX_FLEX
	bool "flex"
	default n
	help
		A fast lexical analyzer generator.

choice
	prompt "Selection Version"
	depends ELBOX_FLEX
	default ELBOX_PROGS_GPL_FLEX_2_5_33
	help
		Select the flex version for the platform

config ELBOX_PROGS_GPL_FLEX_2_5_33
	bool "flex 2.5.33"
	help
		programs which recognized lexical patterns in text. It reads the given input files for a description of a scanner to generate.

endchoice

config ELBOX_FLEX_DONT_INSTALL
	bool

