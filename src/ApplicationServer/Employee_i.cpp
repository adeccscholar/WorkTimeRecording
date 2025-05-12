#include "Employee_i.h"

#include <iostream>
#include <print>
#include <stdexcept>


Employee_i::Employee_i(EmployeeData const& data, PortableServer::POA_ptr poa) : 
                    data_(data), poa_(PortableServer::POA::_duplicate((poa))) {
   std::println(std::cout, "[Employee_i ] Object created for id: {}", personId());
   }

Employee_i::~Employee_i() {
   std::println(std::cout, "[Employee_i ] Object destroyed for id: {}", personId());
   }

CORBA::Long Employee_i::personId() {
   return data_.personID;
   }

char* Employee_i::firstName() {
   return CORBA::string_dup(data_.firstname.c_str());
   }

char* Employee_i::name() {
   return CORBA::string_dup(data_.name.c_str());
   }

/*


   // IDL Attribute
   virtual Organization::EGender gender() override;

   // IDL Operation
   virtual char* getFullName() override;

   // IDL Attribute von Employee
   virtual CORBA::Double salary() override;
   virtual Organization::YearMonthDay startDate() override;
   virtual CORBA::Boolean isActive() override;

   // helper to set oid fpr concrete object
   void set_oid(PortableServer::ObjectId const& oid);

   void destroy() override;
*/