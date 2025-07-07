// SPDX-FileCopyrightText: 2025 adecc Systemhaus GmbH
// SPDX-License-Identifier: GPL-3.0-or-later

/**
\file
\brief Import/export macro definition for the WeatherAPI shared library.

\details
This header centralizes the management of symbol visibility for the WeatherAPI module, ensuring that all exported classes and functions are correctly marked for use with both shared library (DLL/SO) builds and static builds across different platforms and compilers.

- On Windows and Cygwin, `WEATHERAPI_API` expands to `__declspec(dllexport)` during library build (`WEATHERAPI_BUILD_DLL` defined), and to `__declspec(dllimport)` when used by consumers.
- On other systems (Linux, macOS), the macro expands to `__attribute__((visibility("default")))`, marking symbols as publicly visible for dynamic linking.
- If neither condition applies (e.g. static library builds), the macro is defined as empty.

This approach ensures:
- Consistent and correct symbol export/import semantics.
- Portability across Windows, Linux, and other UNIX-like systems.
- Compatibility with all modern C++ compilers.

**Usage example:**
\code
class WEATHERAPI_API WeatherReader { ... };
WEATHERAPI_API void query_weather();
\endcode

\warning
Always use the `WEATHERAPI_API` macro for all classes and functions that should be visible outside the shared library.
Incorrect use may lead to link errors or missing symbols.

\todo Consider integrating static analysis to ensure all exported symbols use the macro.
\todo Extend macro to support future platforms or special build requirements.

  \version 1.0
  \date    30.06.2025
  \author  Volker Hillmann (adecc Systemhaus GmbH)

  \copyright Copyright © 2020 - 2025 adecc Systemhaus GmbH

  \licenseblock{GPL-3.0-or-later}
  This program is free software: you can redistribute it and/or modify it
  under the terms of the GNU General Public License, version 3,
  as published by the Free Software Foundation.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program. If not, see <https://www.gnu.org/licenses/>.
  \endlicenseblock

  \note This file is part of the adecc Scholar project – Free educational materials for modern C++.

*/
#pragma once


#if defined(_WIN32) || defined(__CYGWIN__)
#  ifdef WEATHERAPI_BUILD_DLL
#    define WEATHERAPI_API __declspec(dllexport)
#  else
#    define WEATHERAPI_API __declspec(dllimport)
#  endif
#else
#  define WEATHERAPI_API __attribute__((visibility("default")))
#endif

