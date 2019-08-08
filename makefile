# ----------------------------
# Set NAME to the program name
# Set ICON to the png icon file name
# Set DESCRIPTION to display within a compatible shell
# Set COMPRESSED to "YES" to create a compressed program
# ----------------------------

NAME        ?= SNAP
COMPRESSED  ?= NO
ICON        ?= iconc.png
DESCRIPTION ?= "Snap! CE"

# ----------------------------

include $(CEDEV)/include/.makefile
