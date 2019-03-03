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

#ifndef PYTHON_NITRO_H
#define PYTHON_NITRO_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Api for using the nitro module in C.
 **/

typedef enum {
    NITRO_TO_DATATYPE,
    NITRO_FROM_DATATYPE,
    NITRO_EXCEPTION,
    NITRO_BUILD_BUFFER,
    NITRO_CAPI_N_POINTERS
} NitroCAPIPointers;

#ifndef NITRO_MODULE

static void **NitroCAPI;

/**
 * void nitro_to_datatype ( PyObject*, void* );
 * PyObject* nitro_from_datatype ( const DataType& );
 *
 **/
#define nitro_to_datatype \
    (* (int (*)(PyObject*, void*)) NitroCAPI[NITRO_TO_DATATYPE])
#define nitro_from_datatype \
    (* (PyObject* (*)(const Nitro::DataType&)) NitroCAPI[NITRO_FROM_DATATYPE])

#define nitro_Exception \
    (PyObject*) NitroCAPI[NITRO_EXCEPTION] 

#define nitro_BuildBuffer \
    (* (PyObject* (*)(uint8*, uint32)) NitroCAPI[NITRO_BUILD_BUFFER])

/**
 *  This function imports the nitro module and assigns the 
 *  exported functions for use in the client.
 **/
static int 
import_nitro() {
    NitroCAPI = (void **)PyCapsule_Import("nitro._NITRO_API", 0);
    return (NitroCAPI != 0) ? 0 : -1;
}

#endif // end capi import


#ifdef __cplusplus
} // end extern "C"
#endif


#endif // end header

