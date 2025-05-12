#pragma once

#include "OrganizationS.h" // Skeleton Header

#include "EmployeeData.h"

#include <tao/ORB_Core.h>
#include <tao/PortableServer/PortableServer.h>

class Employee_i : public virtual PortableServer::RefCountServantBase,
   public virtual POA_Organization::Employee {
private:
   EmployeeData                data_;

   PortableServer::POA_var      poa_;
   PortableServer::ObjectId_var oid_;

public:
   Employee_i(EmployeeData const& data, PortableServer::POA_ptr poa);
   virtual ~Employee_i();

   // IDL Attribute
   virtual CORBA::Long personId() override;
   virtual char* firstName() override;
   virtual char* name() override;
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
};
