
#include "Tools.h"

#include <OrganizationC.h>

#include <tao/corba.h>
#include <orbsvcs/CosNamingC.h>

#include <iostream>
#include <string>
#include <vector>
#include <stdexcept>
#include <chrono>
#include <thread>
#include <print>

using namespace std::string_literals;

#ifdef _WIN32
#include <Windows.h>
#endif


int main(int argc, char *argv[]) {
   const std::string strMainClient = "Client"s;
#ifdef _WIN32
   SetConsoleOutputCP(CP_UTF8);
#endif
   // Platzhalter für Variable
   std::println(std::cout, "[{} {}] Client Testprogram for Worktime Tracking started.", strMainClient, ::getTimeStamp());
   try {
      // ORBClient<Organization::Company> orb("Client"s, argc, argv, "GlobalCorp/CompanyService"s);

      // 1st ORB initialisieren
      CORBA::ORB_var orb = CORBA::ORB_init(argc, argv);
      std::println(std::cout, "[{} {}] ORB initialized.", strMainClient, ::getTimeStamp());
      
      // 2nd connecting to nameservice
      CORBA::Object_var naming_obj = orb->resolve_initial_references("NameService");
      CosNaming::NamingContext_var naming_context = CosNaming::NamingContext::_narrow(naming_obj.in());
      if (CORBA::is_nil(naming_context.in())) throw std::runtime_error("Failed to narrow Naming Context.");
      std::println(std::cout, "[{} {}] Naming Service Context obtained.", strMainClient, ::getTimeStamp());

      // 3rd Company Object
      std::string strName = "GlobalCorp/CompanyService"s;
      CosNaming::Name name;
      name.length(1);
      name[0].id = CORBA::string_dup(strName.c_str());
      name[0].kind = CORBA::string_dup("Object");

      std::println(std::cout, "[{} {}] Resolving {}.", strMainClient, ::getTimeStamp(), strName);
      CORBA::Object_var company_obj = naming_context->resolve(name);
      Organization::Company_var company_var = Organization::Company::_narrow(company_obj.in());
      if (CORBA::is_nil(company_var.in())) throw std::runtime_error("Failed to narrow Company reference.");
      std::println(std::cout, "[{} {}] Successfully obtained reference to company {}.", strMainClient, ::getTimeStamp(), strName);

      // 4th using the company reference
      std::println(std::cout, "Company {}, to paid salaries {:.2f}", company_var->nameCompany(), company_var->getSumSalary());

      Organization::Employee_var employee = company_var->getEmployee(180);
      // Problematisch, überspringt Freigabe

      // ----------------------------------------------------------------------------------------------------------------------

      while (orb->work_pending()) orb->perform_work();
      orb->destroy();
      std::println(std::cout, "[{} {}] ORB destroyed.", strMainClient, ::getTimeStamp());
      }
   catch(Organization::EmployeeNotFound const& ex) {
      // Notleine falls Exception vorher nicht behandelt wurde
      std::println(std::cerr, "[{} {}] unhandled EmployNotFound Exception: ID: {} at {}.", 
              strMainClient, ::getTimeStamp(), ex.requestedId, getTimeStamp(convertTo(ex.requestedAt)));
      return 1;
      }
   catch(CORBA::COMM_FAILURE const& ex) {
      std::println(std::cerr, "[{} {}] {}", strMainClient, ::getTimeStamp(), toString(ex));
      return 2;
      }
   catch(CORBA::TRANSIENT const& ex) {
      std::println(std::cerr, "[{} {}] {}", strMainClient, ::getTimeStamp(), toString(ex));
      return 3;
      }
   catch(CORBA::Exception const& ex) {
      std::println(std::cerr, "[{} {}] {}", strMainClient, ::getTimeStamp(), toString(ex));
      return 4;
      }
   catch(std::exception const& ex) {
      std::println(std::cerr, "[{} {}] C++ Standard Exception: {}", strMainClient, ::getTimeStamp(), ex.what());
      }
   catch(...) {
      std::println(std::cerr, "[{} {}] unknown exception caught, critical error.", strMainClient, ::getTimeStamp());
      return 101;
      }


   std::println(std::cout, "[{} {}] Client exited gracefully.", strMainClient, ::getTimeStamp());

   // Platzhalter für verzögerte Ausgabe
   return 0;
    }