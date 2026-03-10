;;; MONK-library Guix channel packages
;;;
;;; To use this channel, add to ~/.config/guix/channels.scm:
;;;
;;;   (cons (channel
;;;           (name 'monk)
;;;           (url "https://github.com/OUH-MESHLab/MONK-library")
;;;           (branch "main"))
;;;         %default-channels)
;;;
;;; Then run: guix pull && guix install monk python-monklib

(define-module (monk packages)
  #:use-module (guix packages)
  #:use-module (guix utils)
  #:use-module (guix git-download)
  #:use-module (guix build-system cmake)
  #:use-module (guix gexp)
  #:use-module ((guix licenses) #:prefix license:)
  #:use-module (gnu packages python)
  #:use-module (gnu packages python-xyz))

(define monk-source
  (origin
    (method git-fetch)
    (uri (git-reference
          (url "https://github.com/OUH-MESHLab/MONK-library.git")
          ;; Update this commit hash when tagging a new release.
          ;; To obtain the hash, run:
          ;;   guix hash -rx /path/to/MONK-library
          (commit "7508918dae0286e0b24759137605e59a773c9a59")))
    (file-name (git-file-name "monk" "0.2.4"))
    (sha256
     (base32
      "1lj6h78n6kxvaz7jzh9jl9kff839bcpcijpvcvfirs38siwlwd9h"))))

(define-public monk
  (package
    (name "monk")
    (version "0.2.4")
    (source monk-source)
    (build-system cmake-build-system)
    (arguments
     (list
      #:tests? #f
      #:configure-flags
      #~(list "-DMONK_PYTHON_BINDINGS=OFF"
              "-DMONK_BUILD_TOOLS=ON"
              "-DBUILD_TESTING=OFF")))
    (synopsis "C++ library and CLI tools for Nihon Kohden MFER waveform data")
    (description
     "MONK is a C++ library for reading waveform data in the Medical waveform
Format Encoding Rules (MFER) format as produced by Nihon Kohden patient
monitors.  This package provides the @code{libmonk} static library, C++ headers
with CMake integration, and the following command-line tools:

@table @command
@item mferdump
Dump raw hex tag contents of an MFER file.
@item mferinfo
Display header information; supports @code{--json} output.
@item mferanon
Anonymize patient fields (PID, PNM, AGE, SEX) in place.
@item mfer2csv
Export waveform channels to CSV with optional channel and time-range selection.
@end table")
    (home-page "https://github.com/OUH-MESHLab/MONK-library")
    (license #f)))  ; TODO: add a LICENSE file and replace #f with e.g. license:expat

(define-public python-monklib
  (package
    (name "python-monklib")
    (version "0.2.4")
    (source monk-source)
    (build-system cmake-build-system)
    (arguments
     (list
      #:tests? #f
      #:configure-flags
      #~(list "-DMONK_PYTHON_BINDINGS=ON"
              "-DMONK_BUILD_TOOLS=OFF"
              "-DBUILD_TESTING=OFF"
              ;; Override Python3_SITEARCH so the module installs under $out,
              ;; not into the Python store path.
              (string-append "-DPython3_SITEARCH="
                             #$output
                             "/lib/python"
                             #$(version-major+minor (package-version python))
                             "/site-packages"))))
    (inputs (list python))
    (native-inputs (list pybind11))
    (synopsis "Python bindings for the MONK MFER waveform library")
    (description
     "Python extension module (@code{monklib}) providing access to Nihon Kohden
MFER waveform data from Python.  Wraps the MONK C++ library via pybind11.")
    (home-page "https://github.com/OUH-MESHLab/MONK-library")
    (license #f)))  ; TODO: same as monk above
