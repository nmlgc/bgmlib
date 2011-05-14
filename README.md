## How to roughly compile this on Visual Studio 2017

* Make sure you got all submodules:

  ```
  $ git submodule init
  $ git submodule update
  ```

* Convert all other Visual Studio solutions to the current version. With FOX,
  you only need to convert `fox.dsp` and `reswrap.dsp`, you can disable all
  other projects to save time.
* Make sure to consistently set the same *C/C++ → Code Generation → Runtime
  Library* option in all projects, because not all of them will come with the
  same by default.

  Most of them should then compile fine, except…

### Fixes for the FOX Toolkit

* Open `libs\fox\windows\vcpp\fox\fox.vcxproj` in a plaintext editor and remove
  the following file names from the two `reswrap` command lines (simply Ctrl-F
  for `reswrap`):
  * `bigfloppy3.gif`
  * `bigfloppy5.gif`
  * `bigharddisk.gif`
  * `minifloppy3.gif`
  * `minifloppy5.gif`

  Or, simply replace everything matching the regular expression

  ```regex
  (bigfloppy3.gif|bigfloppy5.gif|bigharddisk.gif|minifloppy3.gif|minifloppy5.gif)
  ```

  with an empty string.
* Remove the nonexistent `FXReactor.cpp` from the `fox` project.
* Set *C/C++ → Code Generation → Enable Function-Level Linking* to **Yes (/Gy)**
  for the Debug configuration of both the `reswrap` and `fox` projects.
* Add `HAVE_STRTOLL` and `HAVE_STRTOULL` to the list of `#define`'d preprocessor
  macros for the `fox` project at *C/C++ → Preprocessor → Preprocessor
  Definitions*.

### Fixes for `bgmlib`

* Make sure to consistently set the same *C/C++ → Code Generation → Runtime
  Library* option you also used for the dependent libraries.

* Add `SUPPORT_VORBIS_PM` to the list of `#define`'d preprocessor macros at
  *C/C++ → Preprocessor → Preprocessor Definitions*. (The other code paths are
  broken.)

* Point the compiler to the subdirectories of all dependencies by adding the
  following to the list at *C/C++ → General → Additional Include Directories*,
  **in this order**:

  ```
  $(ProjectDir)libs\fox\include\
  $(ProjectDir)libs\ogg\include\
  $(ProjectDir)libs\vorbis\include\
  ```

* Fix the type error in `libvorbis.cpp`:

```diff
--- a/libvorbis.cpp
+++ b/libvorbis.cpp
@@ -547,7 +547,7 @@ const double rate_mapping_44_stereo[12]=

 float vorbis_quality_to_bitrate(const float& q)
 {
-       float ds = 0.0, _is;
+       double ds = 0.0, _is;
        int is = 0.0;

        ds =modf(q, &_is);
```
