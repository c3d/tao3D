# Makefile to build a Windows installation package (tao.msi)
#
# Usage:
#   make debug      # Build a debug package
#   make release    # Build a release package
#   make            # Same as make release

RM_RF = rm -rf
AI = "/c/Program Files/Caphyon/Advanced Installer 7.6.1/bin/x86/AdvancedInstaller.com"

debug:
	./pkg.sh debug
	$(AI) //build tao.aip

release:
	./pkg.sh release
	$(AI) //build tao.aip

clean:
	$(RM_RF) tao-cache/ buildroot/ output/

