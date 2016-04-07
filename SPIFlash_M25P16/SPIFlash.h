// * Copyright (c) 2013 Shinichiro Nakamura (https://github.com/shintamainjp)
// > Updated April. 7, 2016, George KC, Arduino version. (https://github.com/georgekc)
// **********************************************************************************
// License
// **********************************************************************************
// This program is free software; you can redistribute it
// and/or modify it under the terms of the GNU General
// Public License as published by the Free Software
// Foundation; either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will
// be useful, but WITHOUT ANY WARRANTY; without even the
// implied warranty of MERCHANTABILITY or FITNESS FOR A
// PARTICULAR PURPOSE. See the GNU General Public
// License for more details.
//
// You should have received a copy of the GNU General
// Public License along with this program.
// If not, see <http://www.gnu.org/licenses/>.
//
// Licence can be viewed at
// http://www.gnu.org/licenses/gpl-3.0.txt
//
// Please maintain this license information along with authorship
// and copyright notices in any redistribution of this code

#ifndef _SPIFLASH_H_
#define _SPIFLASH_H_


#include <Arduino.h>
#include <SPI.h>
#include <stdint.h>

/// IMPORTANT: NAND FLASH memory requires erase before write, because
///            it can only transition from 1s to 0s and only the erase command can reset all 0s to 1s
/// See http://en.wikipedia.org/wiki/Flash_memory



#define SPI_WRITE_ENABLE                  0x06
#define SPI_WRITE_DISABLE                 0x04
#define SPI_SECTOR_ERASE                  0xD8
#define SPIFLASH_CHIPERASE                0x60
#define SPI_READ_STATUS_REGISTER          0x05
#define SPI_WRITE_STATUS_REGISTER         0x01
#define SPIFLASH_ARRAYREAD                0x0B
#define SPI_READ_DATA_BYTES               0x03
#define SPI_BULK_ERASE                    0xC7
#define SPI_DEEP_POWER_DOWN               0xB9
#define SPI_RELEASE_FROM_DEEP_POWER_DOWN  0xAB
#define SPI_PAGE_PROGRAM                  0x02
#define SPI_READ_IDENTIFICATION           0x9F
#define SPIFLASH_MACREAD                  0x4B



#define flash_PAGE_COUNT       (8192)
#define flash_PAGE_BYTE_SIZE   (256)
#define flash_SECTOR_COUNT     (32)
#define flash_SECTOR_BYTE_SIZE (65536)

/**
   @brief Write Protect.
   @details
   The status register write disable (SRWD) bit is operated in conjunction with the write protect (W#/VPP) signal.
   When the SRWD bit is set to 1 and W#/VPP is driven LOW, the device is put in the hardware protected mode.
   In the hardware protected mode, the non-volatile bits of the status register (SRWD, and the block protect bits)
   become readonly bits and the WRITE STATUS REGISTER command is no longer accepted for execution.
*/
#define FLASH_SREG_WRITE_PROTECT(SREG)         ((SREG) & (1 << 7))

/**
   @brief Block Protect 2.
   @details
   The block protect bits are non-volatile.
   They define the size of the area to be software protected against PROGRAM and ERASE commands.
   The block protect bits are written with the WRITE STATUS REGISTER command.
   When one or more of the block protect bits is set to 1, the relevant memory area,
   as defined in the Protected Area Sizes table, becomes protected against PAGE PROGRAM and SECTOR ERASE commands.
   The block protect bits can be written provided that the HARDWARE PROTECTED mode has not been set.
   The BULK ERASE command is executed only if all block protect bits are 0.
*/
#define FLASH_SREG_BLOCK_PROTECT_BP2(SREG)     ((SREG) & (1 << 4))

/**
   @brief Block Protect 1.
   @details
   The block protect bits are non-volatile.
   They define the size of the area to be software protected against PROGRAM and ERASE commands.
   The block protect bits are written with the WRITE STATUS REGISTER command.
   When one or more of the block protect bits is set to 1, the relevant memory area,
   as defined in the Protected Area Sizes table, becomes protected against PAGE PROGRAM and SECTOR ERASE commands.
   The block protect bits can be written provided that the HARDWARE PROTECTED mode has not been set.
   The BULK ERASE command is executed only if all block protect bits are 0.
*/
#define FLASH_SREG_BLOCK_PROTECT_BP1(SREG)     ((SREG) & (1 << 3))

/**
   @brief Block Protect 0.
   @details
   The block protect bits are non-volatile.
   They define the size of the area to be software protected against PROGRAM and ERASE commands.
   The block protect bits are written with the WRITE STATUS REGISTER command.
   When one or more of the block protect bits is set to 1, the relevant memory area,
   as defined in the Protected Area Sizes table, becomes protected against PAGE PROGRAM and SECTOR ERASE commands.
   The block protect bits can be written provided that the HARDWARE PROTECTED mode has not been set.
   The BULK ERASE command is executed only if all block protect bits are 0.
*/
#define FLASH_SREG_BLOCK_PROTECT_BP0(SREG)     ((SREG) & (1 << 2))

/**
   @brief Write Enable Latch.
   @details
   The write enable latch (WEL) bit indicates the status of the internal write enable latch.
   When the WEL bit is set to 1, the internal write enable latch is set; when the WEL bit is
   set to 0, the internal write enable latch is reset and no WRITE STATUS REGISTER, PROGRAM, or ERASE command is accepted.
*/
#define FLASH_SREG_WRITE_ENABLE_LATCH(SREG)    ((SREG) & (1 << 1))

/**
   @brief Write In Progress Flag.
   @details
   The write in progress (WIP) bit indicates whether the memory is busy with a WRITE
   STATUS REGISTER cycle, a PROGRAM cycle, or an ERASE cycle. When the WIP bit is set
   to 1, a cycle is in progress; when the WIP bit is set to 0, a cycle is not in progress.
*/
#define FLASH_SREG_WRITE_IN_PROGRESS(SREG)     ((SREG) & (1 << 0))


#define SPI_ASSERT()   digitalWrite(_cspin, LOW);
#define SPI_DEASSERT()  digitalWrite(_cspin, HIGH);

typedef struct flash_identification {
  uint8_t manufacturer;
  uint8_t memory_type;
  uint8_t memory_capacity;
  uint8_t cfd_length;
  uint8_t cfd_content[16];
} flash_identification_t;
class SPIFlash {
  public:
    SPIFlash(uint8_t cspin);
    void SPIset();
    void flash_write_enable(void);
    void flash_write_disable(void);
    void flash_read_identification(struct flash_identification *p);
    void flash_read_status_register(uint8_t *sreg);
    void flash_write_status_register(uint8_t sreg);
    void flash_read_data_bytes(uint32_t addr, uint8_t *buf, uint32_t siz);
    void flash_page_program(uint32_t addr, uint8_t *buf, uint32_t siz);
    void flash_sector_erase(uint32_t addr);
    void flash_bulk_erase(void);
    void flash_deep_power_down(void);
    void flash_release_from_deep_power_down(void);
  protected:

    uint16_t lastPage;
    uint32_t pointer;
    uint16_t _pagesize;
    uint32_t _highestaddr;
    uint8_t _cspin;

};

#endif
