# PowerMorph

Implementation of the morphing algorithm used in Michael Jackson's Black or White music video.

This is what this software can produce:
![test2](https://github.com/user-attachments/assets/9da7c779-6c6f-4c20-b1f6-ce3ca59ab7a1)


### Build

You need CMake to generate Makefiles and/or project files for your IDE.

1.) Create folder in the root of the repo called ```build```.

2.) ```cd``` into build and run
```bash
cmake ..
```
The default target should be built by CMake. To check what is your default target
run
```bash
cmake -G
```


### References

Beier-Neely algorithm:

- http://www.hammerhead.com/thad/morph.html
- https://www.cartoonbrew.com/vfx/oral-history-morphing-michael-jacksons-black-white-144015.html
- https://www.youtube.com/watch?v=8CU5SVwzSzs
- https://www.cs.toronto.edu/~mangas/teaching/320/assignments/a4/

