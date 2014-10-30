# Tao3D - A language for dynamic 3D animations

Tao3D is a dynamic document description language designed specifically
for real-time 3D animations. It can be used to present complex information
in an entertaining and interactive way.

Tao3D includes support for the following features:

* A complete programming language, with loops, tests, functions, and more.
* Real-time, capable of displaying complex animations at 60FPS.
* Sophisticated 2D and 3D text layout, 2D and 3D shapes, extrusion.
* 3D objects from a variety of formats, including .OBJ and .3DS
* 3D point clouds supporting hundreds of thousands of points
* Integrated audio and movie support based on industry standard VLC
* Real-time updates based on external data files in .CSV format
* Fetch data, pictures or movies directly over HTTP

For a better overview of the software, please watch this YouTube video:
https://www.youtube.com/watch?v=Fvi29XAo4SI.

## Prerequisites

The software can be built on Windows, MacOSX and Linux.
To build Tao, you will need:

* A C++ compiler (Tested with g++ and clang)
* OpenGL at least version 2.1, and related development files.
  Tao does not presently support OpenGL ES, but we'd like this to happen.
* Git
* Qt version 4.8 or above (there is a crash on MacOSX with 5.3.2)
* LLVM version 2.9 (porting to 3.x is left as an exercise for the reader)

The software also downloads or includes a few extra packages, including:
* NodeJS
* CryptoPP (a somewhat dated)
* GLC-Lib (a rather old version)
* VLC (you may need to build it yourself if you want multistream support)

Many thanks to all the authors who indirectly made Tao possible


## Free Software (GPLv3) and Commercial License

This software is free, available under the GNU General Public License v3.
Commercial licenses are also available. The commercial version of the product
adds, among other things:

* Direct support for stereoscopic and most auto-stereoscopic displays
* Decoding the vast majority of 3D movie formats, tiled or multi-stream
* The ability to encrypt and digitally sign the source code
