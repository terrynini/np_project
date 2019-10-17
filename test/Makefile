CC = g++
CFLAG = -std=c++11
INCS = -I../include
export BUILDDIR
SRCS = $(shell find . -name '*.cpp')
OBJS = $(patsubst %.cpp,%.o,$(SRCS))
PROJECT = Test

all: $(PROJECT)
	./$(PROJECT)
	
$(PROJECT): $(OBJS)
	$(CC) $(CFLAG) $(OBJS) $(INCS) -o $@
	

%.o: %.cpp
	$(CC) $(CFLAG) -c $< $(INCS) -o $@

clean:
	rm $(PROJECT) *.o