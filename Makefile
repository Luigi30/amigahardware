SRC_DIR = src
OBJDIR=obj
OUTDIR=bin

.PHONY: project

project:
	$(MAKE) -C $(SRC_DIR)
	
.PHONY: clean
clean :
	rm -r $(OBJDIR)
	rm $(OUTDIR)/raid
