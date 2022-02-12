PROJ=tiva_daq
PART=TM4C123GH6PM
TIVAWARE=${HOME}/ti/tivaware

include ${TIVAWARE}/makedefs

# Where to find source files that do not live in this directory.
VPATH=src
VPATH+=${TIVAWARE}/utils

# Where to find header files that do not live in the source directory.
IPATH=include
IPATH+=${TIVAWARE}

all: ${COMPILER} ${COMPILER}/${PROJ}.axf

clean:
	@rm -rf ${COMPILER} ${wildcard *~}

# The rule to create the target directory.
${COMPILER}:
	@mkdir -p ${COMPILER}

${COMPILER}/${PROJ}.axf: ${COMPILER}/startup_${COMPILER}.o
${COMPILER}/${PROJ}.axf: ${COMPILER}/uartstdio.o
${COMPILER}/${PROJ}.axf: ${COMPILER}/ustdlib.o
${COMPILER}/${PROJ}.axf: ${COMPILER}/main.o
${COMPILER}/${PROJ}.axf: ${TIVAWARE}/usblib/${COMPILER}/libusb.a
${COMPILER}/${PROJ}.axf: ${TIVAWARE}/driverlib/${COMPILER}/libdriver.a
${COMPILER}/${PROJ}.axf: tiva_flash.ld

SCATTERgcc_${PROJ}=tiva_flash.ld
ENTRY_${PROJ}=ResetISR
CFLAGSgcc=-DTARGET_IS_BLIZZARD_RA1 -DUART_BUFFERED -DDEBUG

# Include the automatically generated dependency files.
#ifneq (${MAKECMDGOALS},clean)
#-include ${wildcard ${COMPILER}/*.d} __dummy__
#endif

flash: ${COMPILER}/${PROJ}.bin
	lm4flash $<

tags:
	ctags -R src/
	ctags -a ${TIVAWARE}/driverlib/*.{c,h}
	ctags -a ${TIVAWARE}/usblib/*.{c,h}
	ctags -a ${TIVAWARE}/utils/*.{c,h}

.PHONY: flash tags
