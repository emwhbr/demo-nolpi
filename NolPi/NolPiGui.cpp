#include <chrono>
#include <functional>

#include "NolPi/NolPiGui.h"
#include "NolPi/PcEnv.h"

#if defined PCENV
#include "LittlevGL/lv_drivers/display/monitor.h"
#include "LittlevGL/lv_drivers/indev/mouse.h"
#else
#include "LittlevGL/lv_drivers/display/fbdev.h"
#endif

////////////////////////////////////////////////////////////////////////////
//               Global definitions
////////////////////////////////////////////////////////////////////////////

// LittlevGL task period times
static constexpr unsigned int TICK_PERIOD_MS = 5;
static constexpr unsigned int TASK_PERIOD_MS = 10;

static constexpr int PRELOAD_TIME_S = 1;

// Temperature definitions
static constexpr int DEG_PER_INTERVAL = 5;

static constexpr int IDX_CURRENT_TEMP = 0;
static constexpr int IDX_WARNING_TEMP = 1;

static constexpr int DEFAULT_CURRENT_TEMP = 20;
static constexpr int DEFAULT_WARNING_TEMP = 25;

static constexpr int NR_TEMP_POINTS = 20;

// Custom widgets for LittlevGL
LV_IMG_DECLARE(tritech_logo);

static constexpr lv_coord_t ANIM_X_MIN = 10;
static constexpr lv_coord_t ANIM_X_MAX = 290;

static constexpr lv_coord_t ANIM_Y_START = 138;

////////////////////////////////////////////////////////////////////////////
//               Public member functions
////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////

NolPiGui::NolPiGui()
{
   // Create user interface
   InitGraphics();
   CreateGUI();
}

////////////////////////////////////////////////////////////////

NolPiGui::~NolPiGui()
{
   DestroyGUI();
   ExitGraphics();
}

////////////////////////////////////////////////////////////////

void NolPiGui::DummyApi()
{
   printf("%s\n", __func__);
}

////////////////////////////////////////////////////////////////

void NolPiGui::StartEntry()
{
   lv_cb_set_checked(m_pSimulateCheckbox, false);
   m_Simulate = false;

   SetCurrentTemp(DEFAULT_CURRENT_TEMP);
   SetWarningTemp(DEFAULT_WARNING_TEMP);

   OpenEntryScreen();
}

////////////////////////////////////////////////////////////////

void NolPiGui::StartMain(bool preload)
{
   if (preload)
   {
      StartPreload();
   }
   else
   {
      OpenMainScreen();
   }
}

////////////////////////////////////////////////////////////////

void NolPiGui::StartChart()
{
   OpenChartScreen();
}

////////////////////////////////////////////////////////////////

void NolPiGui::StartImage()
{
   OpenImageScreen();
}

///////////////////////////////////////////////////////////////

void NolPiGui::SetCurrentTemp(int value)
{
   SetTemp(IDX_CURRENT_TEMP, value);
}

///////////////////////////////////////////////////////////////

void NolPiGui::SetWarningTemp(int value)
{
   SetTemp(IDX_WARNING_TEMP, value);
   lv_slider_set_value(m_pWarningSlider, m_WarningTemp, LV_ANIM_OFF);
}

///////////////////////////////////////////////////////////////

bool NolPiGui::SimulateTempActive()
{
   return m_Simulate;
}

///////////////////////////////////////////////////////////////

void NolPiGui::SetSimulateTempActive(bool value)
{
   m_Simulate = value;
}

///////////////////////////////////////////////////////////////

lv_obj_t* NolPiGui::GetMainPrevBut()
{
   return m_pMainPrevBut;
}

///////////////////////////////////////////////////////////////

lv_obj_t* NolPiGui::GetMainNextBut()
{
   return m_pMainNextBut;
}

///////////////////////////////////////////////////////////////

lv_obj_t* NolPiGui::GetChartPrevBut()
{
   return m_pChartPrevBut;
}

///////////////////////////////////////////////////////////////

lv_obj_t* NolPiGui::GetChartNextBut()
{
   return m_pChartNextBut;
}

///////////////////////////////////////////////////////////////

lv_obj_t* NolPiGui::GetImagePrevBut()
{
   return m_pImagePrevBut;
}

///////////////////////////////////////////////////////////////

lv_obj_t* NolPiGui::GetImageCustomImage()
{
   return m_pImageCustomImage;
}

////////////////////////////////////////////////////////////////////////////
//               Private member functions
////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////

void NolPiGui::InitGraphics()
{
   // Initialize LittlevGL
   lv_init();

#if defined PCENV
   // Use the 'monitor' driver.
   // Creates a SDL window on PC's monitor to simulate a display.
   monitor_init();
#else
   // Use the NolPi 'framebuffer' driver.
   fbdev_init();
#endif

   // Initialize the display buffer
   m_pDrawBuffer = new lv_color_t[LV_HOR_RES_MAX * LV_VER_RES_MAX];
   lv_disp_buf_init(&m_DisplayBuffer,
                    m_pDrawBuffer,
                    NULL,
                    LV_HOR_RES_MAX * LV_VER_RES_MAX);

   // Register the display driver to LittlecGL
   lv_disp_drv_t dispDrv;
   lv_disp_drv_init(&dispDrv);
   dispDrv.hor_res  = LV_HOR_RES_MAX;
   dispDrv.ver_res  = LV_VER_RES_MAX;
   dispDrv.buffer   = &m_DisplayBuffer;
#if defined PCENV
   dispDrv.flush_cb = monitor_flush;
#else
   dispDrv.flush_cb = fbdev_flush;
#endif
   lv_disp_t *monitorDisp = lv_disp_drv_register(&dispDrv);
   if (monitorDisp == NULL)
   {
      printf("%s : can't register display driver\n", __func__);
   }

#if defined PCENV
   // Use the 'mouse' driver.
   // Reads the PC's mouse.
   mouse_init();
#else
   // Use NolPi stuff here
#endif

   // Register the input device driver to LittlevGL
   lv_indev_drv_t indevDrv;
   lv_indev_drv_init(&indevDrv);
   indevDrv.type    = LV_INDEV_TYPE_POINTER;
#if defined PCENV
   indevDrv.read_cb = mouse_read;
#else
   // Use NolPi stuff here
#endif
   lv_indev_t *mouseIndev = lv_indev_drv_register(&indevDrv);
   if (mouseIndev == NULL)
   {
      printf("%s : can't register input device driver\n", __func__);
   }

   // Create LittlevGL threads
   m_TickHandlerThread = new std::thread([this]{this->TickHandler();});
   m_TaskHandlerThread = new std::thread([this]{this->TaskHandler();});
}

///////////////////////////////////////////////////////////////

void NolPiGui::ExitGraphics()
{
   // Stop LittlevGL threads
   m_RunThreads = false;
   if (m_TickHandlerThread)
   {
      m_TickHandlerThread->join();
      delete m_TickHandlerThread;
   }
   if (m_TaskHandlerThread)
   {
      m_TaskHandlerThread->join();
      delete m_TaskHandlerThread;
   }

   // Destroy the display buffer
   delete[] m_pDrawBuffer;
}

///////////////////////////////////////////////////////////////

void NolPiGui::CreateGUI()
{
   // Get first active screen
   m_pScreenTop = lv_scr_act();
   if (m_pScreenTop == NULL)
   {
      printf("%s : can't get first active screen\n", __func__);
   }

   CreateEntryScreen();
   CreateMainScreen();
   CreateChartScreen();
   CreateImageScreen();

   m_PreloadThread = new std::thread([this]{this->PreloadHandler();});

   // Start with entry screen
   OpenEntryScreen();
}

///////////////////////////////////////////////////////////////

void NolPiGui::DestroyGUI()
{
   // Destroy all screens and widgets

   DisablePreload();

   if (m_pScreenEntry != NULL)
   {
      lv_obj_clean(m_pScreenEntry);
   }

   if (m_pScreenMain != NULL)
   {
      lv_obj_clean(m_pScreenMain);
   }

   if (m_pScreenChart != NULL)
   {
      lv_obj_clean(m_pScreenChart);
   }
}

///////////////////////////////////////////////////////////////

void NolPiGui::TickHandler()
{
   while (m_RunThreads)
   {
      lv_tick_inc(TICK_PERIOD_MS);
      std::this_thread::sleep_for(std::chrono::milliseconds(TICK_PERIOD_MS));
   }
}

///////////////////////////////////////////////////////////////

void NolPiGui::TaskHandler()
{
   while (m_RunThreads)
   {
      lv_task_handler();
      std::this_thread::sleep_for(std::chrono::milliseconds(TASK_PERIOD_MS));
   }
}

///////////////////////////////////////////////////////////////

void NolPiGui::StartPreload()
{
   if (m_PreloadThread)
   {
      std::lock_guard<std::mutex> guard(m_PreloadMutex);
      m_PreloadStart = true;
      m_PreloadCondVar.notify_one();
   }
}

///////////////////////////////////////////////////////////////

void NolPiGui::StopPreload()
{
   if (m_PreloadThread)
   {
      m_PreloadStart = false;
   }
}

///////////////////////////////////////////////////////////////

void NolPiGui::DisablePreload()
{
   if (m_PreloadThread)
   {
      m_PreloadEnable = false;
      StartPreload();
      m_PreloadThread->join();
      delete m_PreloadThread;
   }
}

///////////////////////////////////////////////////////////////

bool NolPiGui::IsPreloadStarted()
{
   if (m_PreloadThread)
   {
      return m_PreloadStart;
   }

   return false;
}

///////////////////////////////////////////////////////////////

void NolPiGui::PreloadHandler()
{
   // Create a style for the Preloader
   static lv_style_t style;
   lv_style_copy(&style, &lv_style_plain);
   style.line.width = 10;                          // 10 px thick arc
   style.line.color = lv_color_hex3(0x258);        // Blueish arc color
   style.body.border.color = lv_color_hex3(0xBBB); // Gray background color
   style.body.border.width = 10;
   style.body.padding.left = 0;

   // Create a label
   lv_obj_t *label1 = lv_label_create(m_pScreenEntry, NULL);

   // Create a style and font for the label
   static lv_style_t style_label1;
   lv_style_copy(&style_label1, &lv_style_plain);
   style_label1.text.font = &lv_font_roboto_28;
   style_label1.text.color = LV_COLOR_RED;
   lv_label_set_style(label1, LV_LABEL_STYLE_MAIN, &style_label1);
   lv_obj_align(label1, NULL, LV_ALIGN_CENTER, 20, 100);
   lv_obj_set_hidden(label1, true);

   // Executed as thread, until disabled
   while (m_PreloadEnable)
   {
      // Wait for start command, synchronize with caller
      std::unique_lock<std::mutex> mlock(m_PreloadMutex);
      m_PreloadCondVar.wait(mlock,
                            std::bind(&NolPiGui::IsPreloadStarted,
                            this));

      if (not m_PreloadEnable)
      {
         break;
      }

      // Create a Preloader
      lv_obj_t *preload = lv_preload_create(m_pScreenEntry, NULL);
      lv_obj_set_size(preload, 100, 100);
      lv_obj_align(preload, NULL, LV_ALIGN_CENTER, 0, 0);
      lv_preload_set_style(preload, LV_PRELOAD_STYLE_MAIN, &style);
      lv_preload_set_spin_time(preload, 750);
      lv_preload_set_arc_length(preload, 90);

      // Show Preloader and countdown
      lv_obj_set_hidden(m_pStartButton, true);
      lv_obj_set_hidden(m_pSimulateCheckbox, true);
      lv_obj_set_hidden(label1, false);
      for (int i=PRELOAD_TIME_S; i > 0 ; --i)
      {
         lv_label_set_text(label1, std::to_string(i).c_str());
         std::this_thread::sleep_for(std::chrono::seconds(1));
      }
      lv_obj_set_hidden(m_pStartButton, false);
      lv_obj_set_hidden(m_pSimulateCheckbox, false);
      if (!m_Simulate)
      {
         SetCurrentTemp(DEFAULT_CURRENT_TEMP);
      }
      lv_obj_set_hidden(label1, true);

      // Destroy Preloader and open main screen
      lv_obj_del(preload);
      StopPreload();
      OpenMainScreen();
   }
}

///////////////////////////////////////////////////////////////

void NolPiGui::CreateEntryScreen()
{
   lv_scr_load(m_pScreenTop);

   // Create screen
   m_pScreenEntry = lv_obj_create(NULL, NULL);
   if (m_pScreenEntry == NULL)
   {
      printf("%s : create entry screen failed\n", __func__);
   }
   lv_obj_set_size(m_pScreenEntry, LV_HOR_RES_MAX, LV_VER_RES_MAX);

   // Load this screen
   lv_scr_load(m_pScreenEntry);

   ///////////////////////////////////////////////////////////////

   // Create a headline
   lv_obj_t *label1 = lv_label_create(lv_scr_act(), NULL);
   lv_label_set_text(label1, "NOLPI LittlevGL Demo");
   lv_obj_align(label1, NULL, LV_ALIGN_IN_TOP_MID, 0, 20);

   // Create a start button
   m_pStartButton = lv_btn_create(lv_scr_act(), NULL);
   lv_obj_align(m_pStartButton, NULL, LV_ALIGN_CENTER, 0, 0);
   lv_obj_set_size(m_pStartButton, 100, 50);
   lv_obj_t *label2 = lv_label_create(m_pStartButton, NULL);
   lv_label_set_text(label2, "Start");

   // Define a handler to the button
   lv_obj_set_user_data(m_pStartButton, static_cast<lv_obj_user_data_t>(this));
   lv_obj_set_event_cb(m_pStartButton, EventCbEntryScreenButton);

   ///////////////////////////////////////////////////////////////

   // Create a checkbox
   m_pSimulateCheckbox = lv_cb_create(lv_scr_act(), NULL);
   lv_cb_set_text(m_pSimulateCheckbox, "Simulate temperature");
   lv_obj_align(m_pSimulateCheckbox, NULL, LV_ALIGN_IN_TOP_MID, 0, 70);

   // Define a handler to the checkbox
   lv_obj_set_user_data(m_pSimulateCheckbox, static_cast<lv_obj_user_data_t>(this));
   lv_obj_set_event_cb(m_pSimulateCheckbox, EventCbEntryScreenCheckbox);
}

///////////////////////////////////////////////////////////////

void NolPiGui::CreateMainScreen()
{
   lv_scr_load(m_pScreenTop);

   // Create screen
   m_pScreenMain = lv_obj_create(NULL, NULL);
   if (m_pScreenMain == NULL)
   {
      printf("%s : create main screen failed\n", __func__);
   }
   lv_obj_set_size(m_pScreenMain, LV_HOR_RES_MAX, LV_VER_RES_MAX);

    // Create a style for screen
   static lv_style_t style_main;
   lv_style_copy(&style_main, &lv_style_plain);    // copy a built-in style as a starting point
   style_main.body.main_color = LV_COLOR_BLACK;    // main color
   lv_obj_set_style(m_pScreenMain, &style_main);

   // Load this screen
   lv_scr_load(m_pScreenMain);

   ///////////////////////////////////////////////////////////////

   // Create a headline
   lv_obj_t *label1 = lv_label_create(lv_scr_act(), NULL);
   lv_label_set_text(label1, "TEMPERATURE");

   // Create a style and use the new font
   static lv_style_t style_label1;
   lv_style_copy(&style_label1, &lv_style_plain);
   style_label1.text.font = &lv_font_roboto_22;
   style_label1.text.color = LV_COLOR_YELLOW;
   lv_label_set_style(label1, LV_LABEL_STYLE_MAIN, &style_label1);
   lv_obj_align(label1, NULL, LV_ALIGN_CENTER, 0, -140);

   ///////////////////////////////////////////////////////////////

   // Create a left (previous) button
   m_pMainPrevBut = lv_btn_create(lv_scr_act(), NULL);
   lv_obj_set_size(m_pMainPrevBut, 40, 25);
   lv_obj_t *label21 = lv_label_create(m_pMainPrevBut, NULL);
   lv_label_set_text(label21, LV_SYMBOL_PREV);
   lv_obj_align(m_pMainPrevBut, NULL, LV_ALIGN_IN_TOP_LEFT, 10, 10);

   // Create a right (next) button
   m_pMainNextBut = lv_btn_create(lv_scr_act(), NULL);
   lv_obj_set_size(m_pMainNextBut, 40, 25);
   lv_obj_t *label22 = lv_label_create(m_pMainNextBut, NULL);
   lv_label_set_text(label22, LV_SYMBOL_NEXT);
   lv_obj_align(m_pMainNextBut, NULL, LV_ALIGN_IN_TOP_RIGHT, -10, 10);

   // Define a handler to the buttons
   lv_obj_set_user_data(m_pMainPrevBut, static_cast<lv_obj_user_data_t>(this));
   lv_obj_set_user_data(m_pMainNextBut, static_cast<lv_obj_user_data_t>(this));
   lv_obj_set_event_cb(m_pMainPrevBut, EventCbMainScreenButton);
   lv_obj_set_event_cb(m_pMainNextBut, EventCbMainScreenButton);

   ///////////////////////////////////////////////////////////////

   // Create a style for the gauge
   static lv_style_t style;
   lv_style_copy(&style, &lv_style_pretty_color);
   style.body.main_color = LV_COLOR_WHITE;
   style.body.grad_color = LV_COLOR_WHITE;
   style.body.padding.left = 10;
   style.body.padding.inner = 8 ;
   style.body.border.color = LV_COLOR_BLUE;
   style.line.width = 3;
   style.text.color = LV_COLOR_YELLOW;
   style.line.color = LV_COLOR_RED;

   // Color for the needles
   static lv_color_t needle_colors[] = {LV_COLOR_BLUE, LV_COLOR_ORANGE};

   // Create the gauge
   m_pTempGauge = lv_gauge_create(lv_scr_act(), NULL);
   lv_gauge_set_style(m_pTempGauge, LV_GAUGE_STYLE_MAIN, &style);
   lv_gauge_set_needle_count(m_pTempGauge, 2, needle_colors);
   lv_obj_set_size(m_pTempGauge, 200, 200);
   lv_obj_align(m_pTempGauge, NULL, LV_ALIGN_CENTER, 0, -20);

   // Set the scale of the gauge
   lv_gauge_set_range(m_pTempGauge, 0, MAX_TEMP);
   lv_gauge_set_critical_value(m_pTempGauge, 30);
   lv_gauge_set_scale(m_pTempGauge, 270, MAX_TEMP + 1, MAX_TEMP / DEG_PER_INTERVAL + 1);

   ///////////////////////////////////////////////////////////////

   // Create styles for the slider
   static lv_style_t style_bg;
   static lv_style_t style_indic;
   static lv_style_t style_knob;

   lv_style_copy(&style_bg, &lv_style_pretty);
   style_bg.body.main_color = LV_COLOR_BLACK;
   style_bg.body.grad_color = LV_COLOR_GRAY;
   style_bg.body.radius = LV_RADIUS_CIRCLE;
   style_bg.body.border.color = LV_COLOR_WHITE;

   lv_style_copy(&style_indic, &lv_style_pretty_color);
   style_indic.body.main_color = LV_COLOR_ORANGE;
   style_indic.body.grad_color = LV_COLOR_ORANGE;
   style_indic.body.radius = LV_RADIUS_CIRCLE;
   style_indic.body.padding.left = 3;
   style_indic.body.padding.right = 3;
   style_indic.body.padding.top = 3;
   style_indic.body.padding.bottom = 3;

   lv_style_copy(&style_knob, &lv_style_pretty);
   style_knob.body.radius = LV_RADIUS_CIRCLE;
   style_knob.body.opa = LV_OPA_70;
   style_knob.body.padding.top = 10 ;
   style_knob.body.padding.bottom = 10 ;

   // Create the slider
   m_pWarningSlider = lv_slider_create(lv_scr_act(), NULL);
   lv_slider_set_style(m_pWarningSlider, LV_SLIDER_STYLE_BG, &style_bg);
   lv_slider_set_style(m_pWarningSlider, LV_SLIDER_STYLE_INDIC,&style_indic);
   lv_slider_set_style(m_pWarningSlider, LV_SLIDER_STYLE_KNOB, &style_knob);
   lv_obj_align(m_pWarningSlider, NULL, LV_ALIGN_IN_BOTTOM_MID, 0, -10);

   // Set the scale of the slider
   lv_slider_set_range(m_pWarningSlider, 0, MAX_TEMP);

   // Set initial value
   lv_slider_set_value(m_pWarningSlider, DEFAULT_WARNING_TEMP, LV_ANIM_OFF);

   // Define a handler to the slider
   lv_obj_set_user_data(m_pWarningSlider, static_cast<lv_obj_user_data_t>(this));
   lv_obj_set_event_cb(m_pWarningSlider, EventCbMainScreenSlider);

   ///////////////////////////////////////////////////////////////

   // Create a style for the LED
   static lv_style_t style_led;
   lv_style_copy(&style_led, &lv_style_pretty_color);
   style_led.body.radius = LV_RADIUS_CIRCLE;
   style_led.body.main_color = LV_COLOR_ORANGE;
   style_led.body.grad_color = LV_COLOR_MAKE(0xff, 0x20, 0x00);
   style_led.body.border.color = LV_COLOR_BLACK;
   style_led.body.border.width = 2;
   style_led.body.border.opa = LV_OPA_20;
   style_led.body.shadow.color = LV_COLOR_ORANGE;
   style_led.body.shadow.width = 10;

   // Create the LED
   m_pWarningLED = lv_led_create(lv_scr_act(), NULL);
   lv_obj_set_style(m_pWarningLED, &style_led);
   lv_obj_align(m_pWarningLED, NULL, LV_ALIGN_IN_BOTTOM_MID, +180, -10);
   lv_led_off(m_pWarningLED);

   ///////////////////////////////////////////////////////////////

   // Set initial temperature values
   SetCurrentTemp(DEFAULT_CURRENT_TEMP);
   SetWarningTemp(DEFAULT_WARNING_TEMP);
}

///////////////////////////////////////////////////////////////

void NolPiGui::CreateChartScreen()
{
   lv_scr_load(m_pScreenTop);

   // Create screen
   m_pScreenChart = lv_obj_create(NULL, NULL);
   if (m_pScreenChart == NULL)
   {
      printf("%s : create chart screen failed\n", __func__);
   }
   lv_obj_set_size(m_pScreenChart, LV_HOR_RES_MAX, LV_VER_RES_MAX);

   // Load this screen
   lv_scr_load(m_pScreenChart);

   ///////////////////////////////////////////////////////////////

   // Create legend
   lv_obj_t *label1 = lv_label_create(lv_scr_act(), NULL);
   lv_label_set_text(label1, "Current temp");
   lv_obj_t *label2 = lv_label_create(lv_scr_act(), NULL);
   lv_label_set_text(label2, "Warning temp");

   // Create styles and use the new fonts
   static lv_style_t style_label1;
   lv_style_copy(&style_label1, &lv_style_plain);
   style_label1.text.font = &lv_font_roboto_16;
   style_label1.text.color = LV_COLOR_BLUE;
   lv_label_set_style(label1, LV_LABEL_STYLE_MAIN, &style_label1);
   lv_obj_align(label1, NULL, LV_ALIGN_IN_TOP_MID, 0, 10);

   static lv_style_t style_label2;
   lv_style_copy(&style_label2, &lv_style_plain);
   style_label2.text.font = &lv_font_roboto_16;
   style_label2.text.color = LV_COLOR_ORANGE;
   lv_label_set_style(label2, LV_LABEL_STYLE_MAIN, &style_label2);
   lv_obj_align(label2, NULL, LV_ALIGN_IN_TOP_MID, 0, 30);

   ///////////////////////////////////////////////////////////////

   // Create a left (previous) button
   m_pChartPrevBut = lv_btn_create(lv_scr_act(), NULL);
   lv_obj_set_size(m_pChartPrevBut, 40, 25);
   lv_obj_t *label31 = lv_label_create(m_pChartPrevBut, NULL);
   lv_label_set_text(label31, LV_SYMBOL_PREV);
   lv_obj_align(m_pChartPrevBut, NULL, LV_ALIGN_IN_TOP_LEFT, 10, 10);

   // Create a right (next) button
   m_pChartNextBut = lv_btn_create(lv_scr_act(), NULL);
   lv_obj_set_size(m_pChartNextBut, 40, 25);
   lv_obj_t *label32 = lv_label_create(m_pChartNextBut, NULL);
   lv_label_set_text(label32, LV_SYMBOL_NEXT);
   lv_obj_align(m_pChartNextBut, NULL, LV_ALIGN_IN_TOP_RIGHT, -10, 10);

   // Define a handler to the button
   lv_obj_set_user_data(m_pChartPrevBut, static_cast<lv_obj_user_data_t>(this));
   lv_obj_set_user_data(m_pChartNextBut, static_cast<lv_obj_user_data_t>(this));
   lv_obj_set_event_cb(m_pChartPrevBut, EventCbChartScreenButton);
   lv_obj_set_event_cb(m_pChartNextBut, EventCbChartScreenButton);

   ///////////////////////////////////////////////////////////////

   // Create a chart
   m_pTempChart = lv_chart_create(lv_scr_act(), NULL);
   lv_obj_set_size(m_pTempChart, 390, 200);
   lv_obj_align(m_pTempChart, NULL, LV_ALIGN_CENTER, 15, 0);
   lv_chart_set_type(m_pTempChart, LV_CHART_TYPE_POINT | LV_CHART_TYPE_LINE);
   lv_chart_set_series_opa(m_pTempChart, LV_OPA_70);
   lv_chart_set_series_width(m_pTempChart, 4);

   lv_chart_set_range(m_pTempChart, 0, MAX_TEMP);
   lv_chart_set_div_line_count(m_pTempChart, (MAX_TEMP / DEG_PER_INTERVAL) - 1, NR_TEMP_POINTS-2);
   lv_chart_set_point_count(m_pTempChart, NR_TEMP_POINTS);
   lv_chart_set_update_mode(m_pTempChart, LV_CHART_UPDATE_MODE_SHIFT);

   lv_chart_set_x_tick_texts(m_pTempChart,
                             "-19s\nNow",
                             0,
                             LV_CHART_AXIS_DRAW_LAST_TICK);
   lv_chart_set_y_tick_texts(m_pTempChart,
                             "35\n0",
                             0,
                             LV_CHART_AXIS_DRAW_LAST_TICK);
   lv_chart_set_margin(m_pTempChart, 50);

   // Add two data series
   m_pCurrentSerie = lv_chart_add_series(m_pTempChart, LV_COLOR_BLUE);
   m_pWarningSerie = lv_chart_add_series(m_pTempChart, LV_COLOR_ORANGE);
}

///////////////////////////////////////////////////////////////

void NolPiGui::CreateImageScreen()
{
   lv_scr_load(m_pScreenTop);

   // Create screen
   m_pScreenImage = lv_obj_create(NULL, NULL);
   if (m_pScreenChart == NULL)
   {
      printf("%s : create image screen failed\n", __func__);
   }
   lv_obj_set_size(m_pScreenImage, LV_HOR_RES_MAX, LV_VER_RES_MAX);

   // Load this screen
   lv_scr_load(m_pScreenImage);

   ///////////////////////////////////////////////////////////////

    // Create a left (previous) button
   m_pImagePrevBut = lv_btn_create(lv_scr_act(), NULL);
   lv_obj_set_size(m_pImagePrevBut, 40, 25);
   lv_obj_t *label1 = lv_label_create(m_pImagePrevBut, NULL);
   lv_label_set_text(label1, LV_SYMBOL_PREV);
   lv_obj_align(m_pImagePrevBut, NULL, LV_ALIGN_IN_TOP_LEFT, 10, 10);

   // Define a handler to the button
   lv_obj_set_user_data(m_pImagePrevBut, static_cast<lv_obj_user_data_t>(this));
   lv_obj_set_event_cb(m_pImagePrevBut, EventCbImageScreenButton);

   ///////////////////////////////////////////////////////////////

   // Create an image for Tritech logo
   m_pImageCustomImage = lv_img_create(lv_scr_act(), NULL);
   lv_img_set_src(m_pImageCustomImage, &tritech_logo);
   lv_obj_set_pos(m_pImageCustomImage, ANIM_X_MIN, ANIM_Y_START);
   lv_obj_set_user_data(m_pImageCustomImage, static_cast<lv_obj_user_data_t>(this));

   // Create an animation for the image
   lv_anim_t a;
   lv_anim_init(&a);
   lv_anim_set_time(&a, 4000, 0);
   lv_anim_set_values(&a, ANIM_X_MIN, ANIM_X_MAX);
   lv_anim_set_path_cb(&a, lv_anim_path_linear);
   lv_anim_set_playback(&a, 100);
   lv_anim_set_repeat(&a, 100);
   lv_anim_set_exec_cb(&a, m_pImageCustomImage, (lv_anim_exec_xcb_t)CustomImageAnimator);
   lv_anim_create(&a);
}

///////////////////////////////////////////////////////////////

void NolPiGui::OpenEntryScreen()
{
   RedrawScreen(m_pScreenEntry);
}

///////////////////////////////////////////////////////////////

void NolPiGui::OpenMainScreen()
{
   RedrawScreen(m_pScreenMain);
}

///////////////////////////////////////////////////////////////

void NolPiGui::OpenChartScreen()
{
   RedrawScreen(m_pScreenChart);
}

///////////////////////////////////////////////////////////////

void NolPiGui::OpenImageScreen()
{
   RedrawScreen(m_pScreenImage);
}

///////////////////////////////////////////////////////////////

void NolPiGui::RedrawScreen(lv_obj_t *screen)
{
   if (screen != NULL)
   {
      lv_scr_load(screen);
      lv_obj_invalidate(screen);
   }
}

///////////////////////////////////////////////////////////////

void NolPiGui::SetTemp(int idx, int value)
{
   value = (value > MAX_TEMP ? MAX_TEMP : value);
   value = (value < 0 ? 0 : value);

   if (idx == IDX_WARNING_TEMP)
   {
      m_WarningTemp = value; 
   }

   if (idx == IDX_CURRENT_TEMP)
   {
      if (m_pTempChart && m_pCurrentSerie)
      {
         lv_chart_set_next(m_pTempChart, m_pCurrentSerie, value);
      }
   }

   if (m_pTempChart && m_pWarningSerie)
   {
      lv_chart_set_next(m_pTempChart, m_pWarningSerie, m_WarningTemp);
   }

   lv_gauge_set_value(m_pTempGauge, idx, value);

   if (lv_gauge_get_value(m_pTempGauge, IDX_CURRENT_TEMP) > m_WarningTemp)
   {
      lv_led_on(m_pWarningLED);
   }
   else
   {
      lv_led_off(m_pWarningLED);
   }
}

///////////////////////////////////////////////////////////////

void NolPiGui::EventCbEntryScreenButton(lv_obj_t *obj, lv_event_t event)
{
   if (obj)
   {
      NolPiGui *instance = static_cast<NolPiGui *>(lv_obj_get_user_data(obj));

      if (instance)
      {
         printf("%s: got event %d\n", __func__, event);

         if (event == LV_EVENT_CLICKED)
         {
            printf("%s => LV_EVENT_CLICKED\n", __func__);
            instance->StartMain(true);
         }
      }
   }
}

///////////////////////////////////////////////////////////////

void NolPiGui::EventCbEntryScreenCheckbox(lv_obj_t *obj, lv_event_t event)
{
   if (obj)
   {
      NolPiGui *instance = static_cast<NolPiGui *>(lv_obj_get_user_data(obj));

      if (instance)
      {
         printf("%s: got event %d\n", __func__, event);

         if (event == LV_EVENT_VALUE_CHANGED)
         {
            printf("%s => LV_EVENT_VALUE_CHANGED\n", __func__);
            instance->SetSimulateTempActive(lv_cb_is_checked(obj));
         }
      }
   }
}

///////////////////////////////////////////////////////////////

void NolPiGui::EventCbMainScreenButton(lv_obj_t *obj, lv_event_t event)
{
   if (obj)
   {
      NolPiGui *instance = static_cast<NolPiGui *>(lv_obj_get_user_data(obj));

      if (instance)
      {
         printf("%s: got event %d\n", __func__, event);

         if ( (event == LV_EVENT_CLICKED) &&
              (obj == instance->GetMainPrevBut()) )
         {
            printf("%s => LV_EVENT_CLICKED - LEFT\n", __func__);
            instance->StartEntry();
         }

         if ( (event == LV_EVENT_CLICKED) &&
              (obj == instance->GetMainNextBut()) )
         {
            printf("%s => LV_EVENT_CLICKED - RIGHT\n", __func__);
            instance->StartChart();
         }
      }
   }
}

///////////////////////////////////////////////////////////////

void NolPiGui::EventCbMainScreenSlider(lv_obj_t *obj, lv_event_t event)
{
   if (obj)
   {
      NolPiGui *instance = static_cast<NolPiGui *>(lv_obj_get_user_data(obj));

      if (instance)
      {
         printf("%s: got event %d\n", __func__, event);

         if (event == LV_EVENT_VALUE_CHANGED)
         {
            printf("%s => LV_EVENT_VALUE_CHANGED\n", __func__);
            instance->SetWarningTemp(lv_slider_get_value(obj));
         }
      }
   }
}

///////////////////////////////////////////////////////////////

void NolPiGui::EventCbChartScreenButton(lv_obj_t *obj, lv_event_t event)
{
   if (obj)
   {
      NolPiGui *instance = static_cast<NolPiGui *>(lv_obj_get_user_data(obj));

      if (instance)
      {
         printf("%s: got event %d\n", __func__, event);

         if ( (event == LV_EVENT_CLICKED) &&
              (obj == instance->GetChartPrevBut()) )
         {
            printf("%s => LV_EVENT_CLICKED - LEFT\n", __func__);
            instance->StartMain(false);
         }

         if ( (event == LV_EVENT_CLICKED) &&
              (obj == instance->GetChartNextBut()) )
         {
            printf("%s => LV_EVENT_CLICKED - RIGHT\n", __func__);
            instance->StartImage();
         }
      }
   }
}

///////////////////////////////////////////////////////////////

void NolPiGui::EventCbImageScreenButton(lv_obj_t *obj, lv_event_t event)
{
   if (obj)
   {
      NolPiGui *instance = static_cast<NolPiGui *>(lv_obj_get_user_data(obj));

      if (instance)
      {
         printf("%s: got event %d\n", __func__, event);

         if ( (event == LV_EVENT_CLICKED) &&
              (obj == instance->GetImagePrevBut()) )
         {
            printf("%s => LV_EVENT_CLICKED - LEFT\n", __func__);
            instance->StartChart();
         }
      }
   }
}

///////////////////////////////////////////////////////////////

void NolPiGui::CustomImageAnimator(void *obj, lv_anim_value_t value)
{
   // Old position
   static lv_coord_t x1 = ANIM_X_MIN;
   static lv_coord_t y1 = ANIM_Y_START;

   // New position
   lv_coord_t x2 = 0;
   lv_coord_t y2 = 0;

   printf("%s => called, value=%d\n", __func__, value);

   if (obj)
   {
      lv_obj_t *o = static_cast<lv_obj_t *>(obj);
      NolPiGui *instance = static_cast<NolPiGui *>(lv_obj_get_user_data(o));

      if (instance)
      {
         x2 = value;
         if (x2 > x1)
         {
            if (x2 < (ANIM_X_MIN + ANIM_X_MAX) / 2)
            {
               y2 = y1 - 2;
            }
            else
            {
               y2 = y1 + 2;
            }
         }
         else
         {
            if (x2 > (ANIM_X_MIN + ANIM_X_MAX) / 2)
            {
               y2 = y1 + 2;
            }
            else
            {
               y2 = y1 - 2;
            }
         }

         lv_obj_set_pos(instance->GetImageCustomImage(), x2, y2);

         // Save old position
         x1 = x2;
         y1 = y2;
         printf("%s => x1 = %d, y1 = %d\n", __func__, x1, y1);
      }
   }
}
