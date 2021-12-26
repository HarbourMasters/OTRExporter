# Only used for standalone compilation, usually inherits these from the main makefile

CXX := g++
AR	:= ar
FORMAT := clang-format-11

ASAN ?= 0
DEBUG ?= 1
OPTFLAGS ?= -O0

CXXFLAGS ?= -Wall -Wextra -std=c++17

ifneq ($(DEBUG),0)
	CXXFLAGS += -g
endif

ifneq ($(ASAN),0)
	CXXFLAGS += -fsanitize=address
endif

SRC_DIRS  := $(shell find -type d -not -path "*build*")
CXX_FILES := $(foreach dir,$(SRC_DIRS),$(wildcard $(dir)/*.cpp))
H_FILES   := $(foreach dir,$(SRC_DIRS),$(wildcard $(dir)/*.h))

O_FILES   := $(CXX_FILES:%.cpp=build/%.o)
LIB       := ExporterTest.a

INCDIRS := $(addprefix -I, \
	../../ZAPD/ZAPD \
	../../ZAPD/lib/tinyxml2 \
	../../ZAPD/lib/libgfxd \
	../../ZAPD/ZAPDUtils \
	../../OtrLib/otrlib \
	../../OtrLib/otrlib/lib/spdlog/include \
	../../OtrLib/otrlib/Lib/Fast3D/U64 \
)

# create build directories
$(shell mkdir -p $(SRC_DIRS:%=build/%))

all: $(LIB)

clean:
	rm -rf build $(LIB)

format:
	$(FORMAT) -i $(CXX_FILES) $(H_FILES)

.PHONY: all clean format

build/%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(OPTFLAGS) $(INCDIRS) -c $< -o $@

$(LIB): $(O_FILES)
	$(AR) rcs $@ $^
