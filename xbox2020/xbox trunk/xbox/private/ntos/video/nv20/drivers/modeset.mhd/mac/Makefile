#
#
#
PLATFORM=macos9
FLAVOR=macos
CPU=ppc
OBJDIR={FLAVOR}-{CPU}

LIBNAME=NVmodeset

MAIN_DIR         = ::::
DRIVERS_DIR      = :::
MODESET_DIR      = ::
MODESET_MAC_DIR  = :

MODESET_CODE_DIR = "{MODESET_DIR}CODE"
MODESET_INC_DIR  = "{MODESET_DIR}INCLUDE"
MODESET_BIN_DIR  = "{MODESET_DIR}BIN"
MODESET_LIB_DIR  = "{MODESET_DIR}LIB"
TARGET_DIR       = {MODESET_LIB_DIR}




NV_SDK_DIR       = "{MAIN_DIR}sdk:nvidia"
NV_SDK_INC_DIR   = "{NV_SDK_DIR}:inc"
#MAC_BRANCH       = :::::mac
#MAC_MAIN_DIR     = :::::mac:main
#NV_SDK_INC_DIR   = "{MAC_MAIN_DIR}:sdk:nvidia:inc"

NV_MODESET_LIB={TARGET_DIR}:{LIBNAME}
NV_MODESET_LIB_STATIC={TARGET_DIR}:{LIBNAME}.lib

DEFINES         = -DMAC

#INCLUDES=-inclpaths ignoresystem �
INCLUDES=                        �
     -I:                         �
     -I{NV_SDK_INC_DIR}          �
     -I{MODESET_INC_DIR}         �
     -I{MODESET_CODE_DIR}

CPPFLAGS={DEFINES} {INCLUDES}
#
# list out the warnings since '-w all' produces bogus complaints about prototypes
#
C_WARNINGS=-w illpragmas,empty,possible,extracomma,pedantic,notinlined,largeargs,unusedvar,nounusedarg
CFLAGS=-maxerrors 10 -g -tb on -opt off {C_WARNINGS} -cwd explicit �
       -ansi off -strings reuse,pool,readonly �
       {EXTRA_CFLAGS} �
       {CPPFLAGS}
	   
# 
# compilation tools
#
CC = mwcppc
LD = mwlinkppc  

.c.o � .c
    {CC} {Depdir}{Default}.c -o {Targdir}{Default}.c.o {CFLAGS}

OBJDIRS =                             �
    {MODESET_CODE_DIR}:{OBJDIR}       �
    {MODESET_MAC_DIR}{OBJDIR}

MODESET_OBJS =                               �
    {MODESET_CODE_DIR}:{OBJDIR}:DMT.c.o      �
    {MODESET_CODE_DIR}:{OBJDIR}:EDID.c.o     �
    {MODESET_CODE_DIR}:{OBJDIR}:GTF.c.o      �
    {MODESET_CODE_DIR}:{OBJDIR}:RESTIME.c.o  �
    {MODESET_CODE_DIR}:{OBJDIR}:digmode.c.o  �
    {MODESET_CODE_DIR}:{OBJDIR}:tvmode.c.o   �
    {MODESET_CODE_DIR}:{OBJDIR}:MODESET.c.o  �
    {MODESET_CODE_DIR}:{OBJDIR}:UTILS.c.o    �
    {MODESET_CODE_DIR}:{OBJDIR}:gtfmathc.c.o �
    {MODESET_MAC_DIR}{OBJDIR}:modelist.c.o   �
    {MODESET_MAC_DIR}{OBJDIR}:rtget.c.o      �
    {MODESET_MAC_DIR}{OBJDIR}:getit.c.o      �
    {MODESET_MAC_DIR}{OBJDIR}:mathrtns.c.o

# MPW's idea of VPATH
{MODESET_CODE_DIR}:{OBJDIR}:  � {MODESET_CODE_DIR}:
{MODESET_MAC_DIR}{OBJDIR}:    � {MODESET_MAC_DIR}

#
# Libraries that NVmodeset uses
#
DRIVER_LIBS=
	
# 'default_target' must be first target in this file
default_target � all
retail � all
debug checked � all
symbols � all
prints � all

ENTRY_POINTS=-init __init  -term __fini

{NV_MODESET_LIB} �  {MODESET_OBJS}
    {LD} -sharedlibrary -sharedata -f NVmodeset.exp -msg nodup {ENTRY_POINTS} -link slow -sym on -tb on -o {Targ} {MODESET_OBJS} {DRIVER_LIBS}

{NV_MODESET_LIB_STATIC} �  {MODESET_OBJS}
    {LD} -library -msg nodup -link slow -sym on -tb on -o {Targ} {MODESET_OBJS} {DRIVER_LIBS}

# this rule ensures the commands echo on execution
echo_on �
    unexport Echo
    set Echo 1

all clean � echo_on

#all �  mkdirs {NV_MODESET_LIB} {NV_MODESET_LIB_STATIC}
all �  mkdirs {NV_MODESET_LIB_STATIC}

mkdirs �
    newfolder {OBJDIRS} � Dev:Null || set Status 0

clean � 
    delete -y -ay -i {OBJDIRS}
    delete -y -ay -i {NV_MODESET_LIB}
    delete -y -ay -i {NV_MODESET_LIB_STATIC}



