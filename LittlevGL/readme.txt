LittlevGL is a free and open-source Graphical User Interface (GUI) library.
For further information, see https://littlevgl.com

Directory structure:

|--- LittlevGL.mak
|
|--- lv_conf.h
|
|--- lv_drivers
|
|--- lv_drv_conf.h
|
|--- lvgl
|
|--- readme.txt


-------------------
-- LittlevGL.mak
-------------------
Main makefile, adapted to project build infrastructure.

-------------------
-- lv_conf.h
-------------------
Configuration file with build options for LittlevGL.
This includes graphical setting, memory management and more.

-------------------
-- lv_drivers
-------------------
LittlevGL drivers for input/output devices.

git clone https://github.com/littlevgl/lv_drivers.git
commit d41fa895125359d0675bcea31a1e05cdeae60f4c

-------------------
-- lv_drv_conf.h
-------------------
Configuration file with build options for LittlevGL drivers.
This includes options for various input/output devices.

-------------------
-- lvgl
-------------------
LittlevGL core components with graphical objects and features.

git clone https://github.com/littlevgl/lvgl.git
commit 8e9764532d866719b5a87f0e2181f7d57c501592

-------------------
-- readme.txt
-------------------
This text file.
