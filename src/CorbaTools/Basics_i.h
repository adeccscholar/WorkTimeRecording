// SPDX-FileCopyrightText: 2025 adecc Systemhaus GmbH
// SPDX-License-Identifier: GPL-3.0-or-later
/**
  \file
  \brief CORBA servant base class for destroyable objects (Basics::DestroyableInterface).
 
  \details This header declares the `DestroyableInterface_i` class, a reusable servant
           base implementing the `Basics::DestroyableInterface` CORBA interface.
           It serves as a lifecycle anchor for transient objects managed by the POA.
 
  \details The class provides a concrete `destroy()` method which allows clients
           to explicitly request the destruction of a servant object. It also
           stores the Object ID (OID) assigned by the POA and can be used in more
           advanced servant activation/deactivation patterns.
 
  \note Servants deriving from `DestroyableInterface_i` should be managed using
        reference counting (via `PortableServer::RefCountServantBase`).
 
  \author Volker Hillmann (adecc Systemhaus GmbH)
  \date    06.06.2025
  \version 1.0
  \copyright Copyright © 2020 - 2025 adecc Systemhaus GmbH
             This program is free software: you can redistribute it and/or modify it
             under the terms of the GNU General Public License, version 3.
             See <https://www.gnu.org/licenses/>.
 
  \see Basics.idl
  \see Basics::DestroyableInterface (IDL)
  \see POA_Basics::DestroyableInterface (Skeleton)
 */
#pragma once

#include "BasicsS.h"

#include <tao/ORB_Core.h>
#include <tao/PortableServer/PortableServer.h>

/**
  \brief CORBA servant implementing the Basics::DestroyableInterface interface.
 
  \details This class provides a reusable base for servant implementations that support
           explicit destruction via CORBA. It keeps a reference to its managing POA
           and the associated Object ID, which are necessary to deregister the servant
           on client request (via `destroy()`).
 
  \note This base should be used for servants that are transient or need explicit cleanup
        in dynamic activation scenarios (e.g., servants created on-demand).
 */
class DestroyableInterface_i : public virtual PortableServer::RefCountServantBase,
   public virtual POA_Basics::DestroyableInterface {
public:
   DestroyableInterface_i() = delete;
   DestroyableInterface_i(PortableServer::POA_ptr poa);
   virtual ~DestroyableInterface_i();

   virtual void destroy() override;
   void set_oid(PortableServer::ObjectId const& oid);
private:
   PortableServer::POA_var      poa_; ///< Reference to the Portable Object Adapter managing this servant.
   PortableServer::ObjectId_var oid_; ///< Object ID assigned to this servant instance.

};
