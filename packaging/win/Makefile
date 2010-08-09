# Makefile to build a Windows installer (taosetup.exe)
#
# Usage:
#   make debug      # Build a debug package
#   make release    # Build a release package
#   make            # Same as make release

# Path to the Nullsoft Scriptable Install System (NSIS) compiler
MAKENSIS = "/c/Program Files/NSIS/makensis"
RM_RF = rm -rf

debug:
	./pkg.sh debug
	$(MAKENSIS) -DOUTFILE=taosetup_d.exe tao.nsi

release:
	./pkg.sh release
	$(MAKENSIS) -DOUTFILE=taosetup.exe tao.nsi 

clean:
	$(RM_RF) buildroot/ taosetup.exe taosetup_d.exe

