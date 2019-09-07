#include <iostream>
#include <future>

#include "NolPi/NolPi.h"
#include "LittlevGL/lvgl/src/lv_version.h"
 
int main(int argc, char *argv[])
{
   (void)argc;
   (void)argv;

   std::cout << "NolPi demo, LittlevGL:"
             << LVGL_VERSION_MAJOR << "."
             << LVGL_VERSION_MINOR << "."
             << LVGL_VERSION_PATCH << std::endl;

   NolPi app;

   std::promise<void>().get_future().wait();

   return 0;
}
