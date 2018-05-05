####################################################
# Generic makefile
# for compiling and linking C++ projects on Linux 
####################################################
# Customising
#
# Adjust the following if necessary; TARGET is the target
# library's filename, and LIBS is a list of libraries to link in
# (e.g. alleg, stdcx, iostr, etc). You can override these on make's
# command line of course, if you prefer to do it that way.

TARGET   := downloader 
LIBDIR   := 
LIBS     := pthread crypto
INCLUDES := ./
SRCDIR   :=

# Now alter any implicit rules' variables if you like, e.g.:

CC       := g++
CFLAGS   := -O3 -fPIC #-Wall -O3 -fPIC -g
CPPFLAGS := $(CFLAGS) $(addprefix -I,$(INCLUDES))
LDFLAGS  := 

# The next bit checks to see whether rm is in your bin
# directory; if not it uses del instead, but this can cause (harmless)
# `File not found' error messages. If you are not using DOS at all,
# set the variable to something which will unquestioningly remove
# files.

RM-F := rm -f


# You shouldn't need to change anything below this point.

SRCS := $(wildcard *.cpp) $(wildcard $(addsuffix /*.cpp, $(SRCDIR)))
OBJS := $(patsubst %.cpp,%.o,$(SRCS))
DEPS := $(patsubst %.o,%.d,$(OBJS))
MISSING_DEPS := $(filter-out $(wildcard $(DEPS)),$(DEPS))
MISSING_DEPS_SOURCES := $(wildcard $(patsubst %.d,%.cpp,$(MISSING_DEPS)))

# rules
%.o : %.cpp
	$(CC) $(CPPFLAGS) -c $< -o $@
	

.PHONY : all deps objs clean veryclean rebuild info

all: $(TARGET)

deps: $(DEPS)

objs: $(OBJS)

clean:
	@$(RM-F) *.o
	@$(RM-F) *.d
	
veryclean: clean
	@$(RM-F) $(TARGET)

rebuild: veryclean all
ifneq ($(MISSING_DEPS),)
$(MISSING_DEPS) :
	@$(RM-F) $(patsubst %.d,%.o,$@)
endif

-include $(DEPS)
$(TARGET) : $(OBJS)
	$(CC) $(CPPFLAGS) $(LDFLAGS) -o $(TARGET) $(OBJS) $(addprefix -L,$(LIBDIR)) $(addprefix -l,$(LIBS))

info:
	@echo $(SRCS)
	@echo $(OBJS)
	@echo $(DEPS)
	@echo $(MISSING_DEPS)
	@echo $(MISSING_DEPS_SOURCES)
	
