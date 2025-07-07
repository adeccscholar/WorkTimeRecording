// SPDX-FileCopyrightText: 2025 adecc Systemhaus GmbH
// SPDX-License-Identifier: GPL-3.0-or-later

/**
\file
\brief Generic and extensible pattern for object-mapping from Boost.JSON to C++ types using tag-based ADL.

\details
This header defines a minimal but highly flexible and robust pattern for mapping JSON objects (from Boost.JSON) onto arbitrary C++ types.  
The system leverages **tag dispatching** and **SFINAE** (Substitution Failure Is Not An Error) to enable automatic detection and 
invocation of user-defined conversion functions, without requiring inheritance, base classes, or registration macros.

**Pattern Overview:**
- To enable conversion for a type \c MyType, provide a function with the exact signature:
  \code
  void from_json(MyType& dest, boost::json::object const& obj, boost_tools::from_json_tag);
  \endcode
  This function can be defined in the same namespace as \c MyType (to allow ADL).
- The central entry point \c from_json<T>(json_value) automatically detects if a matching `from_json` is available (using SFINAE and ADL). If not, compilation fails with a clear error.
- This design separates the *mapping logic* (how each type is built from JSON) from the *pattern machinery* (the entry point and detection), and allows extensibility without modifying the framework code.

**Advantages:**
- **Extensibility:** Support for new types is added non-intrusively by providing a suitable \c from_json function, not by modifying the library.
- **Compile-time safety:** Errors are reported at compile time if no mapping is available for a type.
- **ADL-based customization:** The pattern allows for customization in the user's domain, not forced into a central registry.
- **No inheritance or macros:** Clean separation of mapping logic; no intrusive base classes or macro usage.

**Usage Example:**
\code
struct MyStruct { int id; std::string name; };

namespace boost_tools {
   void from_json(MyStruct& out, boost::json::object const& obj, from_json_tag) {
      out.id   = get_value<int>(obj, "id");
      out.name = get_value<std::string>(obj, "name");
   }
}

// Deserialization:
MyStruct s = boost_tools::from_json<MyStruct>(json_value);
\endcode

**Pattern Note:**  
This approach is inspired by tag-based dispatch and traits-based detection as seen in modern 
serialization libraries (such as nlohmann::json), but does not rely on macros or class inheritance. 
Instead, it combines SFINAE with ADL to provide both safety and extensibility.


\see get_value (type-safe field extraction)
\see https://www.boost.org/doc/libs/release/libs/json/

\version 1.0
\date    26.06.2025
\author  Volker Hillmann (adecc Systemhaus GmbH)

\copyright Copyright © 2020 - 2025 adecc Systemhaus GmbH

  \licenseblock{GPL-3.0-or-later}
  This program is free software: you can redistribute it and/or modify it
  under the terms of the GNU General Public License, version 3,
  as published by the Free Software Foundation.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program. If not, see <https://www.gnu.org/licenses/>.
  \endlicenseblock

  \note This file is part of the adecc Scholar project - Free educational materials for modern C++.

*/

#pragma  once

#include <boost/json/value.hpp>
#include <boost/json/object.hpp>
#include <type_traits>
#include <stdexcept>
#include <string>
#include <format>


namespace boost_tools {

   /**
     \brief Tag type for ADL-dispatch of user-defined JSON conversion functions.

     \details
       This struct is used solely as a tag for distinguishing user-defined overloads of \c from_json from unrelated functions.  
       It prevents accidental matching of unrelated functions in ADL.
   */
   struct from_json_tag {};

   /**
     \brief Type trait to detect whether a type \c ty has an ADL-discovered \c from_json function with the correct signature.

     \details
        The primary template yields \c false for types with no matching function; a partial specialization uses SFINAE to check 
		for the presence of a suitable overload.

     \tparam ty Type to test.
   */   
   template <typename ty, typename = void>
   struct has_from_json : std::false_type {};

   /**
     \brief Partial specialization: detected if \c from_json(ty&, boost::json::object const&, from_json_tag) is available.

     \details
      This enables compile-time detection (via \c has_from_json_v) of mapping support for a type.

     \tparam ty Type to test.
   */
   template <typename ty>
   struct has_from_json <ty, std::void_t<decltype(
             from_json(std::declval<ty&>(), std::declval<boost::json::object const&>(), from_json_tag{}))>> : std::true_type {};

   /**
     \brief Convenience variable template for querying \c has_from_json for a type.

     \details
       Use as \c has_from_json_v<T> to test if a suitable \c from_json function exists.

     \tparam ty Type to test.
   */
   template <typename ty>
   constexpr bool has_from_json_v = has_from_json<ty>::value;

   /**
     \brief Entry point for generic JSON-to-object mapping.

     \details
     Attempts to convert a Boost.JSON value to type \c ty using an ADL-found, tag-dispatched \c from_json overload.  
     - Throws if the JSON value is not an object.
     - If a suitable \c from_json function exists (see above), it is called to fill the result.
     - If not, a static assertion fails at compile time with a clear message.

     \tparam ty The type to which to convert.
     \param jv The Boost.JSON value (should be an object).
     \return An instance of \c ty, constructed from the JSON object.
     \throw std::runtime_error if \c jv is not a JSON object.
     \throw static_assert at compile time if no suitable \c from_json function exists.

     \code{.cpp}
     struct Foo { int x; std::string y; };
     namespace boost_tools {
        void from_json(Foo& dest, boost::json::object const& obj, from_json_tag) {
           dest.x = get_value<int>(obj, "x");
           dest.y = get_value<std::string>(obj, "y");
           }
        }
     Foo foo = boost_tools::from_json<Foo>(some_json_value);
     \endcode

     \warning The user must provide a \c from_json function in the same namespace as the target type (to enable ADL).

     \see has_from_json_v, from_json_tag
   */
   template <typename ty>
   ty from_json(boost::json::value const& jv) {
      if (!jv.is_object())
         throw std::runtime_error("expected JSON object");

      boost::json::object const& obj = jv.as_object();
      ty result;

      if constexpr (has_from_json_v<ty>) {
         from_json(result, obj, from_json_tag{});
         }
      else {
         static_assert(sizeof(ty) == 0, "No from_json overload found for this type.");
         }
      return result;
      }

} // namespace boost_tools
