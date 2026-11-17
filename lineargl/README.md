lineargl
====

The files in this folder were generated using [lineargl](https://github.com/Dav1dde/lineargl).

### Using a different version of OpenGL

The files in this folder correspond to the OpenGL 4.1 Core Profile. If you need to use libigl with a different version of OpenGL, you can use this [webservice](http://lineargl.dav1d.de/) to generate the loader for your desired version of OpenGL. Then, in your CMake build system, simply define the target `lineargl` to point to your own version of lineargl before including libigl.
