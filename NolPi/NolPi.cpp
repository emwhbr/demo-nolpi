#include "NolPi/NolPi.h"

////////////////////////////////////////////////////////////////////////////
//               Public member functions
/////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////

NolPi::NolPi()
{
   // Create user interface
   m_Gui = new NolPiGui();

   // Create the temperature generating thread
   m_TemperatureThread = new std::thread([this]{this->TemperatureHandler();});
}

////////////////////////////////////////////////////////////////

NolPi::~NolPi()
{
   if (m_TemperatureThread)
   {
      m_TemperatureEnable = false;
      m_TemperatureThread->join();
      delete m_TemperatureThread;
   }

   delete m_Gui;
}

////////////////////////////////////////////////////////////////////////////
//               Private member functions
/////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////

void NolPi::TemperatureHandler()
{
   int temp = 0;
   int incr = 1;

   // Executed as thread, until disabled
   // Ramp temperature cyclic in interval 0 -> 35 -> 0
   while (m_TemperatureEnable)
   {
      if (m_Gui->SimulateTempActive())
      {
         m_Gui->SetCurrentTemp(temp);
         temp += incr;
         if (temp >= NolPiGui::MAX_TEMP)
         {
            incr = -1;
         }
         if (temp <= 0)
         {
            incr = 1;
         }
      }
      else
      {
         temp = 0;
         incr = 1;
      }
      std::this_thread::sleep_for(std::chrono::seconds(1));
   }
}
