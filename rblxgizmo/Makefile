CXXFLAGS=-std=c++11
CPPFLAGS=-Isrc -Iexample

LIB_OBJS = src/RblxGizmo.o src/GraphEditor.o src/RblxCurveEdit.o src/RblxGradient.o src/RblxSequencer.o
EXAMPLE_OBJS = example/rblxui.o example/rblxui_draw.o example/rblxui_tables.o example/rblxui_widgets.o example/main.o

EXAMPLE_NAME = example.exe
LDFLAGS=-mwindows -static-libgcc -static-libstdc++
LIBS=-limm32 -lopengl32 -lgdi32

$(EXAMPLE_NAME): $(LIB_OBJS) $(EXAMPLE_OBJS)
	$(CXX) $(LDFLAGS) -o $@ $^ $(LIBS)

example/main.o: CXXFLAGS := -std=c++17

clean:
	$(RM) $(LIB_OBJS)
	$(RM) $(EXAMPLE_OBJS)
	$(RM) $(EXAMPLE_NAME)
