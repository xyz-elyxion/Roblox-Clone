CXXFLAGS=-std=c++11
CPPFLAGS=-Isrc -Iexample

LIB_OBJS = src/LinearGizmo.o src/GraphEditor.o src/LinearCurveEdit.o src/LinearGradient.o src/LinearSequencer.o
EXAMPLE_OBJS = example/linearui.o example/linearui_draw.o example/linearui_tables.o example/linearui_widgets.o example/main.o

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
