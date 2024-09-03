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
CASEDIR = case

# change here!!
TESTINPUT = $(CASEDIR)/case1.in
TESTOUTPUT = case1.out

TARGET = Partition

all: $(TARGET)

$(OBJDIR):
	@mkdir $(OBJDIR)

$(TARGET): main.cpp $(OBJS)
	$(CXX) $(WARNINGS) $(CXXFLAGS) $(OPTFLAGS) $^ -o $@

$(OBJDIR)/FM_Partition.o: $(SRCDIR)/FM_Partition.cpp | $(OBJDIR)
	$(CXX) $(WARNINGS) $(CXXFLAGS) $(OPTFLAGS) -c $< -o $@

$(OBJDIR)/myLinkedList.o: $(SRCDIR)/myLinkedList.cpp | $(OBJDIR)
	$(CXX) $(WARNINGS) $(CXXFLAGS) $(OPTFLAGS) -c $< -o $@

run:
	./Partition $(TESTINPUT) $(TESTOUTPUT)

check:
	./SolutionChecker $(TESTINPUT) $(TESTOUTPUT)

clean:
	rm -rf $(OBJDIR) $(TARGET) *.out
