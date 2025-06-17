/**
  \file
  \brief CORBA client-side utility functions for accessing employee data.

  \details
  This file defines a set of helper functions to interact with the Organization module
  of the CORBA server. It includes logic for retrieving and displaying single employees
  or full lists, formatting timestamps, and logging diagnostic information.
  The functions use TAO's CORBA implementation and follow RAII and logging standards
  established in the adecc Scholar project.

  \author Volker Hillmann (adecc Systemhaus GmbH)

  \copyright Copyright © adecc Systemhaus GmbH 2021–2025

  \license This program is free software: you can redistribute it and/or modify it
           under the terms of the GNU General Public License, version 3,
           as published by the Free Software Foundation.

           This program is distributed in the hope that it will be useful,
           but WITHOUT ANY WARRANTY; without even the implied warranty of
           MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
           See the GNU General Public License for more details.

           You should have received a copy of the GNU General Public License
           along with this program. If not, see <https://www.gnu.org/licenses/>.

  \version 1.0
  \date 2025-06-01
 */

#pragma once

#include "Tools.h"
#include "my_logging.h"

#include "Corba_Interfaces.h"

#include <OrganizationC.h>

#include <orbsvcs/CosNamingC.h>
#include <tao/corba.h>

#include <format>
#include <iostream>
#include <print>
#include <stdexcept>
#include <string>

using namespace std::string_literals;


/**
 \brief Retrieves the current timestamp from a given Company object.

 \param comp_in Pointer to the CORBA Company interface.
 \return A timestamp string retrieved from the company server object.
 \throws std::runtime_error if the company reference is nil.
 */
inline std::string getTimeStamp(Organization::Company_ptr comp_in) {
	if (!CORBA::is_nil(comp_in)) [[likely]] {
		return getTimeStamp(convertTo(comp_in->getTimeStamp()));
	   } 
	else
		throw std::runtime_error("company is nil");
   }

/**
 \brief Displays key information for a given employee object.

 \param out Output stream to write the formatted data to.
 \param employee CORBA employee object to show.
 \throws std::runtime_error if the employee reference is nil.
 */
inline void ShowEmployee(std::ostream& out, Organization::Employee_ptr employee) {
	// if (!CORBA::is_nil(employee)) [[likely]] {
	// CORBA::String_var fullName = employee->getFullName();   // fullName.in()
	// CORBA::Boolean active = employee->isActive();
	std::println(out, "ID: {:>4}, Name: {:<25}, Status: {:<3}, Salary: {:>10.2f}", employee->personId(), toString(employee->getFullName()),
	             (employee->isActive() ? "Yes" : "No"), employee->salary());
	//   }
	// else {
	//   throw std::runtime_error(std::format("[ShowEmployee: {}] ERROR: employee is nil unexpectedly.", ::getTimeStamp()));
	//   }
}

/**
 \brief Requests and displays a single employee by ID from the Company object.

 \param comp_in Company CORBA object to query.
 \param seekId Numeric ID of the employee to retrieve.

 \note Handles and logs CORBA exceptions, including EmployeeNotFound.
 */
inline void GetEmployee(Organization::Company_ptr comp_in, CORBA::Long seekId) {
	static const std::string strScope = "GetEmployee()"s;
	log_trace<2>("[{} {}] Requesting employee with ID: {}", strScope, getTimeStamp(comp_in), seekId);
	// Organization::Employee_var employee_var = Organization::Employee::_nil(); // outside declared
	try {
		log_trace<3>("[{} {}]  Entering scope for Employee_var (ID: {}) ...", strScope, getTimeStamp(comp_in), seekId);
		auto employee_var = make_destroyable(comp_in->getEmployee(seekId));
		ShowEmployee(std::cout, employee_var.in());
		log_trace<3>("[{} {}] Leaving scope for Employee_var (ID: {}), Reference released.", strScope, getTimeStamp(comp_in), seekId);
	   } 
	catch (Organization::EmployeeNotFound const& ex) {
		log_error("[{} {}] ERROR: Caught EmployeeNotFound for ID {}", strScope, getTimeStamp(comp_in), ex.requestedId);
	   } 
	catch (CORBA::Exception const& ex) {
		log_error("[{} {}] ERROR: CORBA Exception during getEmployee({}): {}", strScope, getTimeStamp(comp_in), seekId, toString(ex));
	   } 
	catch (std::exception const& ex) {
		log_error("[{} {}] ERROR: C++ Exception during getEmployee({}): {}", strScope, getTimeStamp(comp_in), seekId, ex.what());
	   } 
	catch (...) {
		log_error("[{} {}] CRITICAL ERROR: unexpected Exception during getEmployee({})", strScope, getTimeStamp(comp_in), seekId);
	   }

	// HIER zerstören wir employee_var, so dass die Referenz auf dem Server freigegeben wird.
	// Server zerstört den transienten Servant.
	// if (!CORBA::is_nil(employee_var.in())) {
	//   employee_var->destroy();
	//   employee_var = Organization::Employee::_nil();
	//   }
   }


/*
template<typename T>
concept CORBAStubWithDestroySequenceVar =
	requires {
	typename T::T_elem;
	typename T::T_elem::value_type; // Das ist der tatsächliche `T*`, also Employee*
		requires CORBAStubWithDestroy<std::remove_pointer_t<typename T::T_elem::value_type>>;
};


inline std::vector<Destroyable_Var<Organization::Employee>> move_from_sequence(Organization::EmployeeSeq_var&& seq) {
	std::vector<Destroyable_Var<Organization::Employee>> result;
	result.reserve(seq->length());
	for (CORBA::ULong i = 0; i < seq->length(); ++i) {
		result.emplace_back(std::move(make_destroyable(Organization::Employee::_duplicate(seq[i].in()))));
		}
	seq->length(0);
	return result;
   }
*/

// Generic (not defined = compiler error if not specialized)
template<CORBAStubWithDestroy Stub>
struct CorbaSequenceForStubWithDestroy;

template<>
struct CorbaSequenceForStubWithDestroy<Organization::Person> {
	using type = Organization::PersonSeq;
   };

// Specializations for each known CORBA stub
template<>
struct CorbaSequenceForStubWithDestroy<Organization::Employee> {
	using type = Organization::EmployeeSeq;
   };

template<CORBAStubWithDestroy Stub>
using CorbaSequenceForStubWithDestroy_t = typename CorbaSequenceForStubWithDestroy<Stub>::type;

/**
  \brief Transfers ownership of CORBA sequence elements into a std::vector.
 
  \details This template function accepts a CORBA sequence of `_var_type` elements (e.g. `EmployeeSeq_var`)
           and transfers the ownership of all valid object references into a vector of `Destroyable_Var<>`.
 
           Internally, each element is duplicated using `_duplicate()` to ensure that reference counting is correct.
           Afterwards, the original sequence is cleared via `length(0)` to relinquish ownership and avoid double-deletion.
 
  \tparam corba_ty The CORBA stub type for which this sequence is defined (must satisfy CORBAStubWithDestroy).
  \param seq A move reference to the CORBA sequence (`_var_type`) whose elements should be extracted.
  \return std::vector of `Destroyable_Var` holding the duplicated elements from the sequence.
 
  \note This function assumes that the caller no longer needs the original sequence and ensures
        that the resulting vector is the sole owner of the extracted elements.
 
  \see CORBAStubWithDestroy
  \see make_destroyable
 */
template <CORBAStubWithDestroy corba_ty>
std::vector<Destroyable_Var<corba_ty>> move_from_sequence(typename CorbaSequenceForStubWithDestroy_t<corba_ty>::_var_type && seq) {
	std::vector<Destroyable_Var<corba_ty>> result;
	result.reserve(seq->length());

	for (CORBA::ULong i = 0; i < seq->length(); ++i) {
		result.emplace_back(make_destroyable(corba_ty::_duplicate(seq[i].in())));
	   }

	seq->length(0); // Transfer der Ownership durch Leeren der Sequenz
	return result;
   }

/**
 \brief Retrieves and prints the list of all employees from the company.

 \param comp_in Company CORBA object providing the employee sequence.

 \note The function cleans up the returned references by calling \c destroy() on each.
 */
inline void GetEmployees(Organization::Company_ptr comp_in) {
	static const std::string strScope = "GetEmployees()"s;
	log_trace<2>("[{} {}] Requesting employees.", strScope, getTimeStamp(comp_in));

	auto values = move_from_sequence<Organization::Employee>(Organization::EmployeeSeq_var { comp_in->getEmployees() });
	//auto values = move_from_sequence<Organization::Employee>( comp_in->getEmployees() );
	std::println(std::cout, "[{} {}] Received sequence with {} employee references.", strScope, getTimeStamp(comp_in), values.size());
	for(auto const& value : values) ShowEmployee(std::cout, value.get());

	/*
	Organization::EmployeeSeq_var employees_seq = comp_in->getEmployees();
	std::println(std::cout, "[{} {}] Received sequence with {} employee references.", strScope, getTimeStamp(comp_in),
					 employees_seq->length());

	for (CORBA::ULong i = 0; i < employees_seq->length(); ++i) {
		Organization::Employee_ptr current_employee = employees_seq[i];
		if (CORBA::is_nil(current_employee)) {
			log_error("[{} {}] WARNING: Nil reference encountered in sequence at index {}", strScope, getTimeStamp(comp_in), i);
			continue;
	   	}

		try {
			std::print(std::cout, "  Employee [{:>3}] - ", i);
			ShowEmployee(std::cout, current_employee);

		   } 
		catch (CORBA::Exception const& ex) {
			log_error("[{} {}] Error accessing employee data at index {}: {}", strScope, getTimeStamp(comp_in), i, toString(ex));
		   }
	   }

	std::println(std::cout, "[Client] End of scope for EmployeeSeq_var. Releasing references in sequence...");

	for (CORBA::ULong i = 0; i < employees_seq->length(); ++i) {
		if (!CORBA::is_nil(employees_seq[i].in())) {
			employees_seq[i]->destroy();
			employees_seq[i] = Organization::Employee::_nil();
		   }
	   }
	*/
   }
