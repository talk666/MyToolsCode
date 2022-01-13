current_dir = $(PWD)

SUBDIRS += sdk/api 
SUBDIRS += sdk/cap 
SUBDIRS += library/sdf 
SUBDIRS += application/basic
SUBDIRS += application/sdf/kmt

.PHONY:all
all:
	@list='$(SUBDIRS)';for subdir in $$list; do \
		cd $$subdir && make && cd $(current_dir); \
	done
	
.PHONY:clean
clean:
	@list='$(SUBDIRS)'; for subdir in $$list; do \
		echo "Clean in $$subdir";\
		cd $$subdir && make clean && cd $(current_dir);\
	done
