CXXFLAGS=-fPIC -std=c++11 -I$(COREPATH)/include/plugin -I$(COREPATH)/include/sock -I../ -shared
PLUGINS=$(patsubst ./%,%.so,$(shell find . -maxdepth 1 -mindepth 1 -type d))

all: $(PLUGINS)

%.so : %.plugin
	g++ $(CXXFLAGS) `cat $<`/*.cpp -o $@
