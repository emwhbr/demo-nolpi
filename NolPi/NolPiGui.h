#ifndef _NOLPI_GUI_H_
#define _NOLPI_GUI_H_

#include <thread>
#include <mutex>
#include <condition_variable>

#include "LittlevGL/lvgl/lvgl.h"

class NolPiGui
{
public:
   static constexpr int MAX_TEMP = 35;

   NolPiGui();
   ~NolPiGui();

   void DummyApi();

   void StartEntry();            // Public to be callable from static callback
   void StartMain(bool preload); // Public to be callable from static callback
   void StartChart();            // Public to be callable from static callback
   void StartImage();            // Public to be callable from static callback

   void SetCurrentTemp(int value);
   void SetWarningTemp(int value); // Public to be callable from static callback

   bool SimulateTempActive();
   void SetSimulateTempActive(bool value); // Public to be callable from static callback

   lv_obj_t* GetMainPrevBut();  // Public to be callable from static callback
   lv_obj_t* GetMainNextBut();  // Public to be callable from static callback
   lv_obj_t* GetChartPrevBut(); // Public to be callable from static callback
   lv_obj_t* GetChartNextBut(); // Public to be callable from static callback
   lv_obj_t* GetImagePrevBut(); // Public to be callable from static callback

   lv_obj_t* GetImageCustomImage();  // Public to be callable from static animator

private:
   // Periodic threads for LittlevGL
   bool         m_RunThreads{true};
   std::thread *m_TickHandlerThread{nullptr};
   std::thread *m_TaskHandlerThread{nullptr};

   // LittlevGL display buffer
   lv_color_t    *m_pDrawBuffer{nullptr};
   lv_disp_buf_t  m_DisplayBuffer;

   // LittlevGL screens
   lv_obj_t *m_pScreenTop{nullptr};
   lv_obj_t *m_pScreenEntry{nullptr};
   lv_obj_t *m_pScreenMain{nullptr};
   lv_obj_t *m_pScreenChart{nullptr};
   lv_obj_t *m_pScreenImage{nullptr};

   // Graphical objects in entry screen
   lv_obj_t *m_pStartButton{nullptr};
   lv_obj_t *m_pSimulateCheckbox{nullptr};
   bool      m_Simulate{false};

   // The preloader
   std::mutex               m_PreloadMutex;
   std::condition_variable  m_PreloadCondVar;
   bool                     m_PreloadStart{false};
   bool                     m_PreloadEnable{true};
   std::thread             *m_PreloadThread{nullptr};

   // The navigation buttons in main screen
   lv_obj_t *m_pMainPrevBut{nullptr};
   lv_obj_t *m_pMainNextBut{nullptr};

   // The temperature gauge in main screen
   lv_obj_t *m_pTempGauge{nullptr};

   // The temperature warning slider and LED in main screen
   lv_obj_t *m_pWarningSlider{nullptr};
   lv_obj_t *m_pWarningLED{nullptr};
   int       m_WarningTemp{0};

   // The navigation buttons in chart screen
   lv_obj_t *m_pChartPrevBut{nullptr};
   lv_obj_t *m_pChartNextBut{nullptr};

   // The temperature chart in chart screen
   lv_obj_t          *m_pTempChart{nullptr};
   lv_chart_series_t *m_pCurrentSerie{nullptr};
   lv_chart_series_t *m_pWarningSerie{nullptr};

   // The navigation buttons in image screen
   lv_obj_t *m_pImagePrevBut{nullptr};

   // The custom image in image screen
   lv_obj_t *m_pImageCustomImage{nullptr};

private:
   void InitGraphics();
   void ExitGraphics();

   void CreateGUI();
   void DestroyGUI();

   void SetSchedFifoPriority(int prio);
   void TickHandler();
   void TaskHandler();

   void StartPreload();
   void StopPreload();
   void DisablePreload();
   bool IsPreloadStarted();
   void PreloadHandler();

   void CreateEntryScreen();
   void CreateMainScreen();
   void CreateChartScreen();
   void CreateImageScreen();

   void OpenEntryScreen();
   void OpenMainScreen();
   void OpenChartScreen();
   void OpenImageScreen();

   void RedrawScreen(lv_obj_t *screen);

   void SetTemp(int idx, int value);

   // Event callbacks (must be static)
   static void EventCbEntryScreenButton(lv_obj_t *obj, lv_event_t event);
   static void EventCbEntryScreenCheckbox(lv_obj_t *obj, lv_event_t event);

   static void EventCbMainScreenButton(lv_obj_t *obj, lv_event_t event);
   static void EventCbMainScreenSlider(lv_obj_t *obj, lv_event_t event);

   static void EventCbChartScreenButton(lv_obj_t *obj, lv_event_t event);

   static void EventCbImageScreenButton(lv_obj_t *obj, lv_event_t event);

   // Custom animation function (must be static)
   static void CustomImageAnimator(void *obj, lv_anim_value_t value); 
};

#endif // _NOLPI_GUI_H_
