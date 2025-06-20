// SPDX-FileCopyrightText: 2025 adecc Systemhaus GmbH
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 \file
 \brief Doxygen Documentation Page for the CORBA Client Application

 \details client implementationen, complete workcycle and a first template
 
 Contains documentation pages \ref appclient and \ref ORBClientPage
 
 \author Volker Hillmann (adecc Systemhaus GmbH)
 
 \date 2025-05-15 first version with lifecycle
 \date 2025-06-13 first client template
 
 \copyright
 Copyright © 2020–2025 adecc Systemhaus GmbH  
 This program is free software: you can redistribute it and/or modify it  
 under the terms of the GNU General Public License, version 3.  
 See <https://www.gnu.org/licenses/>.

 \note This documentation is part of the adecc Scholar project —  
       Free educational materials for distributed systems in modern C++.

*/

/**
 \page appclient CORBA Client Application – Overview
 \brief Overview page for implementation of the CORBA client interface.

 \section appclient_intro Introduction

 \details This CORBA Client Application demonstrates how to interact with a distributed employee management system 
          using the `Organization::Company` CORBA interface. It is part of the adecc Scholar initiative and is designed 
          to run on desktop platforms such as Windows and Linux.

 \details It showcases:
           - Remote invocation of company and employee services via CORBA
           - Proper handling of CORBA exceptions and error cases
           - Interaction with the CORBA Naming Service
           - Object lifecycle and memory management for transient servant objects

 \section appclient_goals Learning Objectives

 \details The client illustrates:
          - Resolving CORBA object references via the Naming Service
          - Correct use of `*_var` and `*_ptr` CORBA smart pointer types
          - Managing dynamically created (transient) CORBA servants
          - Cleanly destroying remote servant objects using `destroy()` calls
          - Dealing with CORBA-specific exceptions (e.g., `TRANSIENT`, `COMM_FAILURE`)

 \section appclient_sequence Functional Walkthrough

 The client application performs the following steps:

 1. **ORB Initialization**
    - Initializes the CORBA ORB with command line arguments.

 2. **Naming Service Resolution**
    - Resolves the CORBA Naming Service and looks up the `Organization::Company` object reference.

 3. **Remote Method Calls**
    - Retrieves company name and total salary using `nameCompany()` and `getSumSalary()`.
    - Retrieves complete employee data directly via `getEmployeeData(id)` into a local structure.
    - Uses `getEmployees()` to retrieve all active employees as a CORBA sequence.
    - Calls `getEmployee(id)` to retrieve individual `Employee` servant objects.

 4. **Remote Object Destruction**
    - Each `Employee` servant obtained via `getEmployee()` or `getEmployees()` must be explicitly destroyed
      by calling `employee->destroy()` to release server-side resources.
    - This is critical because these objects are created under a **transient POA** on the server.
      Failure to destroy them results in memory/resource leaks.

 5. **ORB Shutdown**
    - Destroys the ORB and ensures all CORBA memory is properly released.

 \note See also the \ref appserver "Server Documentation" for corresponding servant lifecycle handling.

 \section appclient_references Additional Notes

 - Uses TAO as the CORBA implementation
 - Interacts with both persistent (`Company`) and transient (`Employee`) objects
 - The server handles transient objects using a `DestroyableInterface` base class, and the client
   must cooperate by calling `destroy()` on each servant reference before releasing it.
	
 \section appclient_sources Source code
 
 \details This section presents the original source code fragments of a typical CORBA client. 
          The steps shown in the `main()` function are fundamentally required for all CORBA clients, 
		  including ORB initialization, resolving the naming service, and narrowing object references. 
		  The methods `getEmployee()`, `getEmployeeData()`, and `getEmployees()` are invocations of 
		  CORBA operations defined in the IDL of the sample application.

 \details The method `getEmployee()` returns a full CORBA object reference to a transient servant that 
          exists and operates on the server side. It allows dynamic access to live employee data and 
		  supports active interactions like attribute retrieval or lifecycle control (e.g., destruction 
		  of the servant). This is typically used when real-time data or server-side logic is required.

 \details In contrast, `getEmployeeData()` returns a copy of the employee information in a simple data 
          structure (IDL struct). The data is transmitted to the client, and any subsequent processing or 
		  display takes place entirely on the client side. There is no connection to a server-side servant, 
		  making it more lightweight but limited to static snapshot data.
	
 \subsection appclient_sources_get_employee Reading a Employee via Corba
 \details This method demonstrates how to request and handle a single transient CORBA servant object
          from a remote `Organization::Company` interface based on a specific employee ID.
 
          The function uses `_duplicate()` to retain a local reference to the `Company` interface and then calls 
          `getEmployee(seekId)` to retrieve a transient `Organization::Employee` servant representing the target person.
 
          If a valid servant is returned (i.e., not nil), its main attributes (full name, salary, activity status) 
		  are accessed and printed. Any `CORBA::Exception` or user-defined exceptions like `EmployeeNotFound` are 
		  handled explicitly.
 
          To manage memory and lifecycle correctly, the object is wrapped in a `make_destroyable()` helper that ensures
          the transient servant on the server is properly destroyed at the end of the method's scope.
 
 \note The returned `Employee` reference is transient and must be explicitly destroyed by the client.
       If `make_destroyable()` is not used, the caller must manually call `destroy()` and reset the reference to `_nil()` 
       to avoid leaking resources on the server side.
 
 \note Omitting cleanup will result in orphaned servants within the transient POA.
       This is particularly important in high-load scenarios or long-running clients.
	   
 \code{.cpp}
 void testReadEmployee(Organization::Company_ptr comp_in, CORBA::Long seekId) {
   Organization::Company_var company_var = Organization::Company::_duplicate(comp_in);
   std::println(std::cout, "[Client {}] Requesting employee with ID: {}", 
         getTimeStamp(from_timepoint(company_var->getTimeStamp())), seekId);
   Organization::Employee_var employee_var = Organization::Employee::_nil();
   try {
      std::println(std::cout, "   Entering scope for Employee_var (ID: {}) ...", seekId);
      employee_var = company_var->getEmployee(seekId);
      if (!CORBA::is_nil(employee_var.in())) {
         CORBA::String_var fullName = employee_var->getFullName(); // String_var guaranting destroying of return value
         std::println(std::cout, "    Found Employee - Name: {}, Active: {}, Salary: {:.2f} ",
                                        fullName.in(), 
                                        (employee_var->isActive() ? "Yes" : "No"), 
                                        employee_var->salary());
         }
      else {
         std::println(std::cerr, "    WARNING: getEmployee({}) returned nil unexpectedly.", seekId);
         }
      std::println(std::cout, "  ...leaving scope for Employee_var (ID: {}). Reference released.", seekId);
      }
   catch (Organization::EmployeeNotFound const& ex) { 
      std::println(std::cerr, "  ERROR: Caught EmployeeNotFound for ID {}", ex.requestedId);
      }
   catch (CORBA::Exception const& ex) {
      std::ostringstream os;
      os << ex; 
      std::println(std::cerr, "  ERROR: CORBA Exception during getEmployee({}): {}", seekId, os.str());
      }
   catch(std::exception const& ex) {
      std::println(std::cerr, "  ERROR: C++ Exception during getEmployee({}): {}", seekId, ex.what());
      }

   // destroying the instance which bounded with employee_var, that resources on server get free
   // Server destroying the trnasient Servant.
   if (!CORBA::is_nil(employee_var.in())) {
      employee_var->destroy();
      employee_var = Organization::Employee::_nil();
      }
   }
   \endcode
 
 \subsection appclient_sources_get_employees Reading All Employees via CORBA
 \details This example demonstrates how a CORBA client interacts with a sequence of servant objects 
  returned by a remote call to `Organization::Company::getEmployees()` using TAO/ACE.
 
  The method `ReadEmployees()`:
  - Uses `_duplicate()` to safely retain the reference to the company interface
  - Calls `getEmployees()` to retrieve a sequence of transient CORBA object references
  - Iterates over each reference, reads selected attributes, and formats output
  - Calls `destroy()` on each servant after use to release resources on the server
 
  \details  <b>Important Notes</b>
  - All employee objects in the sequence are transient. They are created per request on the server side.
  - It is the client’s responsibility to explicitly call `destroy()` on each servant.
    This ensures that the server removes the servant from its transient POA.
  - Failing to destroy the servants will cause memory/resource leaks on the server.
  - After calling `destroy()`, the client must also set the reference to `_nil()` to break ownership.
  - Use `CORBA::is_nil()` to check the validity of each reference before accessing.
 
  \details <b>Code Example</b>
  
  \code{.cpp}
  void ReadEmployees(Organization::Company_ptr comp_in) {
     Organization::Company_var company_var = Organization::Company::_duplicate(comp_in);
     Organization::EmployeeSeq_var employees_seq = company_var->getEmployees();
 
     for (CORBA::ULong i = 0; i < employees_seq->length(); ++i) {
        Organization::Employee_ptr current_employee = employees_seq[i];
        if (CORBA::is_nil(current_employee)) continue;
 
        auto fullName = toString(current_employee->getFullName());
        std::println(std::cout, "Employee {}: {}", i, fullName);
     }
 
     for (CORBA::ULong i = 0; i < employees_seq->length(); ++i) {
        if (!CORBA::is_nil(employees_seq[i].in())) {
           employees_seq[i]->destroy();
           employees_seq[i] = Organization::Employee::_nil();
        }
     }
  }
  \endcode
 
	
 \subsection appclient_sourcesmain Main Function
 \details The following subsection show the source code of a main function for a CORBA client to use the walkthrough.
 
 \code{.cpp}
 int main(int argc, char *argv[]) {
   std::println(std::cout, "[Client] Client Testapplication for demonstrate the use of CORBA.");
   std::unique_ptr<Organization::EmployeeData> empData;
   try {
      // 1. ORB initialisieren
      CORBA::ORB_var orb = CORBA::ORB_init(argc, argv);
      std::println(std::cout, "[Client] ORB initialized.");

      // 2. Naming Service Referenz holen
      CORBA::Object_var naming_obj = orb->resolve_initial_references("NameService");
      CosNaming::NamingContext_var naming_context = CosNaming::NamingContext::_narrow(naming_obj.in());
      if (CORBA::is_nil(naming_context.in())) throw std::runtime_error("Failed to narrow Naming Context.");
      std::println(std::cout, "[Client] Naming Service context obtained.");

      // 3. Company Object Reference obtained from Naming Service
      CosNaming::Name name;
      name.length(1);
      name[0].id = CORBA::string_dup("GlobalCorp/CompanyService"); 
      name[0].kind = CORBA::string_dup("Object");

      std::println(std::cout, "[Client] Resolving 'GlobalCorp/CompanyService'...");
      CORBA::Object_var company_obj = naming_context->resolve(name);
      Organization::Company_var company_var = Organization::Company::_narrow(company_obj.in());
      if (CORBA::is_nil(company_var.in())) throw std::runtime_error("Failed to narrow Company reference.");
      std::println(std::cout, "[Client] Successfully obtained reference to Company object.");

      // Firmenname und Gehaltsummer bestimmen
      std::println(std::cout, "Firma: {}, zu zahlende Gehälter: {:.2f}", company_var->nameCompany(), company_var->getSumSalary());

      // Direkter Transfer, Daten werden direkt übertragen und dann nur lokal gespeichert
      empData.reset(company_var->getEmployeeData(110));

      // === Beispiel: getEmployees() ===
      std::println(std::cout);
      std::println(std::cout, "--- Calling getEmployees() ---");
      ReadEmployees(company_var.in());
      

      // === example for getEmployee(personId) ===
      // --- 1st case 1: call  an existing employee ---
      std::println(std::cout);
      std::println(std::cout, " --- Calling getEmployee() ---");

      CORBA::Long existingId = 101;
      std::println(std::cout);
      ReadEmployee(company_var.in(), existingId);
      
      // --- Case 2: not existing employee ---
      CORBA::Long nonExistingId = 999;
      std::println(std::cout);
      testReadEmployee(company_var.in(), nonExistingId);
      
      // short break (optional)
      std::this_thread::sleep_for(std::chrono::seconds(2));

      // 4. clean up
      std::println(std::cout);
      std::println(std::cout, "[Client] Shutting down.");
        
      while (orb->work_pending()) orb->perform_work();
      orb->destroy();

      std::println(std::cout, "[Client] ORB destroyed.");
      } 
   catch (Organization::EmployeeNotFound const& ex) {
      // Safety net, in case the exception occurs outside the specific try-catch block
      std::println(std::cerr, "[Client] Unhandled EmployeeNotFound Exception: ID {}", ex.requestedId);
      return 1;
      } 
   catch (CORBA::COMM_FAILURE const& ex) {
      std::cerr << "[Client] CORBA Communication Failure: " << ex << std::endl;
      std::cerr << "           Is the server running and reachable?" << std::endl;
      std::cerr << "           Is the NameService running and reachable via ORBInitRef?" << std::endl;
      return 1;
      }
   catch (CORBA::TRANSIENT const& ex) {
      std::cerr << "[Client] CORBA Transient Exception: " << ex << std::endl;
      std::cerr << "           The server might be starting up, shutting down, or busy." << std::endl;
      std::cerr << "           Try again later." << std::endl;
      return 1;
      }
   catch (CORBA::Exception const& ex) {
      std::println(std::cerr, "[Client] CORBA Exception: {}", toString(ex));
      return 1;
      }
   catch (std::exception const& stdex) {
      std::println(std::cerr, "[Client] Standard Exception: {}",stdex.what());
      return 1;
      }
   catch (...) {
      std::println(std::cerr, "[Client] Unknown exception caught.");
      return 1;
      }

   std::println(std::cout);
   std::println(std::cout, "[Client] Client exited gracefully.");

   if(empData) 
      std::cout << "\nvorher eingelesene Daten, lokal:\n"
                << "Mitarbeiter " << empData->personId << ": "
                << empData->firstName << " " << empData->name << ", "
                << "Gehalt: " << empData->salary << ", "
                << "Aktiv: " << (empData->isActive ? "Ja" : "Nein") << ", "
                << "Startdatum: " << empData->startDate.day << "."
                                  << empData->startDate.month << "." << empData->startDate.year
                << std::endl;

    return 0;
   }

 \endcode
 
 \author Volker Hillmann (adecc Systemhaus GmbH)
 \version 1.0
 \date 2025-05-15
 \copyright
 Copyright © 2020–2025 adecc Systemhaus GmbH  
 This program is free software: you can redistribute it and/or modify it  
 under the terms of the GNU General Public License, version 3.  
 See <https://www.gnu.org/licenses/>.

 \note This documentation is part of the adecc Scholar project —  
       Free educational materials for distributed systems in modern C++.

*/

/**
 \page ORBClientPage ORBClient Template Class
 \brief Description of the first version of client template with support of one stub
 \section ORBClient_Overview Overview

 The ORBClient template class provides a generic helper for CORBA clients to:

 - initialize the CORBA ORB system,
 - access the CORBA Naming Service,
 - and resolve a service stub (factory) for a specific CORBA interface type.

 This class encapsulates recurring logic to simplify and generalize the implementation
 of CORBA clients and promote reusability.

 \section ORBClient_TemplateParam Template Parameters

 \code{.cpp}
 template <CORBAStub corba_ty>
 \endcode

 - \c corba_ty: The interface type of the CORBA object, typically generated by the IDL compiler.
   The class uses the corresponding \c _var_type to ensure safe and automatic memory management.

 \section ORBClient_Construction Construction

 The constructor requires:

 - \c paName: A name for the ORB system (e.g., the process or application name)
 - \c argc, \c argv: Command line arguments to pass to the ORB
 - \c paService: The service name to be resolved via the Naming Service

 \section ORBClient_Workflow Workflow

 1. ORB initialization via ORBBase
 2. Construction of a name object (CosNaming::Name)
 3. Resolution through the Naming Service
 4. Narrowing to the expected interface type
 5. Logging of success or failure

 \section ORBClient_API Public API

 - \c corba_ty* operator() (): Returns the CORBA service reference (typically a factory)

 \section ORBClient_Code Implementation

 \code{.cpp}
template <CORBAStub corba_ty>
class ORBClient : virtual public ORBBase {
    using corba_var_ty = typename corba_ty::_var_type;

    std::string strService = ""s;
    corba_var_ty factory_ = nullptr;

public:
    ORBClient() = delete;

    ORBClient(std::string const& paName, int argc, char* argv[], std::string const& paService)
        : ORBBase(paName, argc, argv), strService{ paService } {

        CosNaming::Name_var name = new CosNaming::Name;
        name->length(1);
        name[0].id   = CORBA::string_dup(strService.c_str());
        name[0].kind = CORBA::string_dup("Object");

        log_trace<2>("[{} {}] Resolving {}.", Name(), ::getTimeStamp(), strService);

        CORBA::Object_var factory_obj = naming_context()->resolve(name);

        factory_ = typename corba_ty::_narrow(factory_obj.in());

        if (CORBA::is_nil(factory_.in()))
            throw std::runtime_error(std::format("Failed to narrow factory reference for {1:} in {0:}.",
                                                 Name(), strService));

        log_trace<2>("[{} {}] Successfully obtained reference for {}.", Name(), ::getTimeStamp(), strService);
    }

    ~ORBClient() {}

    ORBClient(ORBClient const&) = delete;
    ORBClient& operator = (ORBClient const&) = delete;

    corba_ty* operator() () {
        return factory_.in();
    }
};
 \endcode
 
 \see \ref appclient for the lifecircle of a typical corba client application
 
 \author Volker Hillmann (adecc Systemhaus GmbH)
 \version 1.0
 \date 2025-06-14
 \copyright
 Copyright © 2020–2025 adecc Systemhaus GmbH  
 This program is free software: you can redistribute it and/or modify it  
 under the terms of the GNU General Public License, version 3.  
 See <https://www.gnu.org/licenses/>.

 \note This documentation is part of the adecc Scholar project —  
       Free educational materials for distributed systems in modern C++.

*/