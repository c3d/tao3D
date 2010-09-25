# Makefile to build a Windows installer (taosetup.exe)
#
# Usage:
#   make debug      # Build a debug package
#   make release    # Build a release package
#   make            # Same as make release

# Path to the Nullsoft Scriptable Install System (NSIS) compiler
MAKENSIS = "/c/Program Files/NSIS/makensis"
RM_RF = rm -rf
BITMAPS = inst_header.bmp uninst_header.bmp inst_welcome_finish.bmp uninst_welcome_finish.bmp

define run-nsis-debug
$(MAKENSIS) -DOUTFILE=taosetup_d.exe tao.nsi
endef

define run-nsis-release
$(MAKENSIS) -DOUTFILE=taosetup.exe tao.nsi
endef

debug: $(BITMAPS)
	./pkg.sh debug
	$(run-nsis-debug)

release: $(BITMAPS)
	./pkg.sh release
	$(run-nsis-release)

rnd: $(BITMAPS)
	$(run-nsis-debug)

rnr: $(BITMAPS)
	$(run-nsis-release)


clean:
	$(RM_RF) buildroot/ taosetup.exe taosetup_d.exe *.bmp

%.bmp:%.bmp.gz
	gunzip -c $< >$@

