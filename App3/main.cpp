#include <iostream>

#include "Support/Platform/platform.h"
#include "Support/Xbase/xbase.h"
 
int main(int argc, char *argv[])
{
   (void)argc;
   (void)argv;
   
   std::cout << "Platform version = " << platform_get_version() << "\n";
   std::cout << "Xbase name       = " << xbase_get_name() << "\n";
   return 0;
}
