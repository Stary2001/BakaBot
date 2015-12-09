CXXFLAGS=-std=c++11 -I$(COREPATH)/include/sock -I $(COREPATH)/include/plugin -g -fPIC
LDFLAGS=-L$(COREPATH)/lib -lsock -lplugin -ldl -fPIC -Wl,-E

# thanks, http://stackoverflow.com/questions/2483182/recursive-wildcards-in-gnu-make
rwildcard=$(foreach d,$(wildcard $1*),$(call rwildcard,$d/,$2) $(filter $(subst *,%,$2),$d))
SRC=$(filter-out src/plugins%, $(call rwildcard, src, *.cpp))
OBJ=$(patsubst src/%.cpp, obj/%.o, $(SRC))

obj/%.o: src/%.cpp
	$(CXX) -c $< -o $@ $(CXXFLAGS)

BakaBot: dirs $(OBJ)
	$(CXX) $(OBJ) $(LDFLAGS) -o $@

all: BakaBot

dirs:
	if [ ! -d obj ]; then\
		mkdir obj; \
	fi

clean:
	rm obj/* obj/*/*.o

-include Makefile.deps
Makefile.deps:
	$(CXX) $(CXXFLAGS) -MM $(SRC) > Makefile.deps


