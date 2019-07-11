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
            if (!PyBytes_Check(str)) {
                delete [] str_args;
                PyErr_SetString ( PyExc_Exception, "Non String object in list" );
                return 0;
            }
            str_args[i] = PyBytes_AsString(str);
        }
    } else {
        str_args = new const char* [1];
        str_args[0] = NULL;
    }

    void* ptr = NULL;
    if (NULL != cobj && PyCapsule_CheckExact(cobj)) {
      ptr = PyCapsule_GetPointer(cobj, NULL);
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

#if PY_MAJOR_VERSION >= 3
PyTypeObject nitro_UserDeviceType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "nitro.UserDevice",// char *tp_name; For printing, in format "<module>.<name>"
    sizeof(nitro_UserDeviceObject),//Py_ssize_t tp_basicsize, 
    NULL, //tp_itemsize; /* For allocation */

    /* Methods to implement standard operations */
    NULL, //destructor tp_dealloc;
    NULL, //printfunc tp_print;
    NULL, //getattrfunc tp_getattr;
    NULL, //setattrfunc tp_setattr;
    NULL, //PyAsyncMethods *tp_as_async; formerly known as tp_compare (Python 2) or tp_reserved (Python 3)
    NULL, //reprfunc tp_repr;

    /* Method suites for standard classes */
    NULL, //PyNumberMethods *tp_as_number;
    NULL, //PySequenceMethods *tp_as_sequence;
    NULL, //PyMappingMethods *tp_as_mapping;
    
    /* More standard operations (here for binary compatibility) */
    NULL, //hashfunc tp_hash;
    NULL, //ternaryfunc tp_call;
    NULL, //reprfunc tp_str;
    NULL, //getattrofunc tp_getattro;
    NULL, //setattrofunc tp_setattro;

    /* Functions to access object as input/output buffer */
    NULL, //PyBufferProcs *tp_as_buffer;

    /* Flags to define presence of optional/expanded features */
    Py_TPFLAGS_DEFAULT,//unsigned long tp_flags;

    "Nitro UserDevice Object",//const char *tp_doc; /* Documentation string */

    /* call function for all accessible objects */
    NULL, //traverseproc tp_traverse;

    /* delete references to contained objects */
    NULL, //inquiry tp_clear;

    /* rich comparisons */
    NULL, //richcmpfunc tp_richcompare;

    /* weak reference enabler */
    NULL, //Py_ssize_t tp_weaklistoffset;

    /* Iterators */
    NULL, //getiterfunc tp_iter;
    NULL, //iternextfunc tp_iternext;

    /* Attribute descriptor and subclassing stuff */
    NULL, //struct PyMethodDef *tp_methods;
    NULL, //struct PyMemberDef *tp_members;
    NULL, //struct PyGetSetDef *tp_getset;
    NULL, //struct _typeobject *tp_base;
    NULL, //PyObject *tp_dict;
    NULL, //descrgetfunc tp_descr_get;
    NULL, //descrsetfunc tp_descr_set;
    NULL, //Py_ssize_t tp_dictoffset;
    (initproc)nitro_UserDevice_init, //initproc tp_init;
    NULL, //allocfunc tp_alloc;
    NULL, //newfunc tp_new;
    NULL, //freefunc tp_free; /* Low-level free-memory routine */
    NULL, //inquiry tp_is_gc; /* For PyObject_IS_GC */
    NULL, //PyObject *tp_bases;
    NULL, //PyObject *tp_mro; /* method resolution order */
    NULL, //PyObject *tp_cache;
    NULL, //PyObject *tp_subclasses;
    NULL, //PyObject *tp_weaklist;
    NULL, //destructor tp_del;
    
    ///* Type attribute cache version tag. Added in version 2.6 */
    NULL, //unsigned int tp_version_tag;
    
    NULL //destructor tp_finalize;
};

#else
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
#endif

