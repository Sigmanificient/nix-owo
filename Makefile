.POSIX:

BUILD_DIR := .build
OUT := nix-hash

CXX = g++

CXXFLAGS += -fdiagnostics-color=always
CXXFLAGS += -D_GLIBCXX_ASSERTIONS=1
CXXFLAGS += -D_FILE_OFFSET_BITS=64
CXXFLAGS += -Wall
CXXFLAGS += -Winvalid-pch
CXXFLAGS += -std=c++23 -O0 -g
CXXFLAGS += -Wdeprecated-copy
CXXFLAGS += -Werror=suggest-override
CXXFLAGS += -Werror=switch
CXXFLAGS += -Werror=switch-enum
CXXFLAGS += -Werror=undef
CXXFLAGS += -Werror=unused-result
CXXFLAGS += -Werror=sign-compare
CXXFLAGS += -Wignored-qualifiers
CXXFLAGS += -Wimplicit-fallthrough
CXXFLAGS += -Wno-deprecated-declarations
CXXFLAGS += -fPIC
CXXFLAGS += -DBOOST_CONTEXT_DYN_LINK=1
CXXFLAGS += -DBOOST_COROUTINES_DYN_LINK=1
CXXFLAGS += -DBOOST_IOSTREAMS_DYN_LINK=1 
CXXFLAGS += -DBOOST_ALL_NO_LIB
CXXFLAGS += -pthread
CXXFLAGS += -DBLAKE3_DLL -DBLAKE3_USE_TBB

CXXFLAGS += -I libutil/include
CXXFLAGS += -I libutil/unix/include
CXXFLAGS += -I libutil/linux/include
CXXFLAGS += -I libutil/windows/include
CXXFLAGS += -I libutil/widecharwidth
CXXFLAGS += -I .

CXXFLAGS += -Wunused-function -Wunused-variable

LIBS += libbrotlicommon libbrotlidec libbrotlienc
LIBS += libarchive
LIBS += libblake3
LIBS += libcpuid
LIBS += libsodium
LIBS += nlohmann_json
LIBS += openssl

LDLIBS += $(shell pkg-config --libs $(LIBS))
LDLIBS += -L $(shell pkg-config --variable=libdir boost)
LDLIBS += -lboost_context -lboost_container -lboost_coroutine -lboost_url -lboost_iostreams

VPATH += src
SRC := $(shell find libutil -maxdepth 1 -name "*.cc")
SRC += $(shell find libutil/linux -maxdepth 1 -name "*.cc")
SRC += $(shell find libutil/unix -maxdepth 1 -name "*.cc")

vpath %.cpp $(VPATH)

OBJ := $(SRC:%.cc=$(BUILD_DIR)/%.o)

.PHONY: all
all: $(OUT)

$(BUILD_DIR)/%.o: %.cc
	@ mkdir -p $(dir $@)
	$Q $(CXX) $(CXXFLAGS) -o $@ -c $<
	@ $(LOG_TIME) "CXX $(C_PURPLE) $(notdir $@) $(C_RESET)"

$(OUT): $(OBJ)
	@ mkdir -p $(dir $@)
	$Q $(CXX) -o $@ main.cc $(OBJ) $(CXXFLAGS) $(LDLIBS) $(LDFLAGS)
	@ $(LOG_TIME) "LD $(C_GREEN) $@ $(C_RESET)"

.PHONY: clean
clean:
	$(RM) $(OBJ)
	@ $(LOG_TIME) $@

.PHONY: fclean
fclean: clean
	$(RM) -r $(BUILD_DIR) $(OUT)
	@ $(LOG_TIME) $@

.PHONY: re
.NOTPARALLEL: re
re: fclean all

ifneq ($(shell command -v tput),)
  ifneq ($(shell tput colors),0)

C_RESET := \033[00m
C_BOLD := \e[1m
C_RED := \e[31m
C_GREEN := \e[32m
C_YELLOW := \e[33m
C_BLUE := \e[34m
C_PURPLE := \e[35m
C_CYAN := \e[36m

C_BEGIN := \033[A

  endif
endif

NOW = $(shell date +%s%3N)

STIME := $(shell date +%s%3N)
export STIME

define TIME_MS
$$( expr \( $$(date +%s%3N) - $(STIME) \))
endef

BOXIFY = "[$(C_BLUE)$(1)$(C_RESET)] $(2)"

ifneq ($(shell command -v printf),)
  LOG_TIME = printf $(call BOXIFY, %6s , %b\n) "$(call TIME_MS)"
else
  LOG_TIME = echo -e $(call BOXIFY, $(call TIME_MS) ,)
endif
