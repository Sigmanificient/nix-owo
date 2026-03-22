.POSIX:

BINDIR ?= $(PREFIX)/bin

BUILD_DIR := .build

OUT_NIX_HASH := nix-sri-hash
OUT_NIX_OWO := nix-owo

CXX = g++

CXXFLAGS += -std=c++23 -O2
CXXFLAGS += -Wall -Wextra
CXXFLAGS += -fPIC

CXXFLAGS += -I libutil/include
CXXFLAGS += -I libutil/unix/include
CXXFLAGS += -I .

CXXFLAGS += -Wunused-function -Wunused-variable

LIBS += nix-util

LDLIBS += $(shell pkg-config --libs $(LIBS))
LDLIBS += -L $(shell pkg-config --variable=libdir boost)
LDLIBS += -lboost_program_options

VPATH += src
vpath %.cpp $(VPATH)

OBJ = $(SRC:%.cpp=$(BUILD_DIR)/%.o)

.PHONY: all
all: $(OUT_NIX_HASH) $(OUT_NIX_OWO)

$(BUILD_DIR)/%.o: src/%.cpp
	@ mkdir -p $(dir $@)
	$Q $(CXX) $(CXXFLAGS) -o $@ -c $<
	@ $(LOG_TIME) "CXX $(C_PURPLE) $(notdir $@) $(C_RESET)"

$(OUT_NIX_HASH): $(OBJ) $(BUILD_DIR)/nix-sri-hash.o
	@ mkdir -p $(dir $@)
	$Q $(CXX) -o $@ $^ $(CXXFLAGS) $(LDLIBS) $(LDFLAGS)
	@ $(LOG_TIME) "LD $(C_GREEN) $@ $(C_RESET)"

$(OUT_NIX_OWO): $(OBJ) $(BUILD_DIR)/nix-owo.o
	@ mkdir -p $(dir $@)
	$Q $(CXX) -o $@ $^ $(CXXFLAGS) $(LDLIBS) $(LDFLAGS)
	@ $(LOG_TIME) "LD $(C_GREEN) $@ $(C_RESET)"

.PHONY: clean
clean:
	$(RM) $(OBJ)
	@ $(LOG_TIME) $@

.PHONY: fclean
fclean: clean
	$(RM) -r $(BUILD_DIR) $(OUT_NIX_HASH) $(OUT_NIX_OWO)
	@ $(LOG_TIME) $@

.PHONY: re
.NOTPARALLEL: re
re: fclean all

PREFIX ?= /usr/bin

.PHONY: check
check: README := $(shell mktemp -d)/README.md
check:
	./test-nix-hash.sh ./nix-sri-hash
	echo "test" > $(README)
	echo $$(./nix-owo $(dir $(README)) | grep -P '<!-- \d+ -->') >> $(README)
	nix-hash --sri $(dir $(README)) --type sha256 | grep 0w0

.PHONY: install
install:
	install -Dm0755 nix-owo -t $(BINDIR)

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
