// SPDX-FileCopyrightText: 2023 Michael Fuhs
// SPDX-FileCopyrightText: 2024 Volker Hillmann, adecc Systemhaus GmbH
// SPDX-License-Identifier: MIT

/**
 \file 

 \brief This file provides a C++ RAII-style wrapper class for accessing I2C devices
        using the Linux I2C subsystem. It is intended for use in embedded projects,
        such as on a Raspberry Pi.

 \details This code was originally developed by Michael Fuhs for educational purposes
          in 2023 and later adapted and extended for modern C++ by Volker Hillmann
          as part of the adecc Scholar project.

 \author Michael Fuhs (original)
 \author Volker Hillmann (adecc Systemhaus GmbH) 

 \copyright © 2023–2025 adecc Systemhaus GmbH
            
 \licenseblock{MIT}
 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the “Software”), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in all
 copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 SOFTWARE.
 \endlicenseblock

 \note This file is part of the adecc Scholar project – Free educational materials for modern C++.
 \version 2.0
 \date    25.06.2025

*/

#pragma once


#include <string>
#include <string_view>
#include <span>
#include <system_error>
#include <stdexcept>
#include <memory>
#include <filesystem>
#include <cstdint>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>

using namespace std::string_literals;

/**
 \brief A reusable and extensible I2C device wrapper class with automatic resource management.

 \details This class encapsulates the lifecycle of a Linux file descriptor for I2C devices,
          provides exception-safe I/O operations using `std::system_error`, and supports
          writing and reading using `std::span`.

 \details Designed as a base class for higher-level abstractions like LCD displays or sensors.

 \details Example usage:
          \code
          I2CDevice device("BME280", 0x76);
          std::array<uint8_t, 2> cmd = {0xF4, 0x27};
          std::array<uint8_t, 8> response;
          device.WriteRead(cmd, response);
          \endcode
*/
class I2CDevice {
public:
   /**
    \brief Deleted default constructor to avoid uninitialized objects.
   */
   I2CDevice() = delete;

   /**
    \brief Constructs the I2C device and assigns its slave address.
    \param name A human-readable name for the device (used in error reporting).
    \param addr I2C slave address to assign via ioctl.
    \param device Path to the I2C bus (default: "/dev/i2c-1").
    \throws std::system_error if the device cannot be opened or configured.
   */
   explicit I2CDevice(std::string_view name, uint16_t addr, std::filesystem::path const& device = "/dev/i2c-1")
                                 : name_{ name }, dev_{ Open(device) } {
      if (::ioctl(*dev_, I2C_SLAVE, addr) < 0) {
         throw std::system_error(errno, std::generic_category(), "Failed to set I2C address for device: "s + name_ );
         }
      }

   /**
    \brief Virtual destructor ensures proper cleanup in derived classes.
   */
   virtual ~I2CDevice() = default;

   /**
    \brief Checks if the device was successfully initialized.
    \return true if the file descriptor is valid, false otherwise.
   */
   bool ok() const noexcept {
      return dev_ && *dev_ >= 0;
      }

   /**
    \brief Sends raw bytes to the I2C device.
    \param data The span of data to write.
    \throws std::system_error if the write fails.
   */
   void Write(std::span<const uint8_t> data) {
      auto written = ::write(*dev_, data.data(), data.size());
      if (written != static_cast<ssize_t>(data.size())) {
         throw std::system_error(errno, std::generic_category(), "Write operation failed");
         }
      }

   /**
    \brief Reads bytes from the I2C device.
    \param buffer A span to hold the read data.
    \throws std::system_error if the read fails.
   */
   void Read(std::span<uint8_t> buffer) {
      auto read = ::read(*dev_, buffer.data(), buffer.size());
      if (read != static_cast<ssize_t>(buffer.size())) {
         throw std::system_error(errno, std::generic_category(), "Read operation failed");
         }
      }

   /**
    \brief Performs a sequential write followed by a read on the I2C device.
    \param writeData Data to write before reading.
    \param readData Buffer to store the read result.
    \throws std::system_error if either operation fails.
   */
   void WriteRead(std::span<const uint8_t> writeData, std::span<uint8_t> readData) {
      if (!writeData.empty()) Write(writeData);
      if (!readData.empty()) Read(readData);
      }

protected:
   std::string name_; ///< The device name (used for diagnostics and errors).

   /**
    \brief Unique file descriptor with RAII-based cleanup.
   */
   std::unique_ptr<int, void(*)(int*)> dev_{ nullptr, [](int* fd) {
       if (fd && *fd >= 0) ::close(*fd);
       delete fd;
       } };

private:
   /**
    \brief Helper to open the device file and return a managed file descriptor.
    \param device Path to the device file.
    \return A unique_ptr managing the opened file descriptor.
    \throws std::system_error if the file cannot be opened.
   */
   static std::unique_ptr<int, void(*)(int*)> Open(std::filesystem::path const& device) {
      int rawFd = ::open(device.c_str(), O_RDWR);
      if (rawFd < 0) {
         throw std::system_error(errno, std::generic_category(), "Failed to open I2C device: "s + device.string());
         }
      return std::unique_ptr<int, void(*)(int*)>(new int{ rawFd }, [](int* fd) {
         if (fd && *fd >= 0) ::close(*fd);
         delete fd;
         });
   }
};