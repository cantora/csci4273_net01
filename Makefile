.SECONDARY:

DEFINES 		= 
INCLUDES 		+= -iquote"./src" 
DBG			= -g
#OPTIMIZE		= -Os
C_FLAGS 		= -Wall $(OPTIMIZE) $(DBG) -w $(DEFINES) $(INCLUDES)
C_FLAGS			+= -DSESSION_SERVER_TIMEOUT
CXX_FLAGS		= $(C_FLAGS)
CXX_CMD			= g++ $(CXX_FLAGS)



BUILD 			= ./build

BOOST_TEST_FLAGS	= -L/opt/local/lib
#BOOST_TEST_LIB		= -lboost_test_exec_monitor
BOOST_TEST_INCLUDE	= -I/opt/local/include

OBJECTS 		:= $(patsubst %.cc, $(BUILD)/%.o, $(notdir $(wildcard ./src/*.cc) ) )

DEPENDS			:= $(OBJECTS:.o=.d) 
DEPSDIR			= $(BUILD)
DEP_FLAGS		= -MMD -MP -MF $(patsubst %.o, %.d, $@)

TESTS 			= $(notdir $(patsubst %.cc, %, $(wildcard ./test/*.cc) ) )
TEST_OUTPUTS	= $(foreach test, $(TESTS), $(BUILD)/test/$(test))

default: all

.PHONY: all
all: start_coord start_client

start_client: $(BUILD)/start_client.o $(OBJECTS)
	$(CXX_CMD) $(DEP_FLAGS) $+ -o $@

$(BUILD)/start_client.o: start_client.cc
	$(CXX_CMD) $(DEP_FLAGS) -c $< -o $@

start_coord: $(BUILD)/start_coord.o $(OBJECTS)
	$(CXX_CMD) $(DEP_FLAGS) $+ -o $@

$(BUILD)/start_coord.o: start_coord.cc
	$(CXX_CMD) $(DEP_FLAGS) -c $< -o $@

$(BUILD)/%.o: src/%.cc src/%.h
	$(CXX_CMD) $(DEP_FLAGS) -c $< -o $@

$(BUILD)/test/%.o: test/%.cc $(OBJECTS)
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
	rm -v start_client start_coord

-include $(DEPENDS)