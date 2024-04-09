TARGET = blue


INC_DIRS = metal-cpp 

# header library include flsgs
INC_FLAGS 	=$(addprefix -I,$(INC_DIRS)) -DNS_PRIVATE_IMPLEMENTATION
CFLAGS =  -framework Metal -framework Foundation -framework MetalKit
SRCS := test.cpp
OBJS := $(addsuffix .o,$(basename $(SRCS)))			# replace .c with .o
# OBJS := $(addprefix $(BUILD_DIR)/,$(OBJS))			# add prefix


${TARGET}: ${OBJS}
	clang++ -o $@ ${OBJS} ${CFLAGS} -g3

%.o: %.cpp
	clang++ -o $@ -c $< ${INC_FLAGS} -g3

clean:
	rm ${OBJS} ${TARGET}
