
#pragma once

#include <stdint.h>
#include <iostream>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <time.h>
#include <errno.h>
#include <string.h>
#include <cmath>
#include <linux/spi/spidev.h>



#define MAX(_a, _b)  (_a >= _b ? _a : _b)

class T_SPI
{
public:
  T_SPI(void) : tSPI(-1)
  {

  }

  virtual ~T_SPI(void)
  {
    if (0 < tSPI)
    {
      close(tSPI);
    }
  }

  void Init(uint8_t u8ModeNew, uint32_t u32SpeedNew, uint8_t u8BitsPerWordNew)
  {
    u8Mode = u8ModeNew;
    u32Speed = u32SpeedNew;
    u8BitsPerWord = u8BitsPerWordNew;

    tSPI = open("/dev/spidev0.0", O_RDWR);
    if (0 > tSPI)
    {
      std::cout << "ERR (IMU.cpp:IMU()): Failed to open SPI communication on /dev/spidev0.0" << std::endl;
      return;
    }

    if (ioctl(tSPI, SPI_IOC_WR_MODE, &u8Mode) < 0)
    {
      close(tSPI);
    }
    if (ioctl(tSPI, SPI_IOC_RD_MODE, &u8Mode) < 0)
    {
      close(tSPI);
    }
    /* Set bits per word*/
    if (ioctl(tSPI, SPI_IOC_WR_BITS_PER_WORD, &u8BitsPerWord) < 0)
    {
      close(tSPI);
    }
    if (ioctl(tSPI, SPI_IOC_RD_BITS_PER_WORD, &u8BitsPerWord) < 0)
    {
      close(tSPI);
    }

    /* Set SPI speed*/
    if (ioctl(tSPI, SPI_IOC_WR_MAX_SPEED_HZ, &u32Speed) < 0)
    {
      close(tSPI);
    }
    if (ioctl(tSPI, SPI_IOC_RD_MAX_SPEED_HZ, &u32Speed) < 0)
    {
      close(tSPI);
    }
  }

  int Transfer(uint8_t* pu8DataWrite, uint32_t u32LengthWrite, uint8_t* pu8DataRead, uint32_t u32LengthRead, bool bLeaveCSLow = false)
  {
    struct spi_ioc_transfer tTransfer = { 0 };

    tTransfer.tx_buf = (__u64)pu8DataWrite;
    tTransfer.rx_buf = (__u64)pu8DataRead;
    tTransfer.len = (__u32)MAX(u32LengthWrite, u32LengthRead);
    tTransfer.delay_usecs = 0;
    tTransfer.speed_hz = u32Speed;
    tTransfer.bits_per_word = u8BitsPerWord;
    tTransfer.cs_change = 0;
    if (true == bLeaveCSLow) [[unlikely]]
    {
      tTransfer.cs_change = 1;
    }

    int rResult = ioctl(tSPI, SPI_IOC_MESSAGE(1), &tTransfer);
    if (0 > rResult)
    {
      std::cout << "error in spi_write_reg8(): ioctl(SPI_IOC_MESSAGE(2)) return" << std::endl;
    }
    return rResult;
  }

private:
  int tSPI;
  uint8_t u8Mode;
  uint8_t u8BitsPerWord;
  uint32_t u32Speed;
};
