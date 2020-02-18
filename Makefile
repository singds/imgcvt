# project root directory
P_DIR_PROJECT=${PWD}
# imgcvt build directory
P_DIR_BUILD=${P_DIR_PROJECT}/build
# imgcvt source directory
P_DIR_SRC=${P_DIR_PROJECT}/src

P_GCC_FLAGS= -g

.PHONY: compile
compile:
	echo ${P_DIR_PROJECT}
	if [ ! -d ${P_DIR_BUILD} ]; \
	then \
		mkdir ${P_DIR_BUILD}; \
	fi
	
	gcc ${P_DIR_SRC}/imgcvt.c ${P_DIR_SRC}/lodepng/lodepng.c ${P_GCC_FLAGS} -o ${P_DIR_BUILD}/imgcvt
	@echo ok ... build done

.PHONY: clean
clean:
	rm -r ${P_DIR_BUILD}

.PHONY: run
run:
	${P_DIR_BUILD}/imgcvt ${P_ARGS}