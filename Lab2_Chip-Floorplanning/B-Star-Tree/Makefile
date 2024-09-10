# compiler settings 
CXX = g++
CXXFLAGS = -I ./inc -std=c++11
OPTFLAGS = -march=native -flto -funroll-loops -finline-functions -ffast-math -O3
WARNINGS = -g -Wall

# Source files and object files
SRCDIR = src
OBJDIR = obj
INCDIR = inc
SRCS = $(wildcard $(SRCDIR)/*.cpp)
OBJS = $(patsubst $(SRCDIR)/%.cpp,$(OBJDIR)/%.o,$(SRCS))

# testcase file
CASEDIR = testcase
OUTPUTDIR = output
FIGDIR = fig

# change here!!
TESTINPUT = case2.txt
TESTOUTPUT = $(TESTINPUT).out

TARGET = Floorplan

all: $(TARGET)

$(OBJDIR):
	@mkdir $(OBJDIR)

$(OUTPUTDIR):
	@mkdir $(OUTPUTDIR)

$(FIGDIR):
	@mkdir $(FIGDIR)

$(TARGET): main.cpp $(OBJS)
	$(CXX) $(WARNINGS) $(CXXFLAGS) $(OPTFLAGS) $^ -o $@

$(OBJDIR)/%.o: $(SRCDIR)/%.cpp | $(OBJDIR)
	$(CXX) $(WARNINGS) $(CXXFLAGS) $(OPTFLAGS) -c $< -o $@

run: | $(OUTPUTDIR)
	./$(TARGET) $(CASEDIR)/$(TESTINPUT) $(OUTPUTDIR)/$(TESTOUTPUT)

check:
	./$(CASEDIR)/SolutionChecker $(CASEDIR)/$(TESTINPUT) $(OUTPUTDIR)/$(TESTOUTPUT)

figure: | $(FIGDIR)
	./visualizer $(CASEDIR)/$(TESTINPUT) $(OUTPUTDIR)/$(TESTOUTPUT) $(FIGDIR)/$(TESTINPUT)

clean:
	rm -rf $(OBJDIR) $(TARGET) $(OUTPUTDIR) $(FIGDIR)