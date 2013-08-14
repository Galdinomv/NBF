#***********************************************************
# This File is originally for Moldyn.
# Modified to work with NBF
#***********************************************************/

#***********************************************************/
# File :  Makefile  (for  nbf.c)
#***********************************************************
    CC = g++
    CFLAGS = -g -O3 -DVERSION_DYNAMIC -DLIBTM
    LDFLAGS = -lm -lpthread -l_tm

    VERSION = -DVERSION_DYNAMIC

    export VERSION

LIBTM_DIR = ../..
OUT_DIR = ../..


.c.o:
	$(CC) $(CFLAGS) -c $*.c 

#***********************************************************
    TARGET = nbf
    OBJS   = nbf.o
#***********************************************************

$(TARGET) : $(OBJS)
	$(CC) $(CFLAGS) -o $(OUT_DIR)/$@ $(OBJS) $(LDFLAGS) -L$(LIBTM_DIR)

#***********************************************************
#    Dependencies
#***********************************************************

nbf.o   : nbf.c Makefile

#***********************************************************
clean:
	/bin/rm -f $(OBJS) $(OUT_DIR)/$(TARGET)
