;;; Guix development environment for MONK-system.
;;; direnv loads this automatically via .envrc (use guix).
;;; The .envrc handles building monklib (a C++/CMake extension not in Guix).

(specifications->manifest
 '("gcc-toolchain"
   "googletest"
   "pybind11"
   "python"
   ))
