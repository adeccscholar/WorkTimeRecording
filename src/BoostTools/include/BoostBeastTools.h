#pragma once

#include <boost/beast.hpp>
#include <boost/asio.hpp>
#include <string>
#include <format>

/*
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
*/

namespace asio  = boost::asio;
namespace beast = boost::beast;
namespace http  = beast::http;
using tcp = asio::ip::tcp;

class HttpRequest {
private:
   asio::io_context ioContext_;
   tcp::resolver resolver_;
   tcp::socket socket_;
   std::string host_;
   std::string port_;
public:
   HttpRequest(std::string host, std::string port = "80") : ioContext_(), resolver_(ioContext_), socket_(ioContext_),
               host_(std::move(host)), port_(std::move(port)) {
      boost::system::error_code ec;

      const auto results = resolver_.resolve(host_, port_, ec);
      if (ec) {
         throw std::runtime_error(std::format("Failed to resolve {}:{} - {}", host_, port_, ec.message()));
         }

      boost::asio::connect(socket_, results.begin(), results.end(), ec);
      if (ec) {
         throw std::runtime_error(std::format("Failed to connect to {}:{} - {}", host_, port_, ec.message()));
         }
      }

   ~HttpRequest() {
      if (socket_.is_open()) {
         boost::system::error_code ec;
         socket_.shutdown(tcp::socket::shutdown_both, ec);
         socket_.close(ec); // optional
         }
      }

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

};