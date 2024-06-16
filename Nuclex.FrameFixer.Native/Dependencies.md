Nuclex.FrameFixer.Native Dependencies
=====================================


To Compile the Application
--------------------------

This project is intended to be placed into a source tree using submodules to replicate
the following directory layout:

    root/
        Nuclex.FrameFixer.Native/   <-- you are here
            CMakeLists.txt

        Nuclex.Platform.Native/     <-- Git: nuclex-shared-cpp/Nuclex.Platform.Native
            CMakeLists.txt

        Nuclex.Support.Native/      <-- Git: nuclex-shared-cpp/Nuclex.Support.Native
            CMakeLists.txt

        build-system/               <-- Git: nuclex-shared/build-system
            cmake/
                cplusplus.cmake

        third-party/
            nuclex-googletest/      <-- Git: nuclex-builds/nuclex-googletest
                CMakeLists.txt
            nuclex-celero/          <-- Git: nuclex-builds/nuclex-ceelero
                CMakeLists.txt

You should already have that directory layout in playe if you cloned the "frame fixer"
repository (with `--recurse-submodules`).

The raw dependencies of the code itself are:

  * Nuclex.Support.Native
  * Nuclex.Platform.Native
  * gtest (optional, if unit tests are built)
  * celero (optional, if benchmarks are built)
  * qt5 (the "Widgets" and "Sql" components)
  * SQLite (via qt5's sqlite extension, built-in by default)
