
$(call define-srcs, app4, App4, \
	main.cpp\
)

# Notes:
# -Wno-narrowing
# Disable check of narrowing conversions when using LittlevGL from C++11
#
$(call apply-cppflags, app4, \
  -Wno-narrowing \
)
