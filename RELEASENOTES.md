## 0.8.0



















feature: add Windows dlfcn support and expose mulle-time in amalgamation

* provide dlfcn-win32 implementation and public header to enable dlopen/dlsym on Windows
* include mulle-time headers and sources in public includes and build so time APIs are available to consumers
* install TOC.md from constituent libraries into share/ on install to surface per-constituent metadata
