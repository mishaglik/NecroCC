LIB_DIR = lib/
BIN_DIR = bin/
SRC_DIR = src/

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

SOURCES_LangTree = Tree.cpp

SOURCES_FrontEnd = 

SOURCES_FrontEnd/Parser = Parser.cpp

SUBDIRS = ${shell find $(SRC_DIR) -type d -printf '%P '}

SOURCES =  $(foreach dir, $(SUBDIRS), $(addprefix $(dir)/, $(SOURCES_$(dir))))

EXECUTABLE  = main.cpp 

SRC = $(SOURCES) $(EXECUTABLE)

OBJ = $(SRC:.cpp=.o)

DEP = $(SRC:.cpp=.d)

TARGETS = ncc

init:
	mkdir -p $(addprefix $(BIN_DIR), $(SUBDIRS))

all: $(TARGETS)
	# ./increaseVersion.sh bld_version

ncc: $(addprefix $(BIN_DIR), $(OBJ))
	g++ $(CXXFLAGS) $^ $(LXXFLAGS) -o $@

$(BIN_DIR)%.o : $(SRC_DIR)%.cpp
	g++ -c $(CXXFLAGS) $(LXXFLAGS) -o $@ $<

.PHONY: deps
deps: $(addprefix $(BIN_DIR), $(DEP))
	echo "Deps builded"
	
$(BIN_DIR)%.d : $(SRC_DIR)%.cpp
	g++ -MM -MT $(@:.d=.o) $< -o $@ -I$(LIB_DIR)

-include $(addprefix $(BIN_DIR), $(DEP))

.PHONY: clean
clean:
	rm -f $(addprefix $(BIN_DIR), $(OBJ) $(DEP))
	rm $(TARGETS)
