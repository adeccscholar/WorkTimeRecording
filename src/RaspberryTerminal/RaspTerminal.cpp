
#include "Sensoring.h"

#include "Tools.h"

#include "OrganizationC.h"
#include "Corba_Interfaces.h"

#include <tao/corba.h>
#include <orbsvcs/CosNamingC.h>

#include <iostream>
#include <print>

#ifdef _WIN32
#include <Windows.h>
#endif

int main(int argc, char* argv[]) {
   const std::string strAppl = "Time Terminal"s;
#ifdef _WIN32
   SetConsoleOutputCP(CP_UTF8);
#endif
   std::println("Time Tracking Terminal with Raspberry PI");
   
   SensorReading sensor_reading;

   sensor_reading.initExternDisplay();
   sensor_reading.readSensors();

   TimeTracking tracking(sensor_reading);
   tracking.Init();
   tracking.Test_LEDs();


   try {
      /*
      CORBAClient<Organization::Company> factories("CORBA Factories", argc, argv, "GlobalCorp/CompanyService"s);
      auto company = [&factories]() { return factories.get<0>();  };
      auto empl = make_destroyable(company()->getEmployee(105));
      std::println(std::cout, "ID: {:>4}, Name: {:<25}, Status: {:<3}, Salary: {:>10.2f}", 
                 empl->personId(), toString(empl->getFullName()), (empl->isActive() ? "Yes" : "No"), empl->salary());
      */
      }
   catch (Organization::EmployeeNotFound const& ex) {
      // Safety net, in case the exception occurs outside the specific try-catch block
      log_error("[{} {}] unhandled 'EmployNotFound'- Exception with Employee ID: {} at {}.",
         strAppl, ::getTimeStamp(), ex.requestedId, getTimeStamp(convertTo(ex.requestedAt)));
      return 1;
      }
   catch (CORBA::COMM_FAILURE const& ex) {
      log_error("[{} {}] {}", strAppl, ::getTimeStamp(), toString(ex));
      return 2;
      }
   catch (CORBA::TRANSIENT const& ex) {
      log_error("[{} {}] {}", strAppl, ::getTimeStamp(), toString(ex));
      return 3;
      }
   catch (CORBA::Exception const& ex) {
      log_error("[{} {}] {}", strAppl, ::getTimeStamp(), toString(ex));
      return 4;
      }
   catch (std::exception const& ex) {
      log_error("[{} {}] C++ Standard Exception: {}", strAppl, ::getTimeStamp(), ex.what());
      }
   catch (...) {
      log_error("[{} {}] unknown exception caught, critical error.", strAppl, ::getTimeStamp());
      return 101;
      }

   return 0;
   }