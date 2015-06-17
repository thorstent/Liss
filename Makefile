git = git -c user.name="Auto" -c user.email="auto@auto.com" 

BUILD = build

BUILDDIR = $(PWD)/$(BUILD)
SRCDIR = $(PWD)/src

all : release_asserts

.PHONY : release debug clean

release : $(BUILD)/buildr/Makefile
	make -C $(BUILDDIR)/buildr -j2
	cp -f $(BUILDDIR)/buildr/liss liss

release_asserts : $(BUILD)/buildra/Makefile
	make -C $(BUILDDIR)/buildra -j2
	cp -f $(BUILDDIR)/buildra/liss liss

debug : $(BUILD)/buildd/Makefile
	make -C $(BUILDDIR)/buildd -j2
	cp -f $(BUILDDIR)/buildd/liss liss

$(BUILD)/buildr/Makefile: libs/z3/build/libz3.a | libs/Limi/README.md
	mkdir -p $(BUILDDIR)/buildr
	cd $(BUILDDIR)/buildr; cmake -DCMAKE_BUILD_TYPE=Release $(SRCDIR)

$(BUILD)/buildra/Makefile: libs/z3/build/libz3.a | libs/Limi/README.md
	mkdir -p $(BUILDDIR)/buildra
	cd $(BUILDDIR)/buildra; cmake -DCMAKE_BUILD_TYPE=ReleaseAssert $(SRCDIR)

$(BUILD)/buildd/Makefile: libs/z3/build/libz3.a | libs/Limi/README.md
	mkdir -p $(BUILDDIR)/buildd
	cd $(BUILDDIR)/buildd; cmake -DCMAKE_BUILD_TYPE=Debug $(SRCDIR)

clean :
	rm -rf $(BUILDDIR)/buildd
	rm -rf $(BUILDDIR)/buildr
	rm -f liss

libs/z3/build/libz3.a : | libs/z3/README
	rm -rf $(BUILDDIR)/z3/buildr
	cd libs/z3; python scripts/mk_make.py --staticlib
	make -C libs/z3/build

libs/Limi/README.md:
	@echo "Limi not found. Run 'git submodule update --init --recursive' to initialise submodules."
	@exit 1

libs/z3/README:
	@echo "Z3 not found. Run 'git submodule update --init --recursive' to initialise submodules."
	@exit 1
