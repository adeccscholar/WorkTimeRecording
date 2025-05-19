// SPDX-FileCopyrightText: 2025 adecc Systemhaus GmbH
// SPDX-License-Identifier: GPL-3.0-or-later

/**
  \file 
  \brief Entry point for the application server implementation for the distributed time-tracking system 
 
  \details This project is developed in the adecc Scholar project as free learning project. Its
           demonstrates the development of a distributed, cross-platform  time-tracking system for enterprise 
           environments. It is part of the open-source educational initiative **adecc Scholar**, which began in 
           autumn 2020 during the COVID-19 pandemic.
 
  \details The goal is to teach modern C++ techniques (including C++23) in conjunction with open-source technologies 
           such as Boost, CORBA/TAO, Qt6, and embedded programming on Raspberry Pi.
 
  \details The system consists of several components:
            - Application Server (Ubuntu Linux, CORBA/ODBC)
            - Database Server (Windows Server with MS SQL)
            - Desktop Clients (Qt6-based UI)
            - Raspberry Pi Clients (RFID, sensors, GPIO, I2C, SPI)
 
  \details Communication is handled via CORBA (TAO); finite state machines are implemented using Boost.SML;
           GUI components are created with Qt6. Development is done in Visual Studio.
 
  \note The project also supports Embarcadero C++ Builder (VCL, FMX) to illustrate integration of legacy systems 
        into modern distributed architectures.
 
  \author Volker Hillmann (adecc Systemhaus GmbH)
 
  \copyright Copyright © adecc Systemhaus GmbH 2021–2025
 
  \license This project is mostly licensed under the GNU General Public License v3.0.
           For more information, see the LICENSE file.
 
  \version 1.0
  \date    09.05.2025
 */


// later delete stubs from directory and make this to standard include

#include "Tools.h"

#include "OrganizationC.h"

#include "Company_i.h"

#include <tao/corba.h>
#include <tao/PortableServer/PortableServer.h>
#include <tao/PortableServer/LifespanPolicyC.h>
#include <tao/PortableServer/LifespanPolicyA.h>

#include <orbsvcs/CosNamingC.h>

#include <iostream>
#include <sstream>
#include <string>
#include <chrono>
#include <thread>
#include <atomic>
#include <format>
#include <print>

#include <csignal>

#ifdef _WIN32
   #include <Windows.h>
#endif

using namespace std::string_literals;

/**
  \brief Indicates whether a shutdown has been requested.
 
  \details This atomic boolean flag is set to `true` by the signal handler when a termination signal is received.
           It can be safely read from multiple threads to check whether the application should initiate a graceful shutdown.
 
  \note This flag is intended to be polled in long-running loops or services that need to respond to external termination requests.
 
  \see signal_handler
 */
std::atomic<bool> shutdown_requested = false;


/**
  \brief Handles incoming POSIX signals and triggers a graceful shutdown.
 
  \details This function is designed to be registered as a signal handler (e.g., via `std::signal`). 
           Upon receiving a signal, it logs the reception and sets a global shutdown flag (`shutdown_requested`) 
           to `true`, signaling the application to begin a graceful shutdown procedure.
 
  \param sig_num The signal number received (e.g., SIGINT, SIGTERM).
 
  \return None.
 
  \pre  This function must be registered using `std::signal` or an equivalent mechanism.
  \post The global variable `shutdown_requested` will be set to `true`.
 
  \note This function writes directly to `std::cout` using `std::println`. It is **not** async-signal-safe,
        and should be used only in controlled environments (e.g., small CLI tools) where this is acceptable.
 
  \warning Avoid calling non-signal-safe functions in production signal handlers; for critical applications,
           use safer constructs like `sig_atomic_t` and defer processing to the main loop.
 
  \see shutdown_requested
  \see std::signal
 */
void signal_handler(int sig_num) {
   std::println(std::cout);
   std::println(std::cout, "[signal handler {}] signal {} received. Requesting shutdown ...", getTimeStamp(), sig_num);
   shutdown_requested = true;
   }

int main(int argc, char *argv[]) {

   // -----------------------------------------------------------------------------------
   // (1) Setup: Install signal handlers for graceful shutdown on SIGINT or SIGTERM
   // -----------------------------------------------------------------------------------
   signal(SIGINT, signal_handler);
   signal(SIGTERM, signal_handler);

   const std::string strAppl = "Time Tracking App Server"s;
#ifdef _WIN32
   SetConsoleOutputCP(CP_UTF8);  
#endif

   // ------------------------------------------------------------------------------------
   // (2) Initialize global CORBA ORB (Object Request Broker)
   // Note: The parameters will be assimilated (ownership is transferred).
   // ------------------------------------------------------------------------------------
   std::println("[{} {}] Server starting ...", strAppl, ::getTimeStamp());
   CORBA::ORB_var orb_global = CORBA::ORB_init(argc, argv);
   try {
      if (CORBA::is_nil(orb_global.in())) throw std::runtime_error("Failed to initialized the global ORB Object.");
      std::println("[{} {}] Corba is intialized.", strAppl, ::getTimeStamp());

      // ----------------------------------------------------------------------------------
      // (3) Obtain the RootPOA and activate its POA Manager (Portable Object Adapter)
      // ----------------------------------------------------------------------------------
      
      CORBA::Object_var poa_object = orb_global->resolve_initial_references("RootPOA");
      PortableServer::POA_var root_poa = PortableServer::POA::_narrow(poa_object.in());
      if (CORBA::is_nil(root_poa.in())) throw std::runtime_error("Failed to narrow the POA");

      PortableServer::POAManager_var poa_manager = root_poa->the_POAManager();
      poa_manager->activate();
      std::println("[{} {}] RootPOA obtained and POAManager is activated.", strAppl, ::getTimeStamp());

      // -------------------------------------------------------------------------------------------------------
      // (4) Create policies for both POAs (persistent, transient)
      // -------------------------------------------------------------------------------------------------------
      
      // Policy for the persistent POA (for Company / Organization)
      // PERSISTENT: Object references are independent of a single ORB runtime 
      // - they can refer to the same servant again later (after a restart) (impl.repository or mapping required).
      // Long - term services, “always - on” components, enterprise architectures with restart / failover.
      // IOR Length: approx. 250 - 400 bytes, depending on the length of the adapter and repository name.
      CORBA::PolicyList comp_pol;
      comp_pol.length(1);
      comp_pol[0] = root_poa->create_lifespan_policy(PortableServer::PERSISTENT);

      // Policy for the transient POA (for Employee)
      // TRANSIENT: Object references are only valid for the duration of the ORB instance. 
      // Server restart → References invalid
      // Short-lived objects, in-memory services
      // IOR Length: approx. 200  - 250 Bytes
      CORBA::PolicyList empl_pol;
      empl_pol.length(2);
      empl_pol[0] = root_poa->create_lifespan_policy(PortableServer::TRANSIENT);
      empl_pol[1] = root_poa->create_servant_retention_policy(PortableServer::ServantRetentionPolicyValue::RETAIN);

      // -------------------------------------------------------------------------------------------------------
      // (5) Create both Child POAs with the configured polices
      // -------------------------------------------------------------------------------------------------------
      PortableServer::POA_var company_poa  = root_poa->create_POA("CompanyPOA", poa_manager.in(), comp_pol);
      PortableServer::POA_var employee_poa = root_poa->create_POA("EmployeePOA", poa_manager.in(), empl_pol);

      for (uint32_t i = 0; i < 1; ++i) comp_pol[i]->destroy();
      for (uint32_t i = 0; i < 2; ++i) empl_pol[i]->destroy();

      std::println("[{} {}] Persistent CompanyPOA and transient EmployeePOA created.", strAppl, ::getTimeStamp());

      // -------------------------------------------------------------------------------------------------------
      // (6) Create and activate the Company servant object
      // -------------------------------------------------------------------------------------------------------
      // the live time is depentend of the orb and poa, but I know the live time of servant
      // auto company_servant = std::make_unique<Company_i>(company_poa.in(), employee_poa.in());
      Company_i* company_servant = new Company_i(company_poa.in(), employee_poa.in());
      PortableServer::ObjectId_var company_oid = company_poa->activate_object(company_servant);

      CORBA::Object_var obj_ref = company_poa->id_to_reference(company_oid.in());
      Organization::Company_var company_ref = Organization::Company::_narrow(obj_ref.in());
      if (CORBA::is_nil(company_ref)) {
         throw std::runtime_error(std::format("CORBA Error while narrowing Reference."));
         }
      std::println("[{} {}] Company servant activate under CompanyPOA.", strAppl, ::getTimeStamp());

      // --------------------------------------------------------------------------------
      // (7) Bind the Company servant to the CORBA Naming Service
      // --------------------------------------------------------------------------------
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

      // --------------------
      // end of stream, new 
      
      // --------------------------------------------------------------------------------
      // (8) Launch the ORB event loop in a separate thread
      // --------------------------------------------------------------------------------

      std::println(std::cout, "[{} {}] Starting ORB event loop in a separate thread...", strAppl, ::getTimeStamp());

      std::thread orb_thread([&]() {
         try {
            orb_global->run();
            std::println(std::cout, "[ORB Thread {}] orb->run() finished.", ::getTimeStamp());
            }
         catch (CORBA::Exception const& ex) {
            std::println(std::cerr, "[ORB Thread {}] CORBA Exception in orb->run(): ", ::getTimeStamp(), toString(ex));
            }
         catch(std::exception const& ex) {
            std::println(std::cerr, "[ORB Thread {}] C++ Exception in orb->run(): ", ::getTimeStamp(), ex.what());
            }
         catch (...) {
            std::println(std::cerr, "[ORB Thread {}] Unknown exception in orb->run().", ::getTimeStamp());
            }
         });

      // --------------------------------------------------------------------------------
      // (9) Wait for shutdown signal in the main thread
      // --------------------------------------------------------------------------------
      std::println(std::cout, "[{} {}] Server ready. Waiting for shutdown signal (e.g., Ctrl+C)...", strAppl, ::getTimeStamp());

      while (!shutdown_requested) {
         std::this_thread::sleep_for(std::chrono::milliseconds(200));
         }

      // --------------------------------------------------------------------------------
      // (10) Cleanup: Unbind from Naming Service
      // --------------------------------------------------------------------------------
      std::println(std::cout, "[{} {}] Unbinding from NameService...", strAppl, ::getTimeStamp());
      try {
         naming_context->unbind(name);
         }
      catch (CORBA::Exception const& ex) {
         std::println(std::cerr, "[{} {}] Error unbinding from NameService (maybe already gone): {}", strAppl, ::getTimeStamp(), toString(ex));
         }

      // --------------------------------------------------------------------------------
      // (11) Deactivate servant and remove refcount (= 0) to delete servant
      // --------------------------------------------------------------------------------
      std::println(std::cout, "[{} {}] Deactivate servant and remove refcount...", strAppl, ::getTimeStamp());
      company_poa->deactivate_object(company_oid);
      company_servant->_remove_ref();
      
      // --------------------------------------------------------------------------------
      // (12) Shutdown the ORB to stop event processing
      // --------------------------------------------------------------------------------
      std::println(std::cout, "[{} {}] Shutdown requested, calling orb->shutdown(false) ...", strAppl, ::getTimeStamp());
      orb_global->shutdown(false); // false = nicht blockieren, beende aber sofort run()

      // ORB wurde gestoppt, warte auf Thread-Ende ===
      if (orb_thread.joinable()) {
         orb_thread.join();
         }

      // --------------------------------------------------------------------------------
      // (13) Destroy POAs (first children, then root)
      // wait for running requests, clear objects
      // --------------------------------------------------------------------------------
      std::println(std::cout, "[{} {}] Destroying POAs...", strAppl, ::getTimeStamp());
      employee_poa->destroy(true, true);
      company_poa->destroy(true, true);

      root_poa->destroy(true, true);
      std::println(std::cout, "[{} {}] POAs destroyed.", strAppl, ::getTimeStamp());

      // -----------
      }
   catch(CORBA::Exception const& ex) {
      std::println(std::cerr, "[{} {}] CORBA Exception caught: {}", strAppl, ::getTimeStamp(), toString(ex));
      }
   catch(std::exception const& ex) {
      std::println(std::cerr, "[{} {}] std::exception caught: {}", strAppl, ::getTimeStamp(), ex.what());
      }
   catch (...) {
      std::println(std::cerr, "[{} {}] Unknown exception caught.", strAppl, ::getTimeStamp());
      return 1;
      }
   // -----------------------------------------------------------------------------------------------------------------

   if (!CORBA::is_nil(orb_global.in())) {
      try {
         std::println(std::cout, "[{} {}] Destroying ORB...", strAppl, ::getTimeStamp());
         orb_global->destroy();
         std::println(std::cout, "[{} {}] ORB destroyed.", strAppl, ::getTimeStamp());
         }
      catch (CORBA::Exception const& ex) {
         std::println(std::cerr, "[{} {}] Exception while destroying ORB: {}", strAppl, ::getTimeStamp(), toString(ex));
         }
      }

   std::println(std::cout, "[{} {}] Server exited successfully.", strAppl, ::getTimeStamp());

   // ------------------------------------------------------------------------------------------------------------------
   return 0;
   }
