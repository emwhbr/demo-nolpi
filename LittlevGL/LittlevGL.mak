
# lvgl
include LittlevGL/lvgl/src/lv_core/lv_core.mak
include LittlevGL/lvgl/src/lv_hal/lv_hal.mak
include LittlevGL/lvgl/src/lv_objx/lv_objx.mak
include LittlevGL/lvgl/src/lv_font/lv_font.mak
include LittlevGL/lvgl/src/lv_misc/lv_misc.mak
include LittlevGL/lvgl/src/lv_themes/lv_themes.mak
include LittlevGL/lvgl/src/lv_draw/lv_draw.mak

# lv_drivers
include LittlevGL/lv_drivers/display/display.mak
include LittlevGL/lv_drivers/indev/indev.mak

# custom widgets
include LittlevGL/custom/custom.mak

$(call concat-objs, littlevgl, \
	littlevgl-lvgl-lvcore \
	littlevgl-lvgl-lvhal \
	littlevgl-lvgl-lvobjx \
	littlevgl-lvgl-lvfont \
	littlevgl-lvgl-lvmisc \
	littlevgl-lvgl-lvthemes \
	littlevgl-lvgl-lvdraw \
	littlevgl-lv_drivers-display \
	littlevgl-lv_drivers-indev \
	littlevgl-custom-widgets \
)

# Notes:
# -ILittlevGL
#  Needed for lv_drivers to compile
#
# -Wundef
#  We don't want any undefined macros in LittlevGL
#
$(call apply-cppflags, littlevgl, \
	-ILittlevGL \
	-Wundef \
)
