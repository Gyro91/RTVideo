

################################################################################
# User defined variables.
#
# SRC: put all the .c files you want to compile. e.g.
# 	SRC	= my.c awesome.c set.c of.c files.c
#
# BIN: put the final executable name here. e.g.
# 	BIN = my_awesome_bin
################################################################################

SRC = RTVideo.c Draw.c Periodicity.c Sched_new.c Task.c
BIN = RTVideo

################################################################################
# User libraries
#
# MYLIBS_CFLAGS: mostly useful for defining an include directory, i.e. where to
# 	look for additional header files. e.g.
# 	MYLIBS_CFLAGS	=	-I../p_wrapper/include
#
# MYLIBS_LDFLAGS: mostly useful for adding a directory to be searched for
# libraries and linking an additional shared object
# 	MYLIBS_LDFLAGS	=	-L../p_wrapper/ -Wl,-rpath,../p_wrapper/ -lp_wrapper
################################################################################

MYLIBS_CFLAGS	=
MYLIBS_LDFLAGS	=

################################################################################
# Some allegro installations provide a .pc file to be used with pkg-config
# (which is the tool i prefer for obtaining libs and cflags)
# If you experience linking or compilation problems, comment the pkg-config
# lines and uncomment the allegro-config counterpart.
# Furthermore, this makefile assumes you have a "standard" (see man 7 hier)
# installation of allegro. If you or distribution did anything kinky, you are on
# your own. I'm not sorry about that.
################################################################################

ALLEGRO_LDFLAGS = $(shell pkg-config --libs allegro)
ALLEGRO_CFLAGS	= $(shell pkg-config --cflags allegro)



#### flags #####################################################################
CPPFLAGS 	= $(CPPFLG_D_$(D)) -D_GNU_SOURCE
CFLAGS   	=  -std=c99 -Wpedantic -Wall -Wextra -g3 \
			  $(ALLEGRO_CFLAGS) $(MYLIBS_CFLAGS) 

# I'm not sure librt is needed but let it stay there for the time being
LDFLAGS  	= -lpthread -lrt $(ALLEGRO_LDFLAGS) $(MYLIBS_LDFLAGS) -lm
################################################################################

#### debug disable #############################################################
D = 1
ifdef NDEBUG
	D = 0
endif

CPPFLG_D_1 =
CPPFLG_D_0 = -DNDEBUG
CFLG_D_1 = -g3
CFLG_D_0 =
################################################################################

################################################################################
DEPDIR = .deps
OBJ = ${SRC:.c=.o}
################################################################################

.PHONY: all
all: options $(BIN)

.PHONY: help
help:
	@printf -- "How to use this makefile:\n"
	@printf -- "- Add source files to the SRC variable\n"
	@printf -- "- Set the BIN variable to the output binary name\n"
	@printf -- "- Run 'make'\n"
	@printf -- "\n"
	@printf -- "\t/!\ WARNING!\n"
	@printf -- "\tThis makefile won't save your ass if you put the name of\n"
	@printf -- "\tan existing file in the BIN variable\n"
	@printf -- "\n"
	@printf -- "By default, sources are built in debug mode. This mainly\n"
	@printf -- "implies -g3 in the CFLAGS\n"
	@printf -- "\n"
	@printf -- "Setting the NDEBUG variable will disable debug mode and pass\n"
	@printf -- "the NDEBUG macro to each compilation unit (i.e. no asserts)\n"
	@printf -- "\n"
	@printf -- "The 'all' target will build the BIN target\n"
	@printf -- "The 'options' target can be used to inspect build flags\n"
	@printf -- "The 'clean' target will clean all the generated files\n"
	@printf -- "\n"
	@printf -- "When in doubt, use 'make -n' for a dry run\n"
	@printf -- "\n"
	@printf -- "There's no real lion in the makefile code. Feel free to tweak\n"
	@printf -- "it to better fit your needs\n"

.PHONY: options
options:
	@printf -- "%s build flags\n" "$(BIN)"
	@printf -- "CPPFLAGS = %s\n" "$(CPPFLAGS)"
	@printf -- "CFLAGS   = %s\n" "$(CFLAGS)"
	@printf -- "LDFLAGS  = %s\n" "$(LDFLAGS)"
	@printf -- "DEPDIR	 = %s\n" "$(DEPDIR)"

%.o : %.c | ${DEPDIR}
	$(CC) -MMD -c $(CPPFLAGS) $(CFLAGS) $<
	@mv $*.d "${DEPDIR}/"

$(BIN) : $(OBJ)
	$(CC) -o "$@" $^ $(LDFLAGS)

.PHONY: clean
clean: cleandeps cleanobjs cleanbin

.PHONY: cleandeps
cleandeps:
	rm -f "$(DEPDIR)"/*.d
	-rmdir "$(DEPDIR)"

.PHONY: cleanobjs
cleanobjs:
	rm -f *.o

.PHONY: cleanbin
cleanbin:
	rm -f "$(BIN)"

$(DEPDIR):
	mkdir -p "$(DEPDIR)"

-include $(SRC:%.c=$(DEPDIR)/%.d)

