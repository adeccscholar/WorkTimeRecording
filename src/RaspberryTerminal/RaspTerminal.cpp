
#include "Tools.h"
#include "OrganizationC.h"

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
   return 0;
   }