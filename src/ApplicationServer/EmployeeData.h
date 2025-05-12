
#include "OrganizationC.h"

#include <tao/ORB_Core.h>

#include <string>

using namespace std::string_literals;

struct PersonData {
   CORBA::Long personID   = -1;
   std::string firstname  = ""s;
   std::string name       = ""s;
   Organization::EGender gender = Organization::OTHER;
   };

struct EmployeeData {
   CORBA::Long personID                 = -1;
   std::string firstname                = ""s;
   std::string name                     = ""s;
   Organization::EGender gender         = Organization::OTHER;
   double salary                        = 0.0;
   Organization::YearMonthDay startDate = { 0, 0, 0 };
   CORBA::Boolean isActive              = false;
   };