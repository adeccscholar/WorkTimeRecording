// SPDX-FileCopyrightText: 2025 adecc Systemhaus GmbH
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 \file
 \brief Doxygen Documentation Page for the CORBA Application Server

 \details
 \subpage appserver "CORBA Application Server – Overview"
 */
 
/**
 \page appserver CORBA Application Server – Overview
 \brief Overview page for implementation of an corba application server
 \section appserver_intro Introduction

 The CORBA Application Server is a core component of the distributed enterprise time-tracking system based on CORBA
 developed as part of the **adecc Scholar** educational initiative. It runs on **Ubuntu Linux** and serves
 as a central coordination and data hub in a heterogeneous CORBA-based network.
 
 It manages central components such as companies and provides servants to interact with external clients like time 
 terminals and administration tools.

 Key features:
  - Company servant accessible via CORBA Naming Service
  - Persistent and transient POA separation
  - Multi-threaded ORB event loop
  - Clean shutdown and resource handling

 See \ref app_lifecycle for a detailed flow of initialization and shutdown.

 \section appserver_arch Architecture and Function

 This Application Server fulfills multiple roles:

 - Acts as a **CORBA server** for GUI clients on Windows and Linux desktop systems  
 - Provides employee and time-tracking data by connecting to a Microsoft SQL Server  
 - Uses **ODBC** for database access via the **Qt6**-based wrapper `adecc_Database`
 - Exposes business logic (e.g. salary sums, active employees) through the CORBA interface `Organization::Company`
 - Acts as a **CORBA client** to communicate with embedded devices (e.g. Raspberry Pi-based terminals)
 - Controls and monitors terminals by activating/deactivating access control services remotely

 \section appserver_middleware Middleware and POA Configuration

 - Uses **TAO (The ACE ORB)** for CORBA communication
 - Relies on both **persistent** and **transient POAs**:
   - Persistent POA for long-lived objects like Company or Organization services
   - Transient POA for dynamically activated objects like temporary Employee servants
 - Integrates with the **CORBA Naming Service** for object discovery

 \section appserver_goals Educational Purpose

 The application server illustrates how to:
 - Build a robust, distributed backend system using **modern C++23**
 - Integrate platform-independent technologies (Linux, Windows, Embedded)
 - Use CORBA for distributed object communication and lifetime control
 - Interface with SQL databases in an enterprise environment
 - Bridge modern and legacy infrastructure components

 \section appserver_part Project Context

 This server is part of the open-source adecc Scholar project.
 It is designed to teach modern, real-world distributed application development using
 C++, CORBA/TAO, Qt, and SQL in a heterogeneous network of clients and servers.

 \author Volker Hillmann (adecc Systemhaus GmbH)
 \version 1.0
 \date 2025-05-14
 \copyright
 Copyright © 2020–2025 adecc Systemhaus GmbH  
 This program is free software: you can redistribute it and/or modify it  
 under the terms of the GNU General Public License, version 3.  
 See <https://www.gnu.org/licenses/>.

 \note This documentation is part of the adecc Scholar project —  
       Free educational materials for distributed systems in modern C++.
*/

/**
  \page app_lifecycle Application Lifecycle
  \brief Detailed breakdown of the server application's runtime behavior.
 
  This document describes the control flow of the server, including CORBA setup, POA configuration,
  servant creation, and shutdown handling.
 
  ### 1. Signal Handling
  - Registers signal handlers for graceful termination (SIGINT/SIGTERM).
 
  ### 2. ORB Initialization
  - Initializes the global ORB using command line arguments.
  - Handles and logs errors if the ORB is not created.
 
  ### 3. Root POA Acquisition
  - Resolves "RootPOA" reference and narrows it.
  - Activates its POA manager to allow request processing.
 
  ### 4. POA Policies
  - Defines a persistent POA (`CompanyPOA`) for long-lived servants.
  - Defines a transient POA (`EmployeePOA`) for short-lived objects.
 
  ### 5. Child POA Creation
  - Creates `CompanyPOA` and `EmployeePOA` as children of the root POA.
 
  ### 6. Company Servant Activation
  - Instantiates and activates a servant implementing the `Company` interface.
  - Obtains the object reference for use in the Naming Service.
 
  ### 7. Naming Service Registration
  - Registers the `Company` servant with the CORBA Naming Service under a known name.
 
  ### 8. ORB Event Loop
  - Starts the `orb->run()` loop in a background thread to handle client requests.
 
  ### 9. Shutdown Wait
  - Main thread blocks and periodically checks for shutdown signal.
 
  ### 10. Naming Service Unbind
  - Attempts to remove the object reference from the Naming Service.

  ### 11. Deactivate Company Servant and count ref down to deletr
  - Delete the servant object finally.
  
  ### 12. ORB Shutdown
  - Calls `shutdown(false)` to unblock the ORB thread.
 
  ### 13. POA Destruction
  - Destroys both child POAs and the RootPOA in order.
 
  ### 14. ORB Destruction
  - Finally, the ORB is destroyed to free all resources.
  
  # Sourcecode
  
Full Source of the first version with complete workcycle.  
  
\code{.cpp}
   int main(int argc, char *argv[]) {
      // (1) Setup: Install signal handlers for graceful shutdown on SIGINT or SIGTERM
      signal(SIGINT, signal_handler);
      signal(SIGTERM, signal_handler);

      const std::string strAppl = "Time Tracking App Server"s;
   #ifdef _WIN32
      SetConsoleOutputCP(CP_UTF8);  
   #endif

      // (2) Initialize global CORBA ORB
      std::println("[{} {}] Server starting ...", strAppl, ::getTimeStamp());
      CORBA::ORB_var orb_global = CORBA::ORB_init(argc, argv);
      try {
         if (CORBA::is_nil(orb_global.in())) throw std::runtime_error("Failed to initialized the global ORB Object.");
         std::println("[{} {}] Corba is intialized.", strAppl, ::getTimeStamp());

         // (3) Obtain RootPOA and activate POA Manager
         CORBA::Object_var poa_object = orb_global->resolve_initial_references("RootPOA");
         PortableServer::POA_var root_poa = PortableServer::POA::_narrow(poa_object.in());
         if (CORBA::is_nil(root_poa.in())) throw std::runtime_error("Failed to narrow the POA");

         PortableServer::POAManager_var poa_manager = root_poa->the_POAManager();
         poa_manager->activate();
         std::println("[{} {}] RootPOA obtained and POAManager is activated.", strAppl, ::getTimeStamp());

         // (4) Create policies for POAs
         CORBA::PolicyList comp_pol;
         comp_pol.length(1);
         comp_pol[0] = root_poa->create_lifespan_policy(PortableServer::PERSISTENT);

         CORBA::PolicyList empl_pol;
         empl_pol.length(2);
         empl_pol[0] = root_poa->create_lifespan_policy(PortableServer::TRANSIENT);
         empl_pol[1] = root_poa->create_servant_retention_policy(PortableServer::ServantRetentionPolicyValue::RETAIN);

         // (5) Create Child POAs
         PortableServer::POA_var company_poa  = root_poa->create_POA("CompanyPOA", poa_manager.in(), comp_pol);
         PortableServer::POA_var employee_poa = root_poa->create_POA("EmployeePOA", poa_manager.in(), empl_pol);

         for (uint32_t i = 0; i < comp_pol.length(); ++i) comp_pol[i]->destroy();
         for (uint32_t i = 0; i < empl_pol.length(); ++i) empl_pol[i]->destroy();

         std::println("[{} {}] Persistent CompanyPOA and transient EmployeePOA created.", strAppl, ::getTimeStamp());

         // (6) Create and activate the Company servant
         Company_i* company_servant = new Company_i(company_poa.in(), employee_poa.in());
         PortableServer::ObjectId_var company_oid = company_poa->activate_object(company_servant);

         CORBA::Object_var obj_ref = company_poa->id_to_reference(company_oid.in());
         Organization::Company_var company_ref = Organization::Company::_narrow(obj_ref.in());
         if (CORBA::is_nil(company_ref)) {
            throw std::runtime_error(std::format("CORBA Error while narrowing Reference."));
            }
         std::println("[{} {}] Company servant activate under CompanyPOA.", strAppl, ::getTimeStamp());

         // (7) Bind to Naming Service
         CORBA::Object_var naming_obj = orb_global->resolve_initial_references("NameService");
         CosNaming::NamingContext_var naming_context = CosNaming::NamingContext::_narrow(naming_obj.in());
         if (CORBA::is_nil(naming_context)) throw std::runtime_error("Failed to narrow naming context.");

         std::string strName = "GlobalCorp/CompanyService"s;
         CosNaming::Name name;
         name.length(1);
         name[0].id = CORBA::string_dup(strName.c_str());
         name[0].kind = CORBA::string_dup("Object");

         naming_context->rebind(name, company_ref.in());
         std::println("[{} {}] Company servant registered with nameservice as {}.", strAppl, ::getTimeStamp(), strName);

         // (8) Start ORB event loop in a separate thread
         std::thread orb_thread([&]() {
            std::string strOrb = "ORB Thread"s;
            try {
               orb_global->run();
               std::println("   [{} {}] orb->run() finished.", strAppl, ::getTimeStamp());
               }
            catch(CORBA::Exception const& ex) {
               std::println(std::cerr, "  [{} {}], CORBA Exception in orb->run(): {}", strOrb, ::getTimeStamp(), toString(ex));
               }
            catch (std::exception const& ex) {
               std::println(std::cerr, "  [{} {}], CORBA Exception in orb->run(): {}", strOrb, ::getTimeStamp(), ex.what());
               }
            catch (...) {
               std::println(std::cerr, "  [{} {}], unknown Exception in orb->run()", strOrb, ::getTimeStamp());
               }
            } );

         // (9) Wait for shutdown signal
         std::println("[{} {}] Server is ready. <Waiting for shutdown signal (e.g. Cntrl+C) ...", strAppl, ::getTimeStamp());
         while (!shutdown_requested) { 
            std::this_thread::sleep_for(std::chrono::milliseconds(200));  
            }

         // (10) Cleanup: unbind nameservice
         std::println("[{} {}] Unbinding from nameservice ...", strAppl, ::getTimeStamp());
         try {
            naming_context->unbind(name);
            }
         catch(CORBA::Exception const& ex) {
            std::println(std::cerr, "[{} {}] Error unbinding from nameservice: {}", strAppl, ::getTimeStamp(), toString(ex));
            }

         // (11) Cleanup: deactivate servant
         std::println("[{} {}] Deactivate servant and remove refcount()...", strAppl, ::getTimeStamp());
         company_poa->deactivate_object(company_oid);
         company_servant->_remove_ref();

         // (12) Shutdown ORB
         std::println("[{} {}] Shutdown requested, calling orb->shutdown(false)...", strAppl, ::getTimeStamp());
         orb_global->shutdown(true);

         if (orb_thread.joinable()) orb_thread.join();

         // (13) Destroy POAs
         std::println("[{} {}] Destroying POAs ...", strAppl, ::getTimeStamp());
         employee_poa->destroy(true, true);
         company_poa->destroy(true, true);
         root_poa->destroy(true, true);
         std::println("[{} {}] POAs destroyed.", strAppl, ::getTimeStamp());
         }
      catch(CORBA::Exception const& ex) {
         std::println(std::cerr, "[{} {}] CORBA Exception caught: {}", strAppl, ::getTimeStamp(), toString(ex));
         }
      catch(std::exception const& ex) {
         std::println(std::cerr, "[{} {}] std::exception caught: {}", strAppl, ::getTimeStamp(), ex.what());
         }
      catch (...) {
         std::println(std::cerr, "[{} {}] Unknown exception caught.", strAppl, ::getTimeStamp());
         }

      // (14) Destroy ORB
      if(!CORBA::is_nil(orb_global.in())) {
         try {
            std::println(std::cout, "[{} {}] Destroying ORB ...", strAppl, ::getTimeStamp());
            orb_global->destroy();
            std::println(std::cout, "[{} {}] ORB destroyed.", strAppl, ::getTimeStamp());
            }
         catch(CORBA::Exception const& ex) {
            std::println(std::cerr, "[{} {}] Exception destroying ORB: {}", strAppl, ::getTimeStamp(), toString(ex));
            }
         }
      std::println(std::cout, "[{} {}] Server exited successfully.", strAppl, ::getTimeStamp());
      return 0;
      }
\endcode
  
 */
