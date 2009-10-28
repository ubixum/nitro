// Copyright (C) 2009 Ubixum, Inc. 
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
/**
 * \mainpage Nitrogen Data Acquisition Project API 
 * 
 * The Nitrogen Data Acquisition Project seeks to provide simple USB
 * data acquisition and communication with generic peripheral devices.
 * This is accomplished via three parts:
 *
 * \li <b>\ref board "Data Acqusition Board":</b> 
 *  The PCB contains a generic port for plugging in custom
 *   peripherals, an FPGA, and a USB microprocessor.  
 *
 * \li <b>\ref hwapi "RTL (Verilog) API":</b>
 *   A simple RTL API is used to send and receive data between the FPGA and the USB Microprocessor.
 *
 * \li <b>\ref swapi "Software (C) API":</b>
 *   A simple software API provides communication/control access from the PC 
 *   to the RTL software running on the FPGA.
 *
 * With the simple and generic RTL (Verilog) and software (C) API, a
 * user can easily communicate with peripheral devices and avoid the
 * complexities of USB communications completely.
 *
 * \image html board.png
 *
 * \section board Nitrogen Data Acquisition Board
 *
 *
 * The Nitrogen data acquistion board is a generic USB data
 * acquisition system.  It contains the following hardware:
 *
 * \li USB Microprocessor\n The USB Microprocessor controls IO between
 *      the PC and the board.  All USB traffic is routed through the
 *      microprocessor.  The microprocessor controls communication
 *      with the FPGA and any periphals devices.
 *
 * \li FPGA\n The FPGA is connected to the microprocessor and any
 *      perperal devices connected as daughter boards.  It can
 *      programmed to perform complex control and data management
 *      schemes such as providing double-buffered access to an image
 *      sensor data stream via external SDRAM.
 *
 * \li Peripheral Interface\n
 *     The Nitrogen board contains a connector that allows for generic
 *     peripheral connections as daughter boards to the FPGA.
 *
 * \section drivers Software Installation
 *
 * On every platform, an install package is provided to install the USB device driver required
 * to communicate with a Nitrogen device.  The device drivers must be correctly installed 
 * before the data acquisition software can communicate with the hardware.
 *
 * In addition to device drivers, a \ref nitro.cpp "test program" is provided.  The test program is capable of 
 * getting, setting, reading and writing to/from terminal registers as well as resetting the 
 * device firmware.
 *
 *
 * \section api API Overview

 * The RTL and software API is designed to enable generic
 * communication from the PC to any number of perphiral devices
 * accessed via the board.  Each peripheral is assigned a \e terminal
 * address.  Any number of configurable terminals can be created in
 * the FPGA and in the microprocessor.  Furthermore, each terminal can
 * have a number of registers for further control within each terminal.
 * Terminals and registers are integer based addresses.  Some
 * terminals provide specific registers for different functions.
 * Other terminals only have one function and either only provide one
 * register address, ignore the register address entirely, or use the
 * register address as a memory offset.
 *
 * \subsection hwapi RTL (Verilog) API
 *
 * The Nitrogen USB firmware provides a custom protocol for communicating with the FPGA.  
 * An RTL interface handles this protocol between the FPGA and Microprocessor.
 * This interface then provides generic read/write access to terminals controlled directly
 * by the FPGA.  The FPGA directly responds to the read/write/get/set requests that are
 * made through the software API. 
 *
 * \subsection swapi Software (C) API 
 *
 * In addition to installing device drivers, the installed software optionally contains include
 * files and language bindings for C++ and C#.
 * 
 * The basic order of operations to acquire data from a Nitrogen device are:
 * \li Create a device or devices to work with.  \see Nitro::Device Nitro::USBDevice
 * \li Optionally, add a Device Interface definition (xml) that allows
 *     getting and setting register values by name instead of address.  \see Nitro::Node Nitro::XmlReader Nitro::Device::set_tree
 * \li Make get/set and read/write calls to manipulate the device and read data.
 *     See \ref dataac
 * \li Close the device(s).
 *
 * The simple hello program is provided for an \ref hello.cpp "example" API usage.
 **/


/**
 * \page readme Project Readme
 * \verbinclude README
 **/
/**
 * \page changelog Project Changelog
 * \verbinclude CHANGELOG
 **/

/**
 * \example nitro.cpp
 * 
 * The nitro program uses command line options to set most of the parameters
 * used in the Nitro API calls.
 *
 *
 * \example hello.cpp
 *
 * A simple hello world example that shows how to open a device and read data.
 *
 *
 * \example serial.cpp
 *
 * A simple c example that shows how to query the number of devices available,
 * grap the device serial number, and open devices by the serial number.
 **/

#ifndef NITRO_H
#define NITRO_H

#include "nitro/version.h"
#include "nitro/error.h"
#include "nitro/types.h"
#include "nitro/node.h"
#include "nitro/usb.h"
#include "nitro/userdevice.h"
#include "nitro/xmlreader.h"
#include "nitro/xmlwriter.h"
#include "nitro/scripts.h"


#endif
