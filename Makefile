CC = g++
CFLAG = -std=c++11
INCS = -I./include
BUILDDIR = build
SRCDIRS = src
TESTDIR = test
SRCS = $(shell find $(SRCDIRS) -name '*.cpp')
OBJS = $(patsubst $(SRCDIRS)/%.cpp, $(BUILDDIR)/%.o,$(SRCS))
PROJECT = npshell

TEST_SRCS = $(shell find $(TESTDIR) -name '*.cpp')
TEST_OBJS = $(patsubst $(TEST_SRCS)/%.cpp,$(BUILDDIR)/test/%.o,$(TEST_SRCS))
TEST_TARGET = $(filter-out %$(PROJECT).o,$(OBJS))
TEST_PROJECT = UnitTest

##.SECONDEXPANSION:
$(BUILDDIR)/$(PROJECT): $(OBJS) $(BUILDDIR) # | $$(dir $$@)
	$(CC) $(CFLAG) $(OBJS) $(INCS) -o $@
	$(BUILDDIR)/$(PROJECT)

$(BUILDDIR)/%.o: $(SRCDIRS)/%.cpp 
	$(CC) $(CFLAG) -c $< $(INCS) -o $@

$(BUILDDIR):
	mkdir -p $(BUILDDIR)
.PRECIOUS : $(BUILDDIR)

.PHONY: test
test: $(BUILDDIR)/test/$(TEST_PROJECT) 
	$(BUILDDIR)/test/$(TEST_PROJECT)
	
$(BUILDDIR)/test/$(TEST_PROJECT): $(TEST_OBJS) $(BUILDDIR) $(BUILDDIR)/test
	$(CC) $(CFLAG) $(TEST_OBJS) $(TEST_TARGET) $(INCS) -o $@

$(BUILDDIR)/test/%.o: $(TESTDIR)/%.cpp 
	$(CC) $(CFLAG) -c $< $(INCS) -o $@

$(BUILDDIR)/test:
	mkdir -p $(BUILDDIR)/test
.PRECIOUS : $(BUILDDIR)/test


.PHONY: clean
clean:
	rm -r $(BUILDDIR)/*	
