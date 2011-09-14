
DEFINES 		= 
INCLUDES 		+= -iquote"./src" 
#DBG			= -g
#OPTIMIZE		= -Os
C_FLAGS 		= -Wall $(OPTIMIZE) $(DBG) -w $(DEFINES) $(INCLUDES)
CXX_FLAGS		= $(C_FLAGS)
CXX_CMD			= g++ $(CXX_FLAGS)

MAIN_TARGET		:= net01

BUILD 			= ./build

BOOST_TEST_FLAGS	= -L/opt/local/lib
BOOST_TEST_LIB		= -lboost_test_exec_monitor
BOOST_TEST_INCLUDE	= -I/opt/local/include

OBJECTS 		:= $(patsubst %.cc, $(BUILD)/%.o, $(notdir $(wildcard ./src/*.cc) ) )

DEPENDS			:= $(OBJECTS:.o=.d) 
DEPSDIR			= $(BUILD)
DEP_FLAGS		= -MMD -MP -MF $(patsubst %.o, %.d, $@)

TESTS 			= $(notdir $(patsubst %.cc, %, $(wildcard ./test/*.cc) ) )
TEST_OUTPUTS	= $(foreach test, $(TESTS), $(BUILD)/test/$(test))

default: all

OUTPUT = $(BUILD)/$(MAIN_TARGET)

.PHONY: all
all: $(OUTPUT)


$(BUILD)/%.o: src/%.cc
	$(CXX_CMD) $(DEP_FLAGS) -c $< -o $@

$(BUILD)/test/%.o: test/%.cc 
	$(CXX_CMD) $(DEP_FLAGS) $(BOOST_TEST_FLAGS) $(BOOST_TEST_INCLUDE) -c $< -o $@

$(BUILD)/test/%: $(BUILD)/test/%.o $(OBJECTS)
	$(CXX_CMD) $(BOOST_TEST_FLAGS) $+ $(BOOST_TEST_LIB) -o $@

define test-template
$(1): $$(BUILD)/test/$(1) 
	$(BUILD)/test/$(1)
endef

.PHONY: $(TESTS) 
$(foreach test, $(TESTS), $(eval $(call test-template,$(test)) ) )

.PHONY: clean 
clean: 
	rm -vf $(shell find $(BUILD) -type f -not -name .gitignore )


-include $(DEPENDS)