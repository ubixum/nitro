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

#ifndef PYNODE_H
#define PYNODE_H

#include <nitro.h>

/**
 * Node Mappings 
 *
 **/

typedef struct {
    PyObject_HEAD
    Nitro::NodeRef* m_node;
} nitro_NodeObject;


typedef struct {
    nitro_NodeObject node_base;
} nitro_DeviceInterfaceObject;

typedef struct {
 nitro_NodeObject node_base;
} nitro_TerminalObject;

typedef struct {
    nitro_NodeObject node_base;
} nitro_RegisterObject;

typedef struct {
    nitro_NodeObject node_base;
} nitro_SubregisterObject;


typedef struct {
    nitro_NodeObject node_base;
} nitro_ValuemapObject;


extern PyTypeObject nitro_NodeType;
extern PyTypeObject nitro_DeviceInterfaceType;
extern PyTypeObject nitro_TerminalType;
extern PyTypeObject nitro_RegisterType;
extern PyTypeObject nitro_SubregisterType;
extern PyTypeObject nitro_ValuemapType;


PyObject* nitro_BuildNode(const Nitro::NodeRef& node);


#endif


