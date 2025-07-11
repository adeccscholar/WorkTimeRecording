// SPDX-FileCopyrightText: 2025 adecc Systemhaus GmbH
// SPDX-License-Identifier: GPL-3.0-or-later

/**
\file
\brief HTTP client wrapper using Boost.Beast and Boost.Asio for synchronous GET requests.

\details
This header defines the \c HttpRequest class, a minimal HTTP client for performing synchronous GET requests using the Boost.Beast and Boost.Asio libraries.
It provides robust error handling and basic connection management, including automatic reconnection on typical connection failures. The class is intended for internal or backend use, such as querying public APIs like Open Meteo within finite state machine servers.

\see https://www.boost.org/doc/libs/release/libs/beast/
\see https://www.boost.org/doc/libs/release/libs/asio/

\version 1.0
\date    23.06.2025
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
  along with this program. If not, see https://www.gnu.org/licenses/.
  \endlicenseblock

  \note This file is part of the adecc Scholar project - Free educational materials for modern C++.

*/

#pragma once

#include <boost/beast.hpp>
#include <boost/asio.hpp>
#include <string>
#include <format>

namespace asio  = boost::asio;
namespace beast = boost::beast;
namespace http  = beast::http;
using tcp = asio::ip::tcp;      

/**
\brief Simple synchronous HTTP client using Boost.Beast and Boost.Asio.

\details
This class establishes a TCP connection to a specified HTTP server (host and port) and enables synchronous HTTP GET requests.
It provides basic error handling and supports automatic reconnection if the connection is lost (e.g. due to timeout or server reset).
Typical usage includes requesting external APIs such as Open Meteo in server or state machine contexts.

\warning
This client operates synchronously and is not suitable for high-concurrency scenarios. For non-blocking use cases, consider asynchronous APIs.

*/
class HttpRequest {
private:
   asio::io_context ioContext_; ///< Internal IO context for network operations
   tcp::resolver resolver_;     ///< DNS resolver
   tcp::socket socket_;         ///< TCP socket for the connection
   std::string host_;           ///< Target host
   std::string port_;           ///< Target port
public:
   /**
     \brief Constructs and connects the HTTP client to a given host and port.

     \details
      Resolves the host and establishes a TCP connection. Throws if resolution or connection fails.

     \param host  The remote server hostname (e.g. "api.open-meteo.com").
     \param port  The remote server port as string (default "80").

     \throw std::runtime_error on resolution or connection failure.
   */
   HttpRequest(std::string host, std::string port = "80") : ioContext_(), resolver_(ioContext_), socket_(ioContext_),
               host_(std::move(host)), port_(std::move(port)) {
      boost::system::error_code ec;

     // const auto results = resolver_.resolve(host_, port_, ec);
      const auto results = resolver_.resolve(boost::asio::ip::tcp::v4(), host_, port_, ec);
      if (ec) {
         throw std::runtime_error(std::format("Failed to resolve {}:{} - {}", host_, port_, ec.message()));
         }

      boost::asio::connect(socket_, results.begin(), results.end(), ec);
      if (ec) {
         throw std::runtime_error(std::format("Failed to connect to {}:{} - {}", host_, port_, ec.message()));
         }
      }

   /**
      \brief Destructor: shuts down and closes the TCP socket if open.
   */
   ~HttpRequest() {
      if (socket_.is_open()) {
         boost::system::error_code ec;
         socket_.shutdown(tcp::socket::shutdown_both, ec);
         socket_.close(ec); // optional
         }
      }


   /**
      \brief Attempts to reconnect the TCP socket to the configured host and port.

      \details
       Closes the existing socket (if open), resolves the host, and re-establishes the connection.
       Throws on failure.

      \throw std::runtime_error if resolution or connection fails.
   */
   void reconnect() {
      boost::system::error_code ec;
      socket_.close(ec);
      // const auto results = resolver_.resolve(host_, port_, ec);
      const auto results = resolver_.resolve(boost::asio::ip::tcp::v4(), host_, port_, ec);
      if (ec) throw std::runtime_error("Reconnect: resolve failed: " + ec.message());
      boost::asio::connect(socket_, results.begin(), results.end(), ec);
      if (ec) throw std::runtime_error("Reconnect: connect failed: " + ec.message());
      }


   /**
     \brief Performs a synchronous HTTP GET request to the specified endpoint.
 
     \details
      Sends a GET request and returns the response body as a string.
      On typical lost connection errors (EOF, connection reset, broken pipe, end of stream), the client 
      attempts to reconnect and retries the request once.

     \param endpoint_path The HTTP resource path (e.g. "/v1/forecast?...").
     \returns The HTTP response body as string.

     \throw std::runtime_error on unrecoverable error.

     \note Example for simple implementation without reconnect
     \code{.cpp}
      std::string perform_get(std::string const& endpoint_path) {
         http::request<http::string_body> req{ http::verb::get, endpoint_path, 11 };
         req.set(http::field::host, host_);
         req.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);

         http::write(socket_, req);

         beast::flat_buffer buffer;
         http::response<http::string_body> res;

         http::read(socket_, buffer, res);
         return res.body();
         }
     \endcode
   */
   std::string perform_get(std::string const& endpoint_path) {
      http::request<http::string_body> req{ http::verb::get, endpoint_path, 11 };
      req.set(http::field::host, host_);
      req.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);

      for (uint32_t attempt = 0; attempt < 2; ++attempt) {
         boost::system::error_code ec;
         try {
            http::write(socket_, req, ec);
            if (ec) throw boost::system::system_error(ec);

            beast::flat_buffer buffer;
            http::response<http::string_body> res;
            http::read(socket_, buffer, res, ec);
            if (ec) throw boost::system::system_error(ec);
            return res.body();
            }
         catch (const boost::system::system_error& ex) {
            auto code = ex.code();           
            if (attempt == 0 && // check for typical lost of connections problems 
                (code == asio::error::eof         || code == asio::error::connection_reset ||
                 code == asio::error::broken_pipe || code == http::error::end_of_stream)
               ) {
               reconnect();
               continue; // retry once again after successful reconnect!
               }
            else {
               throw; // other error or second try to get the data
               }
            }
         }
      throw std::runtime_error("perform_get: unreachable code reached");
      }


};