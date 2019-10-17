SUBDIRS = src 
TESTDIR = test
BUILDDIR = build
CC = g++
CFLAG = -std=c++11
INCS = -I./include
SRCS = $(shell find $(SUBDIRS) -name '*.cpp')
OBJS = $(patsubst $(SUBDIRS)/%.cpp, $(BILDIR)/%.o,$(SRCS))
PROJECT = npshell

##.SECONDEXPANSION:
$(BUILDDIR)/$(PROJECT): $(OBJS) $(BUILDDIR) # | $$(dir $$@)
	$(CC) $(CFLAG) $(OBJS) $(INCS) -o $@

%.o: %.cpp
	$(CC) $(CFLAG) -c $< $(INCS) -o $@

$(BUILDDIR):
	mkdir -p $(BUILDDIR)
.PRECIOUS : $(BUILDDIR)

.PHONY: test
test:
	$(MAKE) -C $(TESTDIR)

.PHONY: clean
clean:
	rm $(BUILDDIR)/*	
