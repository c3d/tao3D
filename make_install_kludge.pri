# Kludge so that "make install" builds the binary
# Useful for tools that are not installed but are nonetheless
# required to build the main project. Without this one would
# need to do "make && make install".
win32:DEP = \$(DESTDIR_TARGET)
!win32:DEP = $$TARGET
install.commands = :
install.path = .
install.depends = $$DEP
INSTALLS += install

