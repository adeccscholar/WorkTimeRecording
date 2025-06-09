// SPDX-FileCopyrightText: 2025 adecc Systemhaus GmbH
// SPDX-License-Identifier: GPL-3.0-or-later

/**
  \file
  \brief Implementation of Basics::DestroyableInterface servant.
 
  \details This file contains the implementation of `DestroyableInterface_i`, a base class
           used in TAO CORBA applications to manage the lifecycle of transient server objects.
           The servant supports explicit destruction via the `destroy()` operation and handles
           POA deregistration internally.
 
  \note This implementation is part of the adecc Scholar Project and supports diagnostics
        via structured logging.
 
  \version 1.0
  \date    12.05.2025
  \author Volker Hillmann (adecc Systemhaus GmbH)
  \copyright Copyright © 2020–2025 adecc Systemhaus GmbH
             This program is free software: you can redistribute it and/or modify it
             under the terms of the GNU General Public License, version 3.
             See <https://www.gnu.org/licenses/>.
 */

#include "Basics_i.h"

#include "Tools.h"
#include "my_logging.h"

#include <iostream>
#include <print>
#include <stdexcept>

 /**
   \brief Constructs the servant and stores a reference to the managing POA.
  
   \param poa Pointer to the POA that manages this servant instance.
  
   \note The POA pointer is duplicated to comply with CORBA ownership rules.
  */
DestroyableInterface_i::DestroyableInterface_i(PortableServer::POA_ptr poa) : poa_(PortableServer::POA::_duplicate(poa)) {
   log_trace<4>("[DestroyableInterface_i {}] Object created.", ::getTimeStamp());
   }

/**
  \brief Destructor for the servant.
 
  \details Logs object destruction. This is triggered when the last reference is released.
 */
DestroyableInterface_i::~DestroyableInterface_i() {
   log_trace<4>("[DestroyableInterface_i {}] Object destroyed", ::getTimeStamp());
   }

/**
  \brief Destroys the servant instance from the POA (server-side).
 
  \details This method is part of the implementation of the `Basics::DestroyableInterface` CORBA interface.
           It instructs the Portable Object Adapter (POA) to deactivate the servant using its ObjectId,
           effectively removing the object from the active object map. If the object is already inactive,
           the exception is caught and logged.
 
           After deactivation, the method calls `_remove_ref()` which decrements the servant’s reference count.
           When it reaches zero, the object is deleted.
 
  \note On the client side, a concept named `CORBAStubWithDestroy` is defined. It builds upon the
        `CORBAStub` concept (which verifies that a type represents a CORBA stub), and additionally checks
        that the stub exposes a `destroy()` method.
        This allows generic cleanup logic for CORBA objects in client applications, ensuring that transient
        objects are explicitly destroyed when no longer needed.
 
  \see CORBAStub
  \see CORBAStubWithDestroy
  \see PortableServer::POA::deactivate_object
  \see _remove_ref()
 */
void DestroyableInterface_i::destroy() {
   log_trace<4>("[DestroyableInterface_i {}] destroy() called.", ::getTimeStamp());

   try {
      poa_->deactivate_object(oid_);  // Objekt deregistrieren
      }
   catch (PortableServer::POA::ObjectNotActive const& ex) {
      // Is thrown if the OID is not active (i.e. already deactivated). We can ignore this.
      log_error("[DestroyableInterface_i {}] OID is not active (i.e. already deactivated): {}", ::getTimeStamp(), toString(ex));
      }
   catch (CORBA::Exception const& ex) {
      log_error("[DestroyableInterface_i {}] Exception during deactivate_object: {}", ::getTimeStamp(), toString(ex));
      }

   _remove_ref();  // Führt zu delete this bei RefCount 0
   }

/**
  \brief Assigns the POA Object ID for this servant.
 
  \param oid ObjectId assigned by POA during servant activation.
 
  \note This ID is required to allow proper deactivation via `destroy()`.
 */void DestroyableInterface_i::set_oid(PortableServer::ObjectId const& oid) {
   oid_ = oid;
   }

