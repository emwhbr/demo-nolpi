#ifndef _NOLPI_H_
#define _NOLPI_H_

#include <thread>

#include "NolPi/NolPiGui.h"

class NolPi
{
public:
   NolPi();
   ~NolPi();

private:
   NolPiGui *m_Gui{nullptr};

   bool         m_TemperatureEnable{true};
   std::thread *m_TemperatureThread{nullptr};

private:
   void TemperatureHandler();
};

#endif // _NOLPI_H
