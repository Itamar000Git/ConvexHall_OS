.PHONY: all clean

SUBDIRS = tar1 tar2 tar3 tar4 tar5_8 tar6 tar7 tar9 tar10

all:
	@for dir in $(SUBDIRS); do \
		$(MAKE) -C $$dir; \
	done

clean:
	@for dir in $(SUBDIRS); do \
		$(MAKE) -C $$dir clean; \
	done