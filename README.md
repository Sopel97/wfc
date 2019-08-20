# wfc
Wave Function Collapse algorithm implementation based on

https://github.com/mxgmn/WaveFunctionCollapse/
and the C++ implementation:
https://github.com/emilk/wfc

It aims to achieve clean, readable, well abstracted code; and high performance.
Supports parallel generation of multiple images at once.

Symmetry specification semantics differ slightly from the original implementation.
Also a different side compatibility specification approach is used - side id mapping is being used along with ability to disable specified connections (specify incompatibilities).
main.cpp contains example code.

The implementation does not provide input parsing of any kind.

Some example images are taken from the original repository.
