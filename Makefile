CC = g++
CFLAG = -std=c++11
INCS = -I./include
BUILDDIR = build
SRCDIRS = src
SRCS = $(shell find $(SRCDIRS) -name '*.cpp')
PROJECTS = np_simple np_single_proc
PROJECT = $(foreach project, $(PROJECTS), $(BUILDDIR)/$(project))
OBJS = $(patsubst $(SRCDIRS)/%.cpp, $(BUILDDIR)/%.o,$(SRCS))
COBJS = $(filter-out $(foreach project, $(PROJECTS), $(BUILDDIR)/$(project).o) , $(OBJS))

##.SECONDEXPANSION:
all: $(PROJECT)

$(PROJECT): $(BUILDDIR) $(OBJS) # | $$(dir $$@)
	$(CC) $(CFLAG) $(COBJS) $@.o $(INCS) -o $@
	mv $@ ./
	
$(BUILDDIR)/%.o: $(SRCDIRS)/%.cpp 
	$(CC) $(CFLAG) -c $< $(INCS) -o $@

$(BUILDDIR):
	mkdir -p $(BUILDDIR)
.PRECIOUS : $(BUILDDIR)

.PHONY: clean
clean:
	rm -r $(BUILDDIR)/*	