// SPDX-FileCopyrightText: 2025 adecc Systemhaus GmbH
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 \file
 \brief Doxygen License Overview Page for the adecc Scholar project

 \details
 \subpage licenses "License Overview and Usage"
 */

/**
 \page licenses License Overview and Usage

 \section license_intro Overview

 The **adecc Scholar** project is a free and open-source educational initiative.
 It is licensed primarily under the terms of the **GNU General Public License v3.0 or later (GPL-3.0-or-later)**.
  
 In addition to the main license, the project integrates selected components and examples under other open-source
 licenses such as the **MIT License**, **BSD-3-Clause**, and **Unlicense**. All license texts are included in
 the subdirectory `Licenses/`.

 \section license_structure License Distribution

 The project is modular, and the licensing varies by component:

 - **GPL-3.0-or-later**:  
   The core of the distributed time-tracking system, including:
   - All CORBA server and client components (ApplicationServer, Embedded Terminals, Qt Clients)
   - Source files in the `src/`, `idl/`, `qtgui/`, and `raspi/` directories
   - All code developed in this thread or extensions thereof

 - **MIT License**:  
   - Utility modules originally written under BSD-3-Clause have been relicensed to MIT for consistency
   - Contributions from **Michael Fuhs** (e.g. selected helper classes) are published under the MIT License
   - New helper and wrapper modules (e.g. `adecc_Database`, selected Qt tools)

 - **BSD-3-Clause**:  
   - Some earlier components were derived from BSD-licensed sources
   - These have been relicensed to MIT where appropriate, but the original license is still acknowledged in code comments

 - **Unlicense**:  
   - A few minimalistic utility functions were taken from the public domain
   - These are marked clearly and may be used without restrictions

 \section license_rights Rights and Permissions

 Depending on the license, the following rights apply:

 - **GPL-3.0-or-later**:
   - You may use, study, modify, and redistribute the software
   - Derivative works must also be licensed under GPL-3.0 or later
   - Source code must be made available upon distribution

 - **MIT License**:
   - Free use, modification, and redistribution, including in closed-source applications
   - Must retain the license text and copyright notice

 - **BSD-3-Clause**:
   - Similar to MIT, with the additional requirement to not use the names of contributors to promote derived products

 - **Unlicense**:
   - Public domain – no restrictions apply

 \section license_files License Texts

 All license texts can be found in the following location:

 - `LICENSES/GPL-3.0-or-later.txt`
 - `LICENSES/MIT.txt`
 - `LICENSES/BSD-3-Clause.txt`
 - `LICENSES/Unlicense.txt`

 \section license_conclusion Conclusion

 You are welcome to explore, modify, and reuse this code for educational or commercial purposes,
 within the terms of the respective licenses. When in doubt, prefer the more permissive license listed
 in the relevant module's source file, or consult the license text in the `Licenses/` folder.

 \author Volker Hillmann (adecc Systemhaus GmbH)
 \version 1.0
 \date 2025-05-14
 \copyright
 Copyright © 2020–2025 adecc Systemhaus GmbH  
 This documentation is released under the GNU General Public License v3.0 or later.

 \note This project is part of the adecc Scholar initiative —  
       Free educational resources for modern C++ and distributed systems.
*/
