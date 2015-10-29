CXXFLAGS=-std=c++11 -fPIC -Iinclude

# thanks, http://stackoverflow.com/questions/2483182/recursive-wildcards-in-gnu-make
rwildcard=$(foreach d,$(wildcard $1*),$(call rwildcard,$d/,$2) $(filter $(subst *,%,$2),$d))
SRC=$(call rwildcard, src, *.cpp)
OBJ=$(patsubst src/%.cpp, obj/%.o, $(SRC))

obj/%.o: src/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

libsock.a: dirs $(OBJ)
	$(AR) rcs $@ $(OBJ)

all: libsock.so

dirs:
	if [ ! -d obj ]; then\
		mkdir obj; \
	fi

clean:
	rm obj/**
	rm libsock.a
