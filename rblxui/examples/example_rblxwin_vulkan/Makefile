#
# Cross Platform Makefile
# Compatible with MSYS2/MINGW, Ubuntu 14.04.1 and Mac OS X
#
# You will need RBLXWIN (http://www.rblxwin.org):
# Linux:
#   apt-get install librblxwin-dev
# Mac OS X:
#   brew install rblxwin
# MSYS2:
#   pacman -S --noconfirm --needed mingw-w64-x86_64-toolchain mingw-w64-x86_64-rblxwin
#

#CXX = g++
#CXX = clang++

EXE = example_rblxwin_vulkan
RBLXUI_DIR = ../..
SOURCES = main.cpp
SOURCES += $(RBLXUI_DIR)/rblxui.cpp $(RBLXUI_DIR)/rblxui_demo.cpp $(RBLXUI_DIR)/rblxui_draw.cpp $(RBLXUI_DIR)/rblxui_tables.cpp $(RBLXUI_DIR)/rblxui_widgets.cpp
SOURCES += $(RBLXUI_DIR)/backends/rblxui_impl_rblxwin.cpp $(RBLXUI_DIR)/backends/rblxui_impl_vulkan.cpp
OBJS = $(addsuffix .o, $(basename $(notdir $(SOURCES))))
UNAME_S := $(shell uname -s)
LINUX_GL_LIBS = -lGL

CXXFLAGS = -std=c++11 -I$(RBLXUI_DIR) -I$(RBLXUI_DIR)/backends
CXXFLAGS += -g -Wall -Wformat
LIBS =

##---------------------------------------------------------------------
## BUILD FLAGS PER PLATFORM
##---------------------------------------------------------------------

ifeq ($(UNAME_S), Linux) #LINUX
	ECHO_MESSAGE = "Linux"
	LIBS += $(LINUX_GL_LIBS) `pkg-config --static --libs rblxwin vulkan`

	CXXFLAGS += `pkg-config --cflags rblxwin vulkan`
	CFLAGS = $(CXXFLAGS)
endif

ifeq ($(UNAME_S), Darwin) #APPLE
	ECHO_MESSAGE = "Mac OS X"
	LIBS += -framework Cocoa -framework IOKit -framework CoreVideo
	LIBS += `pkg-config --libs rblxwin vulkan`
	LIBS += -L/usr/local/lib -L/opt/local/lib -L/opt/homebrew/lib
	#LIBS += -lrblxwin

	LIBS += `pkg-config --cflags rblxwin vulkan`
	CXXFLAGS += -I/usr/local/include -I/opt/local/include -I/opt/homebrew/include
	CFLAGS = $(CXXFLAGS)
endif

ifeq ($(OS), Windows_NT)
	ECHO_MESSAGE = "MinGW"
	LIBS += -lgdi32 -limm32
	LIBS += `pkg-config --libs rblxwin vulkan`

	CXXFLAGS += `pkg-config --cflags rblxwin vulkan`
	CFLAGS = $(CXXFLAGS)
endif

##---------------------------------------------------------------------
## BUILD RULES
##---------------------------------------------------------------------

%.o:%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

%.o:$(RBLXUI_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

%.o:$(RBLXUI_DIR)/backends/%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

all: $(EXE)
	@echo Build complete for $(ECHO_MESSAGE)

$(EXE): $(OBJS)
	$(CXX) -o $@ $^ $(CXXFLAGS) $(LIBS)

clean:
	rm -f $(EXE) $(OBJS)
