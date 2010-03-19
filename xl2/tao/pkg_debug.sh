#!/bin/bash
rm -rf pkg
mkdir pkg
cp debug/tao.exe pkg/
cp builtins.xl pkg/
for f in `ldd debug/tao.exe | grep -v 'WINDOWS\\\\system32' | grep -v 'ntdll.dll' | sed 's/^.*=> \\(.*\\)(0x.*)$/\\1/'` ; 
do 
    cp $f pkg/
done
