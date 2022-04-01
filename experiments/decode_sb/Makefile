
CXX = g++
RM = rm
TEST = test
CXXFLAGS = -Wall --std=c++17 -fPIC -Wno-unused-variable -Wno-sequence-point
DFLAGS = #-ggdb -g3 -DDEBUG
LIBS = asound torch
LDFLAGS = -shared

SRCS = \
	c_circular.cpp \
  c_drain.cpp \
	c_bind_test.cpp \
	alsa_rec.c

HDRS = \
	alsa_rec.h \
	c_circular.h

TARGET_LIB = libcbindtest.so

.PHONY: all clean

all: ${TARGET_LIB}

$(TARGET_LIB): $(SRCS)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $(DFLAGS) $(addprefix -l,$(LIBS)) -o $@ $^

clean:
	-$(TEST) $(TARGET_LIB) && $(RM) $(TARGET_LIB)
