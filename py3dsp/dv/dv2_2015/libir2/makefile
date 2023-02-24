#
#  makefile to use if suppling ir2 with appliation
#
G_CFLAGS = -O2 -Wall -Wstrict-prototypes -D_REENTRANT -I/.
G_LFLAGS =
CC=gcc


# Compile vars 
CFLAGS = $(G_CFLAGS) -I/.
HFILES = ir2.h
OFILES = btos.o cbl.o clo.o d_clip_range.o \
df_build_card.o df_free_fbuffer.o df_init.o df_math.o  \
df_read_fits.o df_search_fheader.o df_stats.o df_write_fits.o dirlist.o  \
dpoly.o drange.o d_round.o elapse_msec.o elapse_sec.o error_string.o  \
get_vec.o map.o matchname.o my_atof.o \
my_atol.o my_timegm.o parse_r.o pathname.o  point_in_polygon.o  \
sock_flush.o sock_open.o sock_read_data.o sock_readline.o sock_read_msg.o  \
sock_setup_server.o sock_udp_message.o sock_write_data.o sock_write_msg.o \
stats.o stricmp.o strlwr.o  str_replace_sub.o str_rws.o strupr.o strxcat.o strxcpy.o \
t2d.o timestr.o unpad.o  
LIB_A=libir2.a

%.o:%.c $(HFILES)
	$(CC) -c $(CFLAGS) $*.c
 
all: $(LIB_A) 

$(LIB_A): $(OFILES) 
	ar rv $(LIB_A) $(OFILES) 
	ranlib $(LIB_A)
 
clean:
	rm -f *.o $(LIB_A) 

realclean:
	rm -f *.o $(LIB_A) 

