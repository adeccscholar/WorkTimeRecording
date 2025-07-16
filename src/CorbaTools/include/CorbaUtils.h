#pragma once

#include <tao/corba.h>

#include <sstream>
#include <type_traits>
#include <string_view>

/// \brief Generischer Fallback für beliebige CORBA-Exceptions
template<typename ty>
std::string toString(ty const& ex) requires std::is_base_of_v<CORBA::Exception, ty> {
   std::ostringstream os;
   os << "CORBA Exception: " << ex;
   return os.str();
   }

/// \brief Spezialisierung für COMM_FAILURE mit zusätzlichem Hinweistext
inline std::string toString(CORBA::COMM_FAILURE const& ex) {
   std::ostringstream os;
   os << "CORBA Communication Failure: " << ex << '\n'
      << "Is the server running and reachable?\n"
      << "Is the NameService available and configured correctly (ORBInitRef)?";
   return os.str();
   }

/// \brief Spezialisierung für TRANSIENT mit Hinweis auf mögliche Startverzögerung
inline std::string toString(CORBA::TRANSIENT const& ex) {
   std::ostringstream os;
   os << "CORBA Transient Exception: " << ex << '\n'
      << "The server may be starting, shutting down, or temporarily unavailable.\n"
      << "Retrying might help.";
   return os.str();
   }

/*
inline std::string toString(PortableServer::POA::ObjectNotActive const& ex) {
   // OID is not active (i.e. already deactivated)
   }
*/

template<typename ty>
std::string toString(ty const& ex, std::string_view hint)  requires std::is_base_of_v<CORBA::Exception, ty> {
   std::ostringstream os;
   if (!hint.empty()) os << hint << "\n";
   os << toString(ex);
   return os.str();
   }