CXX = g++
RM = rm
TEST = test
CXXFLAGS = -Wall --std=c++17 -fPIC -Wno-unused-variable -Wno-sequence-point
DFLAGS = -ggdb -g3 -DDEBUG
LIBS = asound torch pipewire-0.3
LDFLAGS = -shared

INC_DIRS=.. /usr/include/spa-0.2 /usr/include/pipewire-0.3

SRCS = \
	c_circular.cpp \
	c_drain.cpp \
	c_bind_test.cpp \
	alsa_rec.c \
	pipewire/pw_wrapper.cpp \
	pipewire/pw_capture.cpp

HDRS = \
	alsa_rec.h \
	c_circular.h \
	buf_type.h

TARGET_LIB = libcbindtest.so

.PHONY: all clean

all: ${TARGET_LIB}

$(TARGET_LIB): $(SRCS)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $(DFLAGS) $(addprefix -l,$(LIBS))  $(addprefix -I,$(INC_DIRS)) -o $@ $^

clean:
	-$(TEST) $(TARGET_LIB) && $(RM) $(TARGET_LIB)
