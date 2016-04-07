// * Copyright (c) 2013 Shinichiro Nakamura (https://github.com/shintamainjp)
// > Updated April. 7, 2016, George KC, Converting code to Arduino version. (https://github.com/georgekc)
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

#include "SPIFlash.h"

SPIFlash::SPIFlash(uint8_t cspin)
{
  _cspin = cspin;
  pinMode(_cspin, OUTPUT);
  digitalWrite(_cspin, HIGH);  // CS pin is high when idle.
  SPI.begin();
  SPIset();
  digitalWrite(_cspin, LOW);
  SPI.transfer(SPI_WRITE_ENABLE);
  digitalWrite(_cspin, HIGH);
}
void SPIFlash::SPIset()
{
  SPI.setBitOrder(MSBFIRST);
  SPI.setDataMode(SPI_MODE0);
  SPI.setClockDivider(SPI_CLOCK_DIV2);
}
/**
   @brief Write Enable.
   @details
   The WRITE ENABLE command sets the write enable latch (WEL) bit.
   The WEL bit must be set before execution of every PROGRAM, ERASE, and WRITE command.
   The WRITE ENABLE command is entered by driving chip select (S#) LOW, sending the command code, and then driving S# HIGH.
*/
void SPIFlash::flash_write_enable(void)
{
  SPI_ASSERT();
  SPI.transfer(SPI_WRITE_ENABLE);
  SPI_DEASSERT();
}
/**
   @brief Write Disable.
   @details
   The WRITE DISABLE command resets the write enable latch (WEL) bit.
   The WRITE DISABLE command is entered by driving chip select (S#) LOW, sending the
   command code, and then driving S# HIGH.
   The WEL bit is reset under the following conditions:
   - Power-up
   - Completion of any ERASE operation
   - Completion of any PROGRAM operation
   - Completion of any WRITE REGISTER operation
   - Completion of WRITE DISABLE operation
*/
void SPIFlash::flash_write_disable(void)
{
  SPI_ASSERT();
  SPI.transfer(SPI_WRITE_DISABLE);
  SPI_DEASSERT();
}

/**
   @brief Read Identification.
   @details
   The READ IDENTIFICATION command reads the following device identification data:
   - Manufacturer identification (1 byte): This is assigned by JEDEC.
   - Device identification (2 bytes):
     This is assigned by device manufacturer;
     the first byte indicates memory type and the second byte indicates device memory capacity.
   - A Unique ID code (UID) (17 bytes,16 available upon customer request):
     The first byte contains length of data to follow;
     the remaining 16 bytes contain optional Customized Factory Data (CFD) content.
*/

void SPIFlash::flash_read_identification(struct flash_identification *p)
{
  int i;
  SPI_ASSERT();
  SPI.transfer(SPI_READ_IDENTIFICATION);
  p->manufacturer = SPI.transfer(0);
  p->memory_type = SPI.transfer(0);
  p->memory_capacity = SPI.transfer(0);
  p->cfd_length = SPI.transfer(0);
  for (i = 0; i < p->cfd_length; i++) {
    p->cfd_content[i] = SPI.transfer(0);
  }
  SPI_DEASSERT();
}

/**
   @brief Read Status Register.
   @details
   The READ STATUS REGISTER command allows the status register to be read.
   The status register may be read at any time, even while a PROGRAM, ERASE, or WRITE STATUS REGISTER cycle is in progress.
   When one of these cycles is in progress, it is recommended to check the write in progress (WIP) bit before sending a new command to the device.
   It is also possible to read the status register continuously.
*/
void SPIFlash::flash_read_status_register(uint8_t *sreg)
{
  SPI_ASSERT();
  SPI.transfer(SPI_READ_STATUS_REGISTER);
  *sreg = SPI.transfer(0);
  SPI_DEASSERT();
}

/**
   @brief Write Status Register.
   @details
   The WRITE STATUS REGISTER command allows new values to be written to the status register.
   Before the WRITE STATUS REGISTER command can be accepted, a WRITE ENABLE command must have been executed previously.
   After the WRITE ENABLE command has been decoded and executed, the device sets the write enable latch (WEL) bit.
   The WRITE STATUS REGISTER command is entered by driving chip select (S#) LOW,
   followed by the command code and the data byte on serial data input (DQ0).
   The WRITE STATUS REGISTER command has no effect on b6, b5, b1 and b0 of the status register.
   The status register b6 and b5 are always read as '0'.
   S# must be driven HIGH after the eighth bit of the data byte has been latched in.
   If not, the WRITE STATUS REGISTER command is not executed.
*/
void SPIFlash::flash_write_status_register(uint8_t sreg)
{
  SPI_ASSERT();
  SPI.transfer(SPI_WRITE_STATUS_REGISTER);
  SPI.transfer(sreg);
  SPI_DEASSERT();
}
/**
   @brief Read Data Bytes.
   @details
   The device is first selected by driving chip select (S#) LOW.
   The command code for READ DATA BYTES is followed by a 3-byte address (A23-A0),
   each bit being latched-in during the rising edge of serial clock (C).
   Then the memory contents at that address is shifted out on serial data output (DQ1),
   each bit being shifted out at a maximum frequency fR during the falling edge of C.
   The first byte addressed can be at any location.
   The address is automatically incremented to the next higher address after each byte of data is shifted out.
   Therefore, the entire memory can be read with a single READ DATA BYTES command.
   When the highest address is reached, the address counter rolls over to 000000h, allowing the read sequence to be continued indefinitely.

   The READ DATA BYTES command is terminated by driving S# HIGH. S# can be driven HIGH at any time during data output.
   Any READ DATA BYTES command issued while an ERASE, PROGRAM, or WRITE cycle is in progress is rejected without any effect on the cycle that is in progress.
*/
void SPIFlash::flash_read_data_bytes(uint32_t addr, uint8_t *buf, uint32_t siz)
{
  uint32_t i;
  SPI_ASSERT();
  SPI.transfer(SPI_READ_DATA_BYTES);
  SPI.transfer(addr >> 16);
  SPI.transfer(addr >>  8);
  SPI.transfer(addr >>  0);
  for (i = 0; i < siz; i++) {
    buf[i] = SPI.transfer(0);
  }
  SPI_DEASSERT();
}
/**
   @brief Page Program.
   @details
   The PAGE PROGRAM command allows bytes in the memory to be programmed,
   which means the bits are changed from 1 to 0.
   Before a PAGE PROGRAM command can be accepted a WRITE ENABLE command must be executed.
   After the WRITE ENABLE command has been decoded, the device sets the write enable latch (WEL) bit.
   The PAGE PROGRAM command is entered by driving chip select (S#) LOW,
   followed by the command code, three address bytes, and at least one data byte on serial data input (DQ0).
   If the eight least significant address bits (A7-A0) are not all zero,
   all transmitted data that goes beyond the end of the current page are programmed from the start address of the same page;
   that is, from the address whose eight least significant bits (A7-A0) are all zero.
   S# must be driven LOW for the entire duration of the sequence.
   If more than 256 bytes are sent to the device,
   previously latched data are discarded and the last 256 data bytes are guaranteed to be programmed correctly within the same page.
   If less than 256 data bytes are sent to device,
   they are correctly programmed at the requested addresses without any effects on the other bytes of the same page.
   For optimized timings, it is recommended to use the PAGE PROGRAM command to program all consecutive targeted bytes
   in a single sequence rather than to use several PAGE PROGRAM sequences, each containing only a few bytes.
   S# must be driven HIGH after the eighth bit of the last data byte has been latched in.
   Otherwise the PAGE PROGRAM command is not executed.
   As soon as S# is driven HIGH, the self-timed PAGE PROGRAM cycle is initiated; the cycles's duration is tPP.
   While the PAGE PROGRAM cycle is in progress, the status register may be read to check the value of the write in progress (WIP) bit.
   The WIP bit is 1 during the self-timed PAGE PROGRAM cycle, and 0 when the cycle is completed.
   At some unspecified time before the cycle is completed, the write enable latch (WEL) bit is reset.
   A PAGE PROGRAM command is not executed if it applies to a page protected by the block protect bits BP2, BP1, and BP0.
*/
void SPIFlash::flash_page_program(uint32_t addr, uint8_t *buf, uint32_t siz)
{
  uint32_t i;
  SPI_ASSERT();
  SPI.transfer(SPI_PAGE_PROGRAM);
  SPI.transfer(addr >> 16);
  SPI.transfer(addr >>  8);
  SPI.transfer(addr >>  0);
  for (i = 0; i < siz; i++) {
    SPI.transfer(buf[i]);
  }
  SPI_DEASSERT();
}
/**
   @brief Sector Erase.
   @details
   The SECTOR ERASE command sets to 1 (FFh) all bits inside the chosen sector.
   Before the SECTOR ERASE command can be accepted, a WRITE ENABLE command must have been executed previously.
   After the WRITE ENABLE command has been decoded, the device sets the write enable latch (WEL) bit.
   The SECTOR ERASE command is entered by driving chip select (S#) LOW, followed
   by the command code, and three address bytes on serial data input (DQ0).
   Any address inside the sector is a valid address for the SECTOR ERASE command.
   S# must be driven LOW for the entire duration of the sequence.
   S# must be driven HIGH after the eighth bit of the last address byte has been latched in.
   Otherwise the SECTOR ERASE command is not executed. As soon as S# is driven HIGH,
   the self-timed SECTOR ERASE cycle is initiated; the cycle's duration is tSE.
   While the SECTOR ERASE cycle is in progress, the status register may be read to check the value of the write in progress (WIP) bit.
   The WIP bit is 1 during the self-timed SECTOR ERASE cycle, and is 0 when the cycle is completed.
   At some unspecified time before the cycle is completed, the WEL bit is reset.
   A SECTOR ERASE command is not executed if it applies to a sector that is hardware or software protected.
*/
void SPIFlash::flash_sector_erase(uint32_t addr)
{
  SPI_ASSERT();
  SPI.transfer(SPI_SECTOR_ERASE);
  SPI.transfer(addr >> 16);
  SPI.transfer(addr >>  8);
  SPI.transfer(addr >>  0);
  SPI_DEASSERT();
}

/**
   @brief Bulk Erase.
   @details
   The BULK ERASE command sets all bits to 1 (FFh).
   Before the BULK ERASE command can be accepted, a WRITE ENABLE command must have been executed previously.
   After the WRITE ENABLE command has been decoded, the device sets the write enable latch (WEL) bit.
   The BULK ERASE command is entered by driving chip select (S#) LOW,
   followed by the command code on serial data input (DQ0).
   S# must be driven LOW for the entire duration of the sequence.
   S# must be driven HIGH after the eighth bit of the command code has been latched in.
   Otherwise the BULK ERASE command is not executed. As soon as S# is driven HIGH, the self-timed BULK ERASE cycle is initiated;
   the cycle's duration is tBE. While the BULK ERASE cycle is in progress,
   the status register may be read to check the value of the write In progress (WIP) bit.
   The WIP bit is 1 during the self-timed BULK ERASE cycle, and is 0 when the cycle is completed.
   At some unspecified time before the cycle is completed, the WEL bit is reset.
   The BULK ERASE command is executed only if all block protect (BP2, BP1, BP0) bits are 0.
   The BULK ERASE command is ignored if one or more sectors are protected.
*/
void SPIFlash::flash_bulk_erase(void)
{
  SPI_ASSERT();
  SPI.transfer(SPI_BULK_ERASE);
  SPI_DEASSERT();
}
/**
   @brief Deep Power Down.
   @details
   Executing the DEEP POWER-DOWN command is the only way to put the device in the lowest power consumption mode, the DEEP POWER-DOWN mode.
   The DEEP POWERDOWN command can also be used as a software protection mechanism while the device is not in active use
   because in the DEEP POWER-DOWN mode the device ignores all WRITE, PROGRAM, and ERASE commands.
   Driving chip select (S#) HIGH deselects the device, and puts it in the STANDBY POWER mode if there is no internal cycle currently in progress.
   Once in STANDBY POWER mode, the DEEP POWER-DOWN mode can be entered
   by executing the DEEP POWERDOWN command, subsequently reducing the standby current from ICC1 to ICC2.
   To take the device out of DEEP POWER-DOWN mode, the RELEASE from DEEP POWER-DOWN command must be issued.
   Other commands must not be issued while the device is in DEEP POWER-DOWN mode.
   The DEEP POWER-DOWN mode stops automatically at power-down. The device always powers up in STANDBY POWER mode.
   The DEEP POWER-DOWN command is entered by driving S# LOW, followed by the command code on serial data input (DQ0).
   S# must be driven LOW for the entire duration of the sequence.
   S# must be driven HIGH after the eighth bit of the command code has been latched in.
   Otherwise the DEEP POWER-DOWN command is not executed. As soon as S# is driven HIGH,
   it requires a delay of tDP before the supply current is reduced to ICC2 and the DEEP POWER-DOWN mode is entered.
   Any DEEP POWER-DOWN command issued while an ERASE, PROGRAM, or WRITE cycle is in progress is rejected
   without any effect on the cycle that is in progress.
*/
void SPIFlash::flash_deep_power_down(void)
{
  SPI_ASSERT();
  SPI.transfer(SPI_DEEP_POWER_DOWN);
  SPI_DEASSERT();
}
/**
   @brief Release From Deep Power Down.
   @details
   Once the device has entered DEEP POWER-DOWN mode, all commands are ignored except RELEASE from DEEP POWER-DOWN and READ ELECTRONIC SIGNATURE.
   Executing either of these commands takes the device out of the DEEP POWER-DOWN mode.
   The RELEASE from DEEP POWER-DOWN command is entered by driving chip select (S#) LOW,
   followed by the command code on serial data input (DQ0).
   S# must be driven LOW for the entire duration of the sequence.
   The RELEASE from DEEP POWER-DOWN command is terminated by driving S# HIGH.
   Sending additional clock cycles on serial clock C while S# is driven LOW causes the command to be rejected and not executed.
   After S# has been driven HIGH, followed by a delay, tRES, the device is put in the STANDBY mode.
   S# must remain HIGH at least until this period is over. The device waits to be selected so that it can receive, decode, and execute commands.
   Any RELEASE from DEEP POWER-DOWN command issued while an ERASE, PROGRAM, or WRITE cycle is in progress is rejected without any effect on the cycle that is in progress.
*/
void SPIFlash::flash_release_from_deep_power_down(void)
{
  SPI_ASSERT();
  SPI.transfer(SPI_RELEASE_FROM_DEEP_POWER_DOWN);
  SPI_DEASSERT();
}
