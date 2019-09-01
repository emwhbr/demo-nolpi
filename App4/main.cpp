#include <iostream>

#include "LittlevGL/lvgl/lvgl.h"
 
int main(int argc, char *argv[])
{
	(void)argc;
	(void)argv;

  	std::cout << "Hello, LittlevGL! - C++\n";

   // Initialize LittlevGL
   lv_init();

   lv_obj_t *m_pScreenTop = lv_scr_act();
   if (m_pScreenTop == NULL)
   {
      return 1;
   }

  	return 0;
}
