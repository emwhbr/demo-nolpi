
$(call define-srcs, nolpi, NolPi, \
	main.cpp \
	NolPi.cpp \
	NolPiGui.cpp \
)

# Notes:
# -ILittlevGL
#  Needed for lv_drivers to compile
#
# -Wno-narrowing
# Disable check of narrowing conversions when using LittlevGL from C++11
#
$(call apply-cppflags, nolpi, \
	-ILittlevGL \
	-Wno-narrowing \
)
