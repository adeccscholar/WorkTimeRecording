// SPDX-FileCopyrightText: 2025 adecc Systemhaus GmbH
// SPDX-License-Identifier: GPL-3.0-or-later

/**
  \file
  \brief Utility functions to enumerate all entries in the CORBA Naming Service.

  \details This file provides helper functions to recursively retrieve all
  registered object names within a given \c CosNaming::NamingContext.
  The functions use the TAO binding model and are efficient enough for small
  to medium-sized naming trees. This retrieval can be used for dynamic lookup
  or diagnostics of registered naming entries.

  \author Volker Hillmann (adecc Systemhaus GmbH)

  \copyright Copyright © adecc Systemhaus GmbH 2021–2025

  \license This project is mostly licensed under the GNU General Public License v3.0.
           See the LICENSE file for details.

  \version 1.0
  \date    31.05.2025
 */


#pragma once

#include "tao/Object.h"
#include <orbsvcs/CosNamingC.h>

#include <concepts>
#include <vector>
#include <string>
#include <sstream>

/**
  \brief Recursive helper function for traversing a NamingContext.

  \param ctx The current \c CosNaming::NamingContext being traversed.
  \param prefix Path prefix used during recursive path construction.
  \param names Output list of fully qualified object paths found.

  \note This function is internally called by \ref get_all_names.
  It handles both simple object bindings and sub-contexts.
 */
inline void collect_names_recursive(CosNaming::NamingContext_ptr ctx, std::string const& prefix, std::vector<std::string>& names) {
   CosNaming::BindingList_var bindingList;
   CosNaming::BindingIterator_var bindingIter;

   // Hole alle Einträge (initial max. 100)
   ctx->list(100, bindingList, bindingIter);

   auto process_binding = [&](const CosNaming::Binding& b) {
      std::ostringstream full_name;
      if (!prefix.empty()) full_name << prefix << "/";

      for (CORBA::ULong j = 0; j < b.binding_name.length(); ++j) {
         if (j > 0) full_name << "/";
         full_name << b.binding_name[j].id;
         }

      std::string name_str = full_name.str();

      if (b.binding_type == CosNaming::nobject) [[likely]] { names.emplace_back(name_str); }
      else if (b.binding_type == CosNaming::ncontext) {
         try {
            CORBA::Object_var obj = ctx->resolve(b.binding_name);
            CosNaming::NamingContext_var subctx = CosNaming::NamingContext::_narrow(obj);
            if (!CORBA::is_nil(subctx)) {
               collect_names_recursive(subctx, name_str, names);
               }
            }
         catch (CORBA::Exception const& ex) {
            std::cerr << "Fehler beim Traversieren von " << name_str << ": " << ex._name() << std::endl;
            }
         }
      };

   for (CORBA::ULong i = 0; i < bindingList->length(); ++i) process_binding(bindingList[i]);

   // iterator weiter abarbeiten
   if (!CORBA::is_nil(bindingIter)) {
      CosNaming::BindingList_var more;
      while (bindingIter->next_n(10, more) && more->length() > 0) {
         for (CORBA::ULong i = 0; i < more->length(); ++i) process_binding(more[i]);
         }
      }
   }

/**
 \brief Returns a list of all registered names within the given context.

 \param root_ctx Root of the naming tree (typically the InitialContext).
 \return A vector containing all object paths in the NamingService.

 \note This function can be used for logging, monitoring, or automatic binding resolution.
 */
inline std::vector<std::string> get_all_names(CosNaming::NamingContext_ptr root_ctx) {
   std::vector<std::string> result;
   collect_names_recursive(root_ctx, "", result);
   return result;
   }
