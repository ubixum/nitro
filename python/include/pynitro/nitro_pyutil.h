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

#ifndef PYNITRO_UTIL_H
#define PYNITRO_UTIL_H


#if PY_MINOR_VERSION <= 4
typedef int Py_ssize_t;
#define charbufferproc getcharbufferproc
#endif

#include <nitro/types.h>
#include <nitro/error.h>

extern PyObject* nitro_Exception;

/**
 * Use the the catch block when catching Nitro::Exception to format the 
 * Exception and return a python exception.
 * \code
 * try {
 *  ...
 * } catch ( const Nitro::Exception& e ) {
 *  NITRO_EXC(e,NULL);
 * }
 * \endcode
 *
 * \param e The Nitro::Exception
 * \param r The return value
 *
 **/

#define SET_NITRO_EXC(e){\
        PyObject* err_ud=from_datatype((e).userdata()); \
        PyErr_SetObject(nitro_Exception, Py_BuildValue( "isN", (e).code(), (e).str_error().c_str(), err_ud ) );}

#define NITRO_EXC(e,r) \
        SET_NITRO_EXC(e); \
        return r;

/**
 *  \brief convert PyObject to DataType.
 *  \param address, Pointer to DataType memory
 *  \return 0 on success or -1 if failure. (And calls PyErr_SetString on failure)
 **/
int to_datatype(PyObject *object, void *address);

/**
 * \brief convert DataType to PyObject.
 * \param dt the DataType to convert
 * \return an object or NULL. (And sets PyErr_SetString on failure)
 **/
PyObject* from_datatype(const Nitro::DataType& dt); 

#endif
