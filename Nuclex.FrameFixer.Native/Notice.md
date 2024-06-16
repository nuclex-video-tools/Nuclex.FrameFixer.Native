Nuclex.FrameFixer.Native Attribution
====================================

This application is licensed under the Apache License 2.0,
http://www.apache.org/licenses/


You can:
--------

- Use this application as-is
- Distributes binaries compiled from the unmodified source code.
- Repackage such binaries together with other tools.

- Modify the source code of this application and keep the changes to yourself.
- Modify the source code of this application and publish the changes,
  so long as you make it very clear that, and how, you changed the code.


You can not:
------------

- Put this application under a different license or
- Sell access to this application's code

- Sue anyone for issues you have with this code.

- Pretend that you wrote this
- Use the names of any of this application's authors to promote your own work.


Attribution
===========

Nuclex.FrameFixer.Native stands on the shoulders of several other libraries
and frameworks provided by the Open Source community.

The following attributions mention these libraries and their authors. If you feel I forgot
someone, or your contribution in particular, feel free to contact me and point it out!

Uses the Qt Framework (Version 5), developed by The Qt Company, under the LGPL 2.1 license.
As mandated by the LGPL, you can relink the application to any compatible version of Qt.
(https://www.qt.io/licensing/open-source-lgpl-obligations)

Uses the SQLite database engine, which has been placed in the public domain. The variant used
is that embedded in the Qt libraries, which should have no modifications on its source code.
(https://www.sqlite.org/copyright.html)

Uses supporting code from the Nuclex.Platform.Native library, written by
Markus Ewald and licensed under the terms of the Apache License 2.0
(http://www.apache.org/licenses/)

Uses supporting code from the Nuclex.Support.Native library, written by
Markus Ewald and licensed under the terms of the Apache License 2.0
(http://www.apache.org/licenses/)

Nuclex.Support.Native also embeds additional libraries, belonging to their
respective owners and used according to their respective licenses:

  * The James Edward Anhalt III integer formatter under the MIT license
  * Junekey Jeon's DragonBox float formatter under the Apache 2.0 license
  * Ulf Adams' float parser from the Ryu library under the Apache 2.0 license
  * Cameron Desrochers' Multi-threaded queues under the Boost Software license


Third-Party Code
================

This library also embeds carefully selected third-party code which falls
under its own licenses, listed below


James Edward Anhalt III Integer Formatter: MIT License
------------------------------------------------------

A modified version of James Edward Anhalt III.'s integer printing algorithm
is used to convert integral values into strings. The original code is licensed
under MIT license (https://mit-license.org/)

If you distribute binaries compiled from this library, you do not have to
do anything, but an acknowledgement of the original author would be nice.


DragonBox: Apache License 2.0
-----------------------------

Uses code from Junekey Jeon's DragonBox reference implementation to convert
floating point values into string independent of the system locale. Licensed
under the Apache 2.0 license (http://www.apache.org/licenses/LICENSE-2.0)

If you distribute binaries compiled from this library, you do not have to
do anything, but an acknowledgement of the original author would be nice.


Ryu String to Float Parser: Apache License 2.0
----------------------------------------------

Code from the Ryu library is used to convert strings to floating point values
independent of the system locale. It is written by Ulf Adams and licensed under
the Apache 2.0 license (http://www.apache.org/licenses/LICENSE-2.0)

If you distribute binaries compiled from this library, you do not have to
do anything, but an acknowledgement of the original author would be nice.


MoodyCamel Lock-Free Unbounded Concurrent Queue
-----------------------------------------------

The Linux thread pool code internally uses the unbounded concurrent queue code
by Cameron Desrochers, included in a subdirectory. It is license under
the Boost Software License (https://www.boost.org/users/license.html)

If you distribute binaries compiled from this library, you do not have to
do anything, but an acknowledgement of the original author would be nice.
