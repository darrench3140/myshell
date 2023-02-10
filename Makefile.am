AUTOMAKE_OPTIONS = foreign subdir-objects

LDDFLAGS = -lreadline -lpthread

bin_PROGRAMS = shell
shell_SOURCES = src/shell.cpp #just add shared source behind this with space, if any

#overrided: added a linkerflag -lreadline at end of sentence to work
shell$(EXEEXT): $(shell_OBJECTS) $(shell_DEPENDENCIES) $(EXTRA_shell_DEPENDENCIES) 
	@rm -f shell$(EXEEXT)
	$(CXX) $(shell_SOURCES) $(LDDFLAGS) -O2 -o $@
	@printf "\nRun \"sudo make install\" to install program\n\n"

install-data-hook:
	@printf "\nProgram Successfully Installed, type \"shell\" to start...\n\n"

clean-local:
	rm -fr config.status config.log config.h 
	rm -fr Makefile stamp-h1
	rm -fr */.dirstamp */.deps
	rm -fr TAGS ID GTAGS GRTAGS GSYMS GPATH tags
	rm -fr cscope.out cscope.in.out cscope.po.out cscope.files
	rm -fr config.cache configure.lineno config.status.lineno

reset:
	@make clean
	rm -fr configure config.h.in Makefile.in 
	rm -fr autom4te.cache aclocal.m4 build

#Extending(Automake) Source: https://www.gnu.org/software/automake/manual/html_node/Extending.html
