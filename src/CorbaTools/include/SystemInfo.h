// using ace, TAO included this. Thats why I added this file to the corba tools

#pragma once

#include <ace/ACE.h>
#include <ace/OS_NS_unistd.h>
#include <ace/OS_NS_netdb.h>
#include <ace/OS_NS_ctype.h>
#include <ace/OS_NS_sys_utsname.h>
#include <ace/SOCK_Dgram.h>
#include <ace/INET_Addr.h>

#include <ace/SOCK_Connector.h>
#include <ace/SOCK_Stream.h>

#include <string>
#include <stdexcept>
#include <filesystem>

namespace ace_tools {

class SystemInformations {
public:
   SystemInformations() = delete;
   SystemInformations(SystemInformations const&) = delete;
   SystemInformations(SystemInformations&&) noexcept = delete;

   SystemInformations(std::string const& name) {
      pathProgram = name;
      char szHostname[256]{};
      if (ACE_OS::hostname(szHostname, sizeof(szHostname)) == -1) {
         throw std::runtime_error("error while read the hostname");
         }
      strHostname = szHostname;
	  
	  strDomain = "";
      // Versuche, FQDN zu erhalten
      struct hostent* pHostEnt = ACE_OS::gethostbyname(szHostname);
      if (pHostEnt && pHostEnt->h_name) {
         std::string strFQDN{pHostEnt->h_name};
         if (auto const uPos = strFQDN.find('.'); uPos != std::string::npos && uPos + 1 < strFQDN.size()) {
            strDomain = strFQDN.substr(uPos + 1); // alles nach dem ersten Punkt
		    }
         }	  
	  

      ACE_INET_Addr remote_addr(80, ACE_TEXT("telekom.de"));
      //ACE_INET_Addr remote_addr(80, ACE_TEXT("91.250.112.78")); 
      //ACE_INET_Addr remote_addr(80, ACE_TEXT("adecc.de"));
      ACE_SOCK_Stream stream;
      ACE_SOCK_Connector connector;

      if (connector.connect(stream, remote_addr) == -1) {
         throw std::runtime_error("Error connecting to remote address");
         }

      ACE_INET_Addr local_addr;
      if (stream.get_local_addr(local_addr) == -1) {
         throw std::runtime_error("Error getting local address");
         stream.close();
         }

      stream.close();

      char szLocalIP[256];
      local_addr.addr_to_string(szLocalIP, sizeof(szLocalIP));
      strLocalAddress = szLocalIP;
      if(size_t colonPos = strLocalAddress.find(':'); colonPos != std::string::npos) {
         strLocalAddress.erase(colonPos);
         }

      // Systeminformationen via uname
      ACE_utsname aUts{};
      if (ACE_OS::uname(&aUts) == 0) {
         strSystemName  = aUts.sysname;
         strSystemRelease = aUts.release;
         strSystemVersion = aUts.version;
         strNodeName      = aUts.nodename;
         strArchitecture  = aUts.machine;
         }
      else {
         strSystemName    = "";
         strSystemRelease = "";
         strSystemVersion = "";
         strNodeName      = "";
         strArchitecture  = "";
         }

      iCpuCount = ACE_OS::num_processors_online();
      llPID     = static_cast<long long>(ACE_OS::getpid());
      }



   std::filesystem::path const& ProgramPath() const { return pathProgram;  }
   std::string const&           Hostname() const { return strHostname; }
   std::string const&           Domain() const { return strDomain; }
   std::string const&           LocalAddress() const { return strLocalAddress; }
   std::string const&           SystemName() const { return strSystemName; }
   std::string const&           SystemRelease() const { return strSystemRelease; }
   std::string const&           SystemVersion() const { return strSystemVersion; }
   std::string const&           NodeName() const { return strNodeName; }
   std::string const&           Architecture() const { return strArchitecture; }
   int                          CpuCount() const { return iCpuCount; }
   long long                    PID() const { return llPID; };

private:
   std::filesystem::path pathProgram;
   std::string strHostname;
   std::string strDomain;
   std::string strLocalAddress;

   std::string strSystemName;
   std::string strSystemRelease;
   std::string strSystemVersion;
   std::string strNodeName;
   std::string strArchitecture;

   int         iCpuCount;
   long long   llPID;
};

} // end of namespace ace_tools