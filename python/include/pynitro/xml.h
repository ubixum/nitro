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

#ifndef PYXML_H
#define PYXML_H

#include <nitro.h>

/**
 *  XmlReader/Writer Mappings 
 **/

typedef struct {
    PyObject_HEAD
    Nitro::XmlReader* m_reader;
} nitro_XmlReaderObject;

typedef struct {
    PyObject_HEAD
    Nitro::XmlWriter* m_writer;
} nitro_XmlWriterObject;


extern PyTypeObject nitro_XmlReaderType;
extern PyTypeObject nitro_XmlWriterType;

#endif


