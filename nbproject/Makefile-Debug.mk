#
# Generated Makefile - do not edit!
#
# Edit the Makefile in the project folder instead (../Makefile). Each target
# has a -pre and a -post target defined where you can add customized code.
#
# This makefile implements configuration specific macros and targets.


# Environment
MKDIR=mkdir
CP=cp
GREP=grep
NM=nm
CCADMIN=CCadmin
RANLIB=ranlib
CC=gcc
CCC=g++
CXX=g++
FC=gfortran
AS=as

# Macros
CND_PLATFORM=MinGW-Windows
CND_DLIB_EXT=dll
CND_CONF=Debug
CND_DISTDIR=dist
CND_BUILDDIR=build

# Include project Makefile
include Makefile

# Object Directory
OBJECTDIR=${CND_BUILDDIR}/${CND_CONF}/${CND_PLATFORM}

# Object Files
OBJECTFILES= \
	${OBJECTDIR}/compiler.o \
	${OBJECTDIR}/func.o \
	${OBJECTDIR}/lib.o \
	${OBJECTDIR}/main.o \
	${OBJECTDIR}/map.o \
	${OBJECTDIR}/object.o \
	${OBJECTDIR}/primitives.o \
	${OBJECTDIR}/string.o \
	${OBJECTDIR}/symbol.o \
	${OBJECTDIR}/tokenizer.o \
	${OBJECTDIR}/value.o \
	${OBJECTDIR}/vm.o


# C Compiler Flags
CFLAGS=

# CC Compiler Flags
CCFLAGS=
CXXFLAGS=

# Fortran Compiler Flags
FFLAGS=

# Assembler Flags
ASFLAGS=

# Link Libraries and Options
LDLIBSOPTIONS=

# Build Targets
.build-conf: ${BUILD_SUBPROJECTS}
	"${MAKE}"  -f nbproject/Makefile-${CND_CONF}.mk ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/picovm.exe

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/picovm.exe: ${OBJECTFILES}
	${MKDIR} -p ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}
	${LINK.c} -o ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/picovm ${OBJECTFILES} ${LDLIBSOPTIONS}

${OBJECTDIR}/compiler.o: compiler.c
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.c) -g -Wall -std=c99 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/compiler.o compiler.c

${OBJECTDIR}/func.o: func.c
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.c) -g -Wall -std=c99 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/func.o func.c

${OBJECTDIR}/lib.o: lib.c
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.c) -g -Wall -std=c99 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/lib.o lib.c

${OBJECTDIR}/main.o: main.c
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.c) -g -Wall -std=c99 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/main.o main.c

${OBJECTDIR}/map.o: map.c
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.c) -g -Wall -std=c99 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/map.o map.c

${OBJECTDIR}/object.o: object.c
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.c) -g -Wall -std=c99 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/object.o object.c

${OBJECTDIR}/primitives.o: primitives.c
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.c) -g -Wall -std=c99 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/primitives.o primitives.c

${OBJECTDIR}/string.o: string.c
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.c) -g -Wall -std=c99 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/string.o string.c

${OBJECTDIR}/symbol.o: symbol.c
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.c) -g -Wall -std=c99 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/symbol.o symbol.c

${OBJECTDIR}/tokenizer.o: tokenizer.c
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.c) -g -Wall -std=c99 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/tokenizer.o tokenizer.c

${OBJECTDIR}/value.o: value.c
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.c) -g -Wall -std=c99 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/value.o value.c

${OBJECTDIR}/vm.o: vm.c
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.c) -g -Wall -std=c99 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/vm.o vm.c

# Subprojects
.build-subprojects:

# Clean Targets
.clean-conf: ${CLEAN_SUBPROJECTS}
	${RM} -r ${CND_BUILDDIR}/${CND_CONF}

# Subprojects
.clean-subprojects:

# Enable dependency checking
.dep.inc: .depcheck-impl

include .dep.inc
