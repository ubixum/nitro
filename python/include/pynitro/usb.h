/**
 * Copyright (C) 2009 Ubixum, Inc. 
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 **/


#include <Python.h>

#ifndef PYUSB_H
#define PYUSB_H

#include "device.h"

/**
 * The usb device type
 *
 **/

typedef struct {
    nitro_DeviceObject dev_base; 
    PyObject* wrapped_dev; // allow for wrapping another device
} nitro_USBDeviceObject;


extern PyTypeObject nitro_USBDeviceType;

PyObject* nitro_USBDevice_Open(nitro_USBDeviceObject* self, PyObject* args);
PyObject* nitro_USBDevice_OpenBySerial(nitro_USBDeviceObject* self, PyObject* args);
PyObject* nitro_USBDevice_OpenByAddress(nitro_USBDeviceObject* self, PyObject* args);
PyObject* nitro_USBDevice_IsOpen(nitro_USBDeviceObject* self);
PyObject* nitro_USBDevice_Renum(nitro_USBDeviceObject* self);
PyObject* nitro_USBDevice_Reset(nitro_USBDeviceObject* self);
PyObject* nitro_USBDevice_LoadFirmware(nitro_USBDeviceObject* self, PyObject* args);
PyObject* nitro_USBDevice_GetFirmwareVersion(nitro_USBDeviceObject* self);
PyObject* nitro_USBDevice_SetSerial(nitro_USBDeviceObject* self, PyObject *arg);
PyObject* nitro_USBDevice_GetSerial(nitro_USBDeviceObject* self, PyObject* args);
PyObject* nitro_USBDevice_GetAddress(nitro_USBDeviceObject* self, PyObject* args);
PyObject* nitro_USBDevice_GetVid(nitro_USBDeviceObject* self, PyObject* args);
PyObject* nitro_USBDevice_GetPid(nitro_USBDeviceObject* self, PyObject* args);

//*********** static functions ****************

PyObject* nitro_USBDevice_GetDeviceCount(nitro_USBDeviceObject* self, PyObject* args);
PyObject* nitro_USBDevice_GetDeviceList(nitro_USBDeviceObject* self, PyObject* args);
PyObject* nitro_USBDevice_GetDeviceSerial(nitro_USBDeviceObject* self, PyObject* args);
PyObject* nitro_USBDevice_GetDeviceAddress(nitro_USBDeviceObject* self, PyObject* args);


#endif


