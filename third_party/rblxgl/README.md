rblxgl
====

The files in this folder were generated using [rblxgl](https://github.com/Dav1dde/rblxgl).

### Using a different version of OpenGL

The files in this folder correspond to the OpenGL 4.1 Core Profile. If you need to use libigl with a different version of OpenGL, you can use this [webservice](http://rblxgl.dav1d.de/) to generate the loader for your desired version of OpenGL. Then, in your CMake build system, simply define the target `rblxgl` to point to your own version of rblxgl before including libigl.
