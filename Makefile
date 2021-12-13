LIB_DIR = lib/
BIN_DIR = bin/
SRC_DIR = src/

NCC_DIR = ncc/

FRONT_DIR = FrontEnd/
MIDDLE_DIR = MiddleEnd
BACK_DIR  = BackEnd/

MAJOR_VERSION = 1
MINOR_VERSION = 0
# BUILD_VERSION = `cat bld_version`
# TODO: Auto increment version

LIBRARIES = MGK Stack File

CXXFLAGS = `cat $(LIB_DIR)Cflags` -I$(LIB_DIR)
SANFLAGS = `cat $(LIB_DIR)SanitizeFlags`
LXXFLAGS = -L$(LIB_DIR) $(addprefix -l, $(LIBRARIES))

CXXFLAGS += -DMAJOR_VERSION=$(MAJOR_VERSION)
CXXFLAGS += -DMINOR_VERSION=$(MINOR_VERSION)
# CXXFLAGS += -DBUILD_VERSION=$(BUILD_VERSION)

CXXFLAGS += $(SANFLAGS)

SOURCES_COMMON = $(patsubst $(SRC_DIR)%, %, $(shell find $(SRC_DIR)LangTree -name *.cpp))

FRONTENDS = cht

BACKENDS = asm

SRC = $(shell find $(SRC_DIR) -name *.cpp -printf "%P ")

SUBDIRS = ${shell find $(SRC_DIR) -type d -printf '%P '}

OBJ_COMMON = $(addprefix $(BIN_DIR), $(SOURCES_COMMON:.cpp=.o))

DEP = $(SRC:.cpp=.d)

TARGETS = $(addprefix $(BACK_DIR), $(BACKENDS)) $(addprefix $(FRONT_DIR), $(FRONTENDS))  $(MIDDLE_DIR)

init:
	mkdir -p $(addprefix $(BIN_DIR), $(SUBDIRS))

all: $(addprefix $(NCC_DIR), $(TARGETS))
	# ./increaseVersion.sh bld_version

.SECONDEXPANSION:
$(addprefix $(NCC_DIR), $(TARGETS)): $(NCC_DIR)% : $$(subst $(SRC_DIR), $(BIN_DIR), $$(addsuffix .o, $$(basename $$(shell find $(SRC_DIR)% -name *.cpp)))) $(OBJ_COMMON)
	g++ $(CXXFLAGS) $^ $(LXXFLAGS) -o $@

$(BIN_DIR)%.o : $(SRC_DIR)%.cpp
	g++ -c $(CXXFLAGS) $(LXXFLAGS) -o $@ $<

.PHONY: deps
deps: $(addprefix $(BIN_DIR), $(DEP))
	@echo $^
	@echo Deps builded
	
$(addprefix $(BIN_DIR), $(DEP)) :$(BIN_DIR)%.d : $(SRC_DIR)%.cpp
	g++ -MM -MT $(@:.d=.o) $< -o $@ -I$(LIB_DIR)


-include $(addprefix $(BIN_DIR), $(DEP))

.PHONY: clean
clean:
	rm -f $(addprefix $(BIN_DIR), $(OBJ) $(DEP))
	rm $(TARGETS)
