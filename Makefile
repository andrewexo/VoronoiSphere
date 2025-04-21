UNAME := $(shell uname)

DEBUG_OR_OPT := -O3 # -g

ifeq ($(UNAME), Linux)
COMPILER = g++
LINKER = g++
FLAGS = -Wall -std=c++17 $(DEBUG_OR_OPT) -msse4.2 -Igoogletest/include/ -Iboost_1_62_0/ -Wno-register -Wno-class-memaccess -Wno-unused-variable
LINKS = -Lboost_1_62_0/stage/lib -lboost_timer -lboost_chrono -lboost_system -lpthread
endif

ifeq ($(UNAME), Darwin)
COMPILER = g++
LINKER = g++
FLAGS = -Wall -std=c++17 $(DEBUG_OR_OPT) -msse4.2 -Igoogletest/include/ -Iboost_1_76_0/ -Wno-class-memaccess -Wno-unused-variable -Wno-shift-op-parentheses
LINKS = -Lusr/local/Cellar/boost/1.76.0/lib -lboost_timer-mt -lboost_chrono-mt -lboost_system-mt -lpthread
endif


TEST_LINKS = -lgtest -lpthread


VORONOI_GENERATOR_OBJS = voronoi_event.o voronoi_cell.o voronoi_generator.o beachline.o priqueue.o globals.o spin_lock.o task_graph.o voronoi_site.o mp_sample_generator.o voronoi_sweeper.o
TEST_OBJS = tests.o


vg: vg_main.o $(VORONOI_GENERATOR_OBJS)
	$(LINKER) vg_main.o $(VORONOI_GENERATOR_OBJS) $(LINKS) -o vg

tests: $(TEST_OBJS)
	$(LINKER) $(TEST_OBJS) $(VORONOI_GENERATOR_OBJS) $(TEST_LINKS) $(LINKS) -o tests

all: vg tests

vg_main.o: vg_main.cpp
	$(COMPILER) vg_main.cpp $(FLAGS) -c

voronoi_event.o: src/voronoi_event.h src/voronoi_event.cpp
	$(COMPILER) src/voronoi_event.cpp $(FLAGS) -c

voronoi_sweeper.o: src/voronoi_sweeper.cpp src/voronoi_event_compare.h
	$(COMPILER) src/voronoi_sweeper.cpp $(FLAGS) -c

beachline.o: src/beachline.h src/beachline.cpp
	$(COMPILER) src/beachline.cpp $(FLAGS) -c

priqueue.o: src/priqueue.h src/priqueue.cpp src/voronoi_event_compare.h
	$(COMPILER) src/priqueue.cpp $(FLAGS) -c

buckets.o: src/buckets.h src/buckets.cpp
	$(COMPILER) src/buckets.cpp $(FLAGS) -c

voronoi_cell.o: src/voronoi_cell.h src/voronoi_cell.cpp
	$(COMPILER) src/voronoi_cell.cpp $(FLAGS) -c

voronoi_site.o: src/voronoi_site.h src/voronoi_site.cpp
	$(COMPILER) src/voronoi_site.cpp $(FLAGS) -c

voronoi_generator.o: src/voronoi_generator.h src/voronoi_generator.cpp
	$(COMPILER) src/voronoi_generator.cpp $(FLAGS) -c

task_graph.o: src/task_graph.h src/task_graph.cpp
	$(COMPILER) src/task_graph.cpp $(FLAGS) -c

spin_lock.o: src/spin_lock.h src/spin_lock.cpp
	$(COMPILER) src/spin_lock.cpp $(FLAGS) -c

globals.o: src/globals.h src/globals.cpp
	$(COMPILER) src/globals.cpp $(FLAGS) -c

mp_sample_generator.o: src/mp_sample_generator.h src/mp_sample_generator.cpp
	$(COMPILER) src/mp_sample_generator.cpp $(FLAGS) -c

tests.o: test/tests.cpp test/voronoi_tests.cpp test/priqueue_tests.cpp src/priqueue.cpp
	$(COMPILER) test/tests.cpp $(FLAGS) -c


.PHONY: clean spaces

ifeq ($(UNAME), Linux)
clean:
	rm *.o vg tests 2>/dev/null
endif

ifeq ($(UNAME), Darwin)
clean:
	rm *.o vg tests
endif

spaces:
	find . \( -name '*.h' -o -name '*.cpp' \) ! -type d -exec bash -c 'expand -i -t 4 "$$0" > /tmp/e && mv /tmp/e "$$0"' {} \;
