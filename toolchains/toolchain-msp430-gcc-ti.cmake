# To be able to use Force Compiler macros.
include(CMakeForceCompiler)

# Add the location of your "toolchains" folder to the module path.
list(APPEND CMAKE_MODULE_PATH "/home/benjamin/openchronos-ng-elf/toolchains")

# Name should be 'Generic' or something for which a
# Platform/<name>.cmake (or other derivatives thereof, see cmake docs)
# file exists. The cmake installation comes with a Platform folder with
# defined platforms, and we add our custom ones to the "Platform" folder
# within the "toolchain" folder.
set(CMAKE_SYSTEM_NAME msp430-gcc)

# Compiler and related toochain configuration
# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

SET(MSP430_TI_COMPILER_FOLDER $ENV{MSP430_TI})
SET(MSP430_TI_BIN_FOLDER      ${MSP430_TI_COMPILER_FOLDER}/bin)
SET(MSP430_TI_INCLUDE_FOLDER  ${MSP430_TI_COMPILER_FOLDER}/include)
SET(TOOLCHAIN_PREFIX		  msp430-elf)
SET(TOOLCHAIN_BIN_PATH		  ${MSP430_TI_BIN_FOLDER})

INCLUDE_DIRECTORIES(${MSP430_TI_INCLUDE_FOLDER} ${INCLUDE_DIRECTORIES})
LINK_DIRECTORIES(${MSP430_TI_INCLUDE_FOLDER}    ${LINK_DIRECTORIES})

# This can be skipped to directly set paths below, or augmented with hints
# and such. See cmake docs of FIND_PROGRAM for details.
FIND_PROGRAM(MSP430_CC		 ${TOOLCHAIN_PREFIX}-gcc     PATHS ${TOOLCHAIN_BIN_PATH})
FIND_PROGRAM(MSP430_CXX		 ${TOOLCHAIN_PREFIX}-g++     PATHS ${TOOLCHAIN_BIN_PATH})
FIND_PROGRAM(MSP430_AR		 ${TOOLCHAIN_PREFIX}-ar      PATHS ${TOOLCHAIN_BIN_PATH})
FIND_PROGRAM(MSP430_AS		 ${TOOLCHAIN_PREFIX}-as      PATHS ${TOOLCHAIN_BIN_PATH})
FIND_PROGRAM(MSP430_OBJDUMP	 ${TOOLCHAIN_PREFIX}-objdump PATHS ${TOOLCHAIN_BIN_PATH})
FIND_PROGRAM(MSP430_OBJCOPY	 ${TOOLCHAIN_PREFIX}-objcopy PATHS ${TOOLCHAIN_BIN_PATH})
FIND_PROGRAM(MSP430_SIZE	 ${TOOLCHAIN_PREFIX}-size    PATHS ${TOOLCHAIN_BIN_PATH})
FIND_PROGRAM(MSP430_NM		 ${TOOLCHAIN_PREFIX}-nm      PATHS ${TOOLCHAIN_BIN_PATH})
FIND_PROGRAM(MSP430_MSPDEBUG mspdebug)

# Since compiler need a -mmcu flag to do anything, checks need to be bypassed
CMAKE_FORCE_C_COMPILER(${MSP430_CC} 	GNU)
CMAKE_FORCE_CXX_COMPILER(${MSP430_CXX} 	GNU)

set(AS 		${MSP430_AS}      CACHE STRING "AS Binary")
set(AR 		${MSP430_AR}      CACHE STRING "AR Binary")
set(OBJCOPY ${MSP430_OBJCOPY} CACHE STRING "OBJCOPY Binary")
set(OBJDUMP ${MSP430_OBJDUMP} CACHE STRING "OBJDUMP Binary")
set(SIZE 	${MSP430_SIZE}    CACHE STRING "SIZE Binary")

IF(NOT CMAKE_BUILD_TYPE)
    SET(CMAKE_BUILD_TYPE RelWithDebInfo CACHE STRING
            "Choose the type of build, options are: None Debug Release RelWithDebInfo MinSizeRel."
            FORCE)
ENDIF(NOT CMAKE_BUILD_TYPE)

set(MSPGCC_OPT_LEVEL 	"0" CACHE STRING "MSPGCC OPT LEVEL")

set(MSPGCC_WARN_PROFILE "-Wall -Wshadow -Wpointer-arith -Wbad-function-cast -Wcast-align -Wsign-compare -Wstrict-prototypes -Wmissing-prototypes -Wmissing-declarations -Wunused"
        CACHE STRING "MSPGCC WARNINGS")

set(MSPGCC_OPTIONS 	"-fdata-sections -ffunction-sections"
        CACHE STRING "MSPGCC OPTIONS")

set(CMAKE_C_FLAGS 	"${MSPGCC_WARN_PROFILE} ${MSPGCC_OPTIONS} -O${MSPGCC_OPT_LEVEL}  -DGCC_MSP430" CACHE STRING "C Flags")

set(CMAKE_SHARED_LINKER_FLAGS 	"-Wl,--gc-sections -Wl,--print-gc-sections"
        CACHE STRING "Linker Flags")
set(CMAKE_EXE_LINKER_FLAGS 	"-Wl,--gc-sections"
        CACHE STRING "Linker Flags")

# Specify linker command. This is needed to use gcc as linker instead of ld
# This seems to be the preferred way for MSPGCC atleast, seemingly to avoid
# linking against stdlib.
set(CMAKE_CXX_LINK_EXECUTABLE
        "<CMAKE_C_COMPILER> ${CMAKE_EXE_LINKER_FLAGS} <LINK_FLAGS> <OBJECTS> -o <TARGET> <LINK_LIBRARIES>"
        CACHE STRING "C++ Executable Link Command")

set(CMAKE_C_LINK_EXECUTABLE ${CMAKE_CXX_LINK_EXECUTABLE}
        CACHE STRING "C Executable Link Command")

# Programmer and related toochain configuration
# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

set(PROGBIN 	${MSP430_MSPDEBUG} CACHE STRING "Programmer Application")
set(PROGRAMMER	tilib CACHE STRING "Programmer driver")