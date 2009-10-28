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

#include <pynitro/userdevice.h>

#include <structmember.h>

#include <iostream>
#include <string>


#include <pynitro/nitro_pyutil.h>

using namespace std;
using namespace Nitro;


static int
nitro_UserDevice_init(nitro_UserDeviceObject* self, PyObject *args, PyObject *kwds) {
    if ( nitro_DeviceType.tp_init((PyObject*)self, args, kwds) < 0) {
        return -1;
    }

    const char* path;
    PyObject* user_args=NULL; 
    PyObject* cobj=NULL;

    if (!PyArg_ParseTuple(args,"s|OO",&path,&user_args, &cobj)) {
        PyErr_SetString ( PyExc_Exception, "UserDevice( path, [arg0,[arg1,..]], [cobject] )" );
        return -1;
    }

    if (NULL != user_args && !PyList_Check(user_args)) {
        PyErr_SetString ( PyExc_Exception, "2nd argument is list of Strings to pass to user device." );
        return -1;
    }
    const char** str_args=NULL;
    if ( NULL != user_args ) {
        int len = PyList_Size(user_args);
        str_args = new const char* [ len + 1];
        str_args[len] = NULL;
        for (int i=0;i<len; ++i ) {
            PyObject* str = PyList_GetItem(user_args,i);
            if (!PyString_Check(str)) {
                delete [] str_args;
                PyErr_SetString ( PyExc_Exception, "Non String object in list" );
                return NULL;
            }
            str_args[i] = PyString_AsString(str);
        }
    } else {
        str_args = new const char* [1];
        str_args[0] = NULL;
    }

    void* ptr = NULL;
    if (NULL != cobj && PyCObject_Check(cobj)) {
        ptr = PyCObject_AsVoidPtr(cobj);
    }

    try {
        self->dev_base.nitro_device = new UserDevice(path, str_args, ptr);
    } catch ( const Exception &e) {
        if ( str_args ) delete [] str_args;
        NITRO_EXC(e,-1); 
    }
    if ( str_args ) delete [] str_args;
    return 0;
}

PyTypeObject nitro_UserDeviceType = {

    PyObject_HEAD_INIT(NULL)
    0,                         /*ob_size*/
    "nitro.UserDevice",            /*tp_name*/
    sizeof(nitro_UserDeviceObject),  /*tp_basicsize*/
    0,                         /*tp_itemsize*/
    0,                         /*tp_dealloc*/
    0,                         /*tp_print*/
    0,                         /*tp_getattr*/
    0,                         /*tp_setattr*/
    0,                         /*tp_compare*/
    0,                         /*tp_repr*/
    0,                         /*tp_as_number*/
    0,                         /*tp_as_sequence*/
    0,                         /*tp_as_mapping*/
    0,                         /*tp_hash */
    0,                         /*tp_call*/
    0,                         /*tp_str*/
    0,                         /*tp_getattro*/
    0,                         /*tp_setattro*/
    0,                         /*tp_as_buffer*/
    Py_TPFLAGS_DEFAULT,        /*tp_flags*/
    "Nitro UserDevice Object",           /* tp_doc */
    0,                    /* tp_traverse */
    0,                     /* tp_clear */
    0,                     /* tp_richcompare */
    0,                     /* tp_weaklistoffset */
    0,                     /* tp_iter */
    0,                     /* tp_iternext */
    0,   /* tp_methods */
    0,                       /* tp_members */
    0,                         /* tp_getset */
    0,                         /* tp_base */
    0,                         /* tp_dict */
    0,                         /* tp_descr_get */
    0,                         /* tp_descr_set */
    0,                         /* tp_dictoffset */
    (initproc)nitro_UserDevice_init,  /* tp_init */
    0,                         /* tp_alloc */
    0,                 /* tp_new */
};


