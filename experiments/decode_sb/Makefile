CXX = g++
RM = rm
TEST = test
CXXFLAGS = -Wall --std=c++17 -fPIC -Wno-unused-variable -Wno-sequence-point
DFLAGS += #-ggdb -g3 -DDEBUG
LIBS =
#LIBS += asound
LIBS += torch
#LIBS += pipewire-0.3
LIBS += pulse pulse-simple
LDFLAGS = -shared

LIB_DIRS +=
LIB_DIRS += /usr/local/lib/python3.10/dist-packages/torch/lib

INC_DIRS +=
INC_DIRS += /usr/local/lib/python3.10/dist-packages/torch/include
#INC_DIRS +=.. /usr/include/spa-0.2 /usr/include/pipewire-0.3

SRCS = \
	c_drain.cpp \
	c_py_binding.cpp \
	pulse/pulse_rec.c
#	c_circular.cpp \
#	alsa_rec.c \
#	pipewire/pw_wrapper.cpp \
#	pipewire/pw_capture.cpp \

HDRS = \
#	alsa_rec.h \
	c_circular.h \
	buf_type.h

TARGET_LIB = libcpybinding.so

.PHONY: all clean

all: ${TARGET_LIB}

$(TARGET_LIB): $(SRCS)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $(DFLAGS) $(addprefix -L,$(LIB_DIRS)) $(addprefix -l,$(LIBS)) $(addprefix -I,$(INC_DIRS)) -o $@ $^

clean:
	-$(TEST) $(TARGET_LIB) && $(RM) $(TARGET_LIB)
