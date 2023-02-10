build:
	g++ src/shell.cpp -lreadline -lpthread -O2 -o obj_dir/shell

install:
	cp obj_dir/shell /usr/bin/
