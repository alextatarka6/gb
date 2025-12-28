CXX := clang++
CXXFLAGS := -Wall -Wextra -g

ROMINFO := rominfo

UTIL_SRCS := $(wildcard src/util/*.cc)

ROMINFO_SRCS := \
	platforms/test/main.cc \
	src/cartridge/cartridge_info.cc \
	$(UTIL_SRCS)

ROMINFO_OBJS := $(ROMINFO_SRCS:.cc=.o)

all: $(ROMINFO)

$(ROMINFO) : $(ROMINFO_OBJS)
	$(CXX) $(ROMINFO_OBJS) -o $@

%.o: %.cc
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(ROMINFO) $(ROMINFO_OBJS)