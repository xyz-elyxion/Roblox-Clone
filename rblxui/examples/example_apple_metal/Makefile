# Makefile for example_apple_metal, for macOS only (**not iOS**)
CXX = clang++
EXE = example_apple_metal
RBLXUI_DIR = ../../
SOURCES = main.mm
SOURCES += $(RBLXUI_DIR)/rblxui.cpp $(RBLXUI_DIR)/rblxui_demo.cpp $(RBLXUI_DIR)/rblxui_draw.cpp $(RBLXUI_DIR)/rblxui_tables.cpp $(RBLXUI_DIR)/rblxui_widgets.cpp
SOURCES += $(RBLXUI_DIR)/backends/rblxui_impl_osx.mm $(RBLXUI_DIR)/backends/rblxui_impl_metal.mm

CXXFLAGS = -std=c++11 -ObjC++ -fobjc-arc -Wall -Wextra -I$(RBLXUI_DIR) -I$(RBLXUI_DIR)/backends
FRAMEWORKS = -framework AppKit -framework Metal -framework MetalKit -framework QuartzCore -framework GameController

all: $(EXE)

$(EXE): $(SOURCES)
	$(CXX) $(CXXFLAGS) $^ $(FRAMEWORKS) -o $@

run: all
	./$(EXE)

clean:
	rm -f $(EXE) *.o
