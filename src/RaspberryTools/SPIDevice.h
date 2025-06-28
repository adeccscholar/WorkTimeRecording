// SPDX-FileCopyrightText: 2025 Michael Fuhs
// SPDX-FileCopyrightText: 2025 Volker Hillmann, adecc Systemhaus GmbH
// SPDX-License-Identifier: MIT

/**
  \file
  \brief Modern C++23 RAII-based SPI device wrapper for Linux.
 
  \details
  This header defines a reusable, type-safe, and exception-aware abstraction over the Linux
  SPI subsystem, using file descriptors accessed via `/dev/spidev*`. The class `SPIDevice`
  provides RAII resource management via `std::unique_ptr` and modern C++ error handling using
  `std::system_error` for all system-level failures.
 
  It supports full-duplex SPI transfers through `std::span` views, allows configuration
  of SPI mode, speed, and word size, and is suitable for embedded applications such as
  Raspberry Pi-based sensor systems.
 
  Key features:
  - Modern C++23 syntax and standard library features
  - Exception-safe resource cleanup using `unique_ptr` with custom deleter
  - Portable, idiomatic interface with strong typing
  - No explicit destructor needed (RAII)
 
  \author Michael Fuhs (C like original classes)
  \author Volker Hillmann (adecc Systemhaus GmbH)
  \copyright © 2025 adecc Systemhaus GmbH
 
  \licenseblock{MIT}
  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:
 
  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.
 
  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
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

#include <span>             // std::span — used in transfer interface
#include <string>           // std::string — used in std::format, path output
#include <string_view>      // Optional: could be removed if unused (currently unused)
#include <format>           // std::format — formatting error messages
#include <memory>           // std::unique_ptr — manages file descriptor RAII
#include <stdexcept>        // std::runtime_error (inherited by std::system_error)
#include <system_error>     // std::system_error — used for exception-based error reporting
#include <cstdint>          // fixed width integer types (e.g., uint8_t, uint32_t)
#include <filesystem>       // std::filesystem::path — path abstraction for devices

#include <fcntl.h>            // ::open — POSIX function to open a device file
#include <unistd.h>           // ::close — POSIX function to close file descriptor
#include <sys/ioctl.h>        // ioctl — required to configure SPI parameters
#include <linux/spi/spidev.h> // Linux SPI device API (defines spi_ioc_transfer etc.)


 /**
   \brief C++23 RAII wrapper for Linux SPI interface.
  
   \details Manages a Linux SPI device using modern C++ RAII principles. Opens `/dev/spidev*`
   with the given path, configures SPI settings (mode, speed, bits per word), and performs
   full-duplex SPI transfers. Ensures file descriptor is properly closed via `std::unique_ptr`
   and a custom deleter.
  */
class SPIDevice {
public:
   /**
     \brief Constructs and configures a SPI device with RAII ownership.
    
     \details
     This constructor opens the specified SPI device file and applies the desired configuration
     (mode, speed, bits per word). It takes ownership of the device path via move semantics and
     wraps the resulting file descriptor in a `std::unique_ptr` with a custom deleter to ensure
     proper cleanup.
    
     All parameters are passed by value to allow copy or move construction. Especially for the
     path, moving a `std::filesystem::path` into the class allows efficient resource transfer
     without unnecessary copies.
    
     All configuration steps (via `ioctl`) are performed immediately after opening the device.
     Any error will result in a `std::system_error` being thrown.
    
     \param path A `std::filesystem::path` to the Linux SPI device (e.g. "/dev/spidev0.0"); must be readable and valid.
     \param mode SPI bus mode to configure (0–3), controlling clock polarity and phase.
     \param speedHz Desired bus clock speed in Hertz (e.g. 1'000'000 for 1 MHz).
     \param bitsPerWord Bit width per transfer unit, typically 8 for most peripherals.
    
     \throws std::system_error If the device cannot be opened or any ioctl configuration fails.
    
     \note Passing parameters by value allows move-optimization by the caller, in line with C++23 conventions.
    */
   SPIDevice(std::filesystem::path path, uint8_t mode, uint32_t speedHz, uint8_t bitsPerWord)
      : devicePath_{ std::move(path) }, mode_{ mode }, speedHz_{ speedHz }, bitsPerWord_{ bitsPerWord } {

      int rawFd = ::open(devicePath_.c_str(), O_RDWR);
      if (rawFd < 0)
         throw std::system_error(errno, std::generic_category(), std::format("Failed to open SPI device '{}'", devicePath_.string()));

      fd_.reset(new int(rawFd));

      auto set = [this](int request, auto* value) {
         if (::ioctl(*fd_, request, value) < 0)
            throw std::system_error(errno, std::generic_category(), std::format("SPI ioctl {} failed on '{}'", request, devicePath_.string()));
         };

      set(SPI_IOC_WR_MODE, &mode_);
      set(SPI_IOC_RD_MODE, &mode_);
      set(SPI_IOC_WR_BITS_PER_WORD, &bitsPerWord_);
      set(SPI_IOC_RD_BITS_PER_WORD, &bitsPerWord_);
      set(SPI_IOC_WR_MAX_SPEED_HZ, &speedHz_);
      set(SPI_IOC_RD_MAX_SPEED_HZ, &speedHz_);
      }

   /**
     \brief Performs a full-duplex SPI transfer.
     \param tx Buffer to write (may be empty)
     \param rx Buffer to read into (same size as transfer)
     \param leaveCSLow Whether to leave chip-select asserted after transfer
     \return Number of bytes transferred
     \throws std::system_error on transfer error
    */
   int transfer(std::span<const uint8_t> tx, std::span<uint8_t> rx, bool leaveCSLow = false) {
      spi_ioc_transfer xfer = {
         .tx_buf = reinterpret_cast<__u64>(tx.data()),
         .rx_buf = reinterpret_cast<__u64>(rx.data()),
         .len = static_cast<__u32>(std::max(tx.size(), rx.size())),
         .delay_usecs = 0,
         .speed_hz = speedHz_,
         .bits_per_word = bitsPerWord_,
         .cs_change = static_cast<__u8>(leaveCSLow ? 1 : 0),
         };

      if (::ioctl(*fd_, SPI_IOC_MESSAGE(1), &xfer) < 0)
         throw std::system_error(errno, std::generic_category(), "SPI transfer failed");

      return static_cast<int>(xfer.len);
      }

private:
   std::filesystem::path devicePath_; ///< Filesystem path to the SPI device (e.g. "/dev/spidev0.0")
   /// Unique pointer managing the file descriptor with custom deleter
   std::unique_ptr<int, decltype([](int* fd) { if (fd && *fd >= 0) ::close(*fd); delete fd; }) > fd_{ nullptr };
   uint8_t mode_;        ///< SPI communication mode (0–3)
   uint8_t bitsPerWord_; ///< Number of bits per SPI transfer word
   uint32_t speedHz_;    ///< SPI bus speed in Hz
};