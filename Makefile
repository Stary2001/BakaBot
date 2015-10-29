CXXFLAGS=-std=c++11 -Ilibsock/include -g
LDFLAGS=-Llibsock -lsock

# thanks, http://stackoverflow.com/questions/2483182/recursive-wildcards-in-gnu-make
rwildcard=$(foreach d,$(wildcard $1*),$(call rwildcard,$d/,$2) $(filter $(subst *,%,$2),$d))
SRC=$(call rwildcard, src, *.cpp)
OBJ=$(patsubst src/%.cpp, obj/%.o, $(SRC))

obj/%.o: src/%.cpp
	$(CXX) -c $< -o $@ $(CXXFLAGS)

BakaBot: dirs libsock/libsock.a $(OBJ)
	$(CXX) $(OBJ) $(LDFLAGS) -o $@

all: BakaBot

dirs:
	if [ ! -d obj ]; then\
		mkdir obj; \
	fi

clean:
	rm obj/**

libsock/libsock.a:
	cd libsock; \
	make
