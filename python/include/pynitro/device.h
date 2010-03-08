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

#ifndef NPYDEVICE_H
#define NPYDEVICE_H

#include <nitro.h>


class PyRetryFunc; // predef

/**
 * The basic device type
 *
 **/

typedef struct {
    PyObject_HEAD
    Nitro::Device* nitro_device;
    PyRetryFunc* retry_func;
} nitro_DeviceObject;


extern PyTypeObject nitro_DeviceType;

PyObject* nitro_Device_GetDi(nitro_DeviceObject* self);
PyObject* nitro_Device_SetDi(nitro_DeviceObject* self, PyObject *arg);
PyObject* nitro_Device_Lock(nitro_DeviceObject* self);
PyObject* nitro_Device_Unlock(nitro_DeviceObject* self);
PyObject* nitro_Device_Get(nitro_DeviceObject* self, PyObject *args);
PyObject* nitro_Device_GetSubregs(nitro_DeviceObject* self, PyObject *args);
PyObject* nitro_Device_Set(nitro_DeviceObject* self, PyObject *args);
PyObject* nitro_Device_Read(nitro_DeviceObject* self, PyObject *args);
PyObject* nitro_Device_Write(nitro_DeviceObject* self, PyObject *args);
PyObject* nitro_Device_Close(nitro_DeviceObject* self);
PyObject* nitro_Device_LoadXML(nitro_DeviceObject*, PyObject *args);
PyObject* nitro_Device_WriteXML(nitro_DeviceObject*, PyObject *arg);
PyObject* nitro_Device_EnableMode(nitro_DeviceObject* self, PyObject *args);
PyObject* nitro_Device_DisableMode(nitro_DeviceObject* self, PyObject *args);
PyObject* nitro_Device_SetModes(nitro_DeviceObject* self, PyObject *args);
PyObject* nitro_Device_GetModes(nitro_DeviceObject* self, PyObject *args);
PyObject* nitro_Device_SetTimeout(nitro_DeviceObject* self, PyObject *arg);
PyObject* nitro_Device_SetRetryFunc(nitro_DeviceObject* self, PyObject *arg);
#endif


