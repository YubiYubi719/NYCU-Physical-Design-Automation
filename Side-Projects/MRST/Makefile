# compiler settings 
CXX = g++
CXXFLAGS = -Iinc -I/usr/include/jsoncpp/ -std=c++17
OPTFLAGS = -flto -funroll-loops -finline-functions -O3
WARNINGS = -g -Wall
LIBS = -ljsoncpp

# Source files and object files
SRCDIR = src
OBJDIR = obj
INCDIR = inc
SRCS = $(wildcard $(SRCDIR)/*.cpp)
OBJS = $(patsubst $(SRCDIR)/%.cpp,$(OBJDIR)/%.o,$(SRCS))
OUTPUTDIR = graph_json
PYDIR = visualizer

# testcase file
TARGET = main

all: $(TARGET)

$(OBJDIR):
	@mkdir $(OBJDIR)

$(TARGET): $(TARGET).cpp $(OBJS)
	$(CXX) $(WARNINGS) $(CXXFLAGS) $(OPTFLAGS) $^ $(LIBS) -o $@

$(OBJDIR)/%.o: $(SRCDIR)/%.cpp | $(OBJDIR)
	$(CXX) $(WARNINGS) $(CXXFLAGS) $(OPTFLAGS) -c $< $(LIBS) -o $@

run:
	./$(TARGET)

step1:
	python $(PYDIR)/visualize.py $(OUTPUTDIR)/ST

step2:
	python $(PYDIR)/visualize_Kruskal.py $(OUTPUTDIR)/MST

step3:
	python $(PYDIR)/visualize.py $(OUTPUTDIR)/steiner

graph:
	python $(PYDIR)/draw.py $(OUTPUTDIR)/graph.inf

test:
	valgrind ./$(TARGET)

clean:
	rm -rf $(OBJDIR) $(TARGET) $(OUTPUTDIR)