#include <iostream>

#include "NolPi/NolPi.h"
 
int main(int argc, char *argv[])
{
	(void)argc;
	(void)argv;

  	std::cout << "Hello, NolPi! - C++\n";

   NolPi app;
  
   std::this_thread::sleep_for(std::chrono::seconds(500));

  	return 0;
}
