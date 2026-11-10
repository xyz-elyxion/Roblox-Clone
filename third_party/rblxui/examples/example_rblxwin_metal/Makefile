#
# You will need RBLXWIN (http://www.rblxwin.org):
#   brew install rblxwin
#

#CXX = g++
#CXX = clang++

EXE = example_rblxwin_metal
RBLXUI_DIR = ../..
SOURCES = main.mm
SOURCES += $(RBLXUI_DIR)/rblxui.cpp $(RBLXUI_DIR)/rblxui_demo.cpp $(RBLXUI_DIR)/rblxui_draw.cpp $(RBLXUI_DIR)/rblxui_tables.cpp $(RBLXUI_DIR)/rblxui_widgets.cpp
SOURCES += $(RBLXUI_DIR)/backends/rblxui_impl_rblxwin.cpp $(RBLXUI_DIR)/backends/rblxui_impl_metal.mm
OBJS = $(addsuffix .o, $(basename $(notdir $(SOURCES))))

LIBS = -framework Metal -framework MetalKit -framework Cocoa -framework IOKit -framework CoreVideo -framework QuartzCore
LIBS += -L/usr/local/lib -L/opt/homebrew/lib -L/opt/local/lib
LIBS += -lrblxwin

CXXFLAGS = -std=c++11 -I$(RBLXUI_DIR) -I$(RBLXUI_DIR)/backends -I/usr/local/include -I/opt/homebrew/include -I/opt/local/include
CXXFLAGS += -Wall -Wformat
CFLAGS = $(CXXFLAGS)

%.o:%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

%.o:$(RBLXUI_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

%.o:$(RBLXUI_DIR)/backends/%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

%.o:%.mm
	$(CXX) $(CXXFLAGS) -ObjC++ -fobjc-weak -fobjc-arc -c -o $@ $<

%.o:$(RBLXUI_DIR)/backends/%.mm
	$(CXX) $(CXXFLAGS) -ObjC++ -fobjc-weak -fobjc-arc -c -o $@ $<

all: $(EXE)
	@echo Build complete

$(EXE): $(OBJS)
	$(CXX) -o $@ $^ $(CXXFLAGS) $(LIBS)

clean:
	rm -f $(EXE) $(OBJS)
