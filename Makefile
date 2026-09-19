CXX = g++
TARGET = EditorCajasV3

CXXFLAGS = -std=c++17 $(shell pkg-config --cflags gtk4)
LDLIBS = $(shell pkg-config --libs gtk4)

SOURCES = EditorCajasV0.cpp \
	dialogo_archivo.cpp \
	editor_interfaz.cpp \
	procesador_celdas.cpp

OBJECTS = $(SOURCES:.cpp=.o)

.PHONY: all clean run

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CXX) $(OBJECTS) -o $@ $(LDLIBS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(OBJECTS) $(TARGET)
