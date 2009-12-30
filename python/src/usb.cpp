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

#include <pynitro/usb.h>

#include <structmember.h>

#include <iostream>
#include <string>
#include <typeinfo>


#include <nitro.h>


#include <pynitro/nitro_pyutil.h>

using namespace std;
using namespace Nitro;

PyMethodDef nitro_USBDevice_methods[] = {
    {"open", (PyCFunction)nitro_USBDevice_Open, METH_VARARGS, "Wrapped C++ API member function" },
    {"open_by_serial", (PyCFunction)nitro_USBDevice_OpenBySerial, METH_VARARGS, "Wrapped C++ API member function" },
    {"open_by_address", (PyCFunction)nitro_USBDevice_OpenByAddress, METH_VARARGS, "open_by_address(addr) -> Open device by usb bus address." },
    {"set_serial", (PyCFunction)nitro_USBDevice_SetSerial, METH_O, "Wrapped C++ API member function" },
    {"get_serial", (PyCFunction)nitro_USBDevice_GetSerial, METH_NOARGS, "Wrapped C++ API member function" },
    {"get_vid", (PyCFunction)nitro_USBDevice_GetVid, METH_VARARGS, "get_vid() -> The Vendor Id" },
    {"get_pid", (PyCFunction)nitro_USBDevice_GetPid, METH_VARARGS, "get_pid() -> The Product Id" }, 
    {"get_device_serial", (PyCFunction)nitro_USBDevice_GetDeviceSerial, METH_STATIC|METH_VARARGS, "Wrapped C++ API member function" },
    {"get_device_count", (PyCFunction)nitro_USBDevice_GetDeviceCount, METH_STATIC|METH_VARARGS, "Wrapped C++ API member function" },
    {"get_device_address", (PyCFunction)nitro_USBDevice_GetDeviceAddress, METH_STATIC|METH_VARARGS, "get_device_address(vid,pid,index) -> Static method for retrieving device address." },
    {"get_address", (PyCFunction)nitro_USBDevice_GetAddress, METH_NOARGS, "get_device_address() -> Member method to retrieve device address." },
    {"is_open", (PyCFunction)nitro_USBDevice_IsOpen, METH_NOARGS, "Wrapped C++ API member function" },
    {"renum", (PyCFunction)nitro_USBDevice_Renum, METH_NOARGS, "Wrapped C++ API member function" },
    {"reset", (PyCFunction)nitro_USBDevice_Reset, METH_NOARGS, "Wrapped C++ API member function" },
    {"load_firmware", (PyCFunction)nitro_USBDevice_LoadFirmware, METH_VARARGS, "Wrapped C++ API member function" },
    {"get_firmware_version", (PyCFunction)nitro_USBDevice_GetFirmwareVersion, METH_NOARGS, "Wrapped C++ API member function" },
    {NULL}
};

static void nitro_USBDevice_dealloc (nitro_USBDeviceObject* self) {
    if (!self->wrapped_dev) {
       delete  ((nitro_DeviceObject*)self)->nitro_device ;
    } else {
        Py_DECREF(self->wrapped_dev);
    }
    // freed by base class
    //self->ob_type->tp_free ((PyObject*)self);
}

static int
nitro_USBDevice_init(nitro_USBDeviceObject* self, PyObject *args, PyObject *kwds) {
    PyObject *ar, *kw;
    ar = PyTuple_New(0);
    kw = PyDict_New();
    int ret = nitro_DeviceType.tp_init((PyObject*)self, ar, kw);
    Py_DECREF(ar);
    Py_DECREF(kw);
    if(ret < 0) { return ret; }

    int len = PyObject_Length(args);
    if (-1==len) return -1; 

    if ( 2 == len ) {


        uint32 vid, pid;
    
        if (!PyArg_ParseTuple(args,"II",&vid,&pid)) {
            PyErr_SetString ( PyExc_Exception, "USBDevice( vid, pid )" );
            return -1;
        }
    
        ((nitro_DeviceObject*)self)->nitro_device = new USBDevice(vid,pid);
        self->wrapped_dev=NULL;
        return 0;
    } else if ( 1 == len ) {
        PyObject* arg;
        if (!PyArg_ParseTuple(args,"O",&arg) || !PyObject_TypeCheck(arg, &nitro_DeviceType))  {
            PyErr_SetString ( PyExc_Exception, "USBDevice ( Device )" );
            return -1;
        }
        try {
            Device* dev = ((nitro_DeviceObject*)arg)->nitro_device;
            if (!dev || (typeid(*dev) !=
                         typeid(USBDevice))) {
                PyErr_SetString ( PyExc_Exception, "Device must be of type Nitro::USBDevice" );
                return -1;
            }
        
        } catch ( ... ) {
            PyErr_SetString ( PyExc_Exception, "Error checking wrapped device type." );
            return -1;
        }

        Py_INCREF(arg);
        self->wrapped_dev=arg;
        ((nitro_DeviceObject*)self)->nitro_device = 
            ((nitro_DeviceObject*)arg)->nitro_device;


    } else {
        PyErr_SetString ( PyExc_Exception, "USBDevice (vid,pid) | USBDevice(Device)" );
        return -1;
    }
}

PyTypeObject nitro_USBDeviceType = {

    PyObject_HEAD_INIT(NULL)
    0,                         /*ob_size*/
    "nitro.USBDevice",            /*tp_name*/
    sizeof(nitro_USBDeviceObject),  /*tp_basicsize*/
    0,                         /*tp_itemsize*/
    (destructor)nitro_USBDevice_dealloc,   /*tp_dealloc*/
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
    "Nitro USB Device Object",           /* tp_doc */
    0,                    /* tp_traverse */
    0,                     /* tp_clear */
    0,                     /* tp_richcompare */
    0,                     /* tp_weaklistoffset */
    0,                     /* tp_iter */
    0,                     /* tp_iternext */
    nitro_USBDevice_methods,   /* tp_methods */
    0,                       /* tp_members */
    0,                         /* tp_getset */
    0,                         /* tp_base */
    0,                         /* tp_dict */
    0,                         /* tp_descr_get */
    0,                         /* tp_descr_set */
    0,                         /* tp_dictoffset */
    (initproc)nitro_USBDevice_init,  /* tp_init */
    0,                         /* tp_alloc */
    0,                 /* tp_new */
};


PyObject* nitro_USBDevice_Open(nitro_USBDeviceObject* self, PyObject* args) {
    uint32 idx=0;
    PyObject* ver_override = Py_False;
    if (!PyArg_ParseTuple( args, "|IO", &idx, &ver_override )) {
        PyErr_SetString(PyExc_Exception,"USBDevice.open(index=0,override_version=False)");
        return NULL;
    }
    try {
        bool override_version = Py_True == ver_override;
        ((USBDevice*)self->dev_base.nitro_device)->open(idx,override_version);
    } catch (const Exception& e) {
        NITRO_EXC(e,NULL);
    }
    Py_RETURN_NONE;
}


PyObject* nitro_USBDevice_OpenBySerial(nitro_USBDeviceObject* self, PyObject* args) {
    const char* serial;
    if (!PyArg_ParseTuple( args, "s", &serial )) {
        return NULL;
    }
    try {
        ((USBDevice*)self->dev_base.nitro_device)->open(string(serial));
    } catch (const Exception &e) {
        NITRO_EXC(e,NULL);
    }
    Py_RETURN_NONE;    
}

PyObject* nitro_USBDevice_OpenByAddress(nitro_USBDeviceObject* self, PyObject* args) {

    uint16 addr;
    if (!PyArg_ParseTuple( args, "I", &addr)) {
        return NULL;
    }
    try {
        ((USBDevice*)self->dev_base.nitro_device)->open_by_address(addr);
    } catch (const Exception &e) {
        NITRO_EXC(e,NULL);
    }
    Py_RETURN_NONE;    

}

PyObject* nitro_USBDevice_IsOpen(nitro_USBDeviceObject* self) {
    try {
        bool o = ((USBDevice*)self->dev_base.nitro_device)->is_open();
        if (o) {
            Py_RETURN_TRUE;
        } else {
            Py_RETURN_FALSE;
        }
    } catch ( const Exception &e) {
        NITRO_EXC(e,NULL);
    }
    Py_RETURN_NONE;
}

PyObject* nitro_USBDevice_Renum(nitro_USBDeviceObject* self) {
    try {
        ((USBDevice*)self->dev_base.nitro_device)->renum();
    } catch ( const Exception &e) {
        NITRO_EXC(e,NULL);
    }
    Py_RETURN_NONE;
}

PyObject* nitro_USBDevice_Reset(nitro_USBDeviceObject* self) {
    try {
        ((USBDevice*)self->dev_base.nitro_device)->reset();
    } catch ( const Exception &e) {
        NITRO_EXC(e,NULL);
    }
    Py_RETURN_NONE;
}

PyObject* nitro_USBDevice_LoadFirmware(nitro_USBDeviceObject* self, PyObject* args) {
    PyObject* str;
    if (!PyArg_ParseTuple( args, "O", &str)) {
        return NULL;
    }
    if (!PyString_Check(str)) {
        PyErr_SetString(PyExc_Exception, "Firmware bytes should be entered as a String");
        return NULL;
    }
    const char* firmware = PyString_AsString(str);
    int length = PyString_Size(str);
    try {
       ((USBDevice*)self->dev_base.nitro_device)->load_firmware ( firmware, length ); 
    } catch ( const Exception &e) {
        NITRO_EXC(e,NULL);
    }
    Py_RETURN_NONE;
}

PyObject* nitro_USBDevice_GetDeviceCount(nitro_USBDeviceObject* self, PyObject* args) {
    uint32 vid,pid;
    if (!PyArg_ParseTuple( args, "II", &vid, &pid )) {
        return NULL;
    }
    try {
       uint32 n=USBDevice::get_device_count(vid,pid);
       return Py_BuildValue("I",n);
    } catch ( const Exception &e) {
        NITRO_EXC(e,NULL);
    }

}

#define RETURN_NITRO_SERIAL(s) \
    return PyString_FromStringAndSize(s.c_str(), s.size());

PyObject* nitro_USBDevice_GetSerial(nitro_USBDeviceObject* self, PyObject* args) {

    try {
       string s=((USBDevice*)self->dev_base.nitro_device)->get_device_serial();
       RETURN_NITRO_SERIAL(s);
    } catch ( const Exception &e) {
        NITRO_EXC(e,NULL);
    }
}
PyObject* nitro_USBDevice_SetSerial(nitro_USBDeviceObject* self, PyObject *arg) {
   if (!PyString_Check(arg)) {
        PyErr_SetString(PyExc_Exception, "Method requires string argument.");
        return NULL;
   }
   try {
      Py_ssize_t length;
      char *buffer;
      if (-1==PyString_AsStringAndSize( arg, &buffer, &length ) ) return NULL; // type error raised by function
      std::string serial;
      serial.assign ( buffer, length );
      ((USBDevice*)self->dev_base.nitro_device)->set_device_serial(serial);
      Py_RETURN_NONE;
   } catch ( const Exception &e) {
       NITRO_EXC(e,NULL);
   }
}

PyObject* nitro_USBDevice_GetDeviceSerial(nitro_USBDeviceObject* self, PyObject* args) {
    uint32 vid,pid, idx=0;
    if (!PyArg_ParseTuple( args, "II|I", &vid, &pid,&idx )) {
        return NULL;
    }
    try {
       string s=USBDevice::get_device_serial(vid,pid,idx);
       RETURN_NITRO_SERIAL(s);
    } catch ( const Exception &e) {
        NITRO_EXC(e,NULL);
    }
}

PyObject* nitro_USBDevice_SetDeviceSerial(nitro_USBDeviceObject* self, PyObject *arg) {
   if (!PyString_Check(arg)) {
        PyErr_SetString(PyExc_Exception, "Method requires string argument.");
        return NULL;
   }

   try {
      
      ((USBDevice*)self->dev_base.nitro_device)->set_device_serial(PyString_AsString(arg));
      Py_RETURN_NONE;
   } catch ( const Exception &e) {
       NITRO_EXC(e,NULL);
   }
}

PyObject* nitro_USBDevice_GetFirmwareVersion(nitro_USBDeviceObject* self) {
   try {
       uint16 v =((USBDevice*)self->dev_base.nitro_device)->get_firmware_version();
       return Py_BuildValue("I",v);
    } catch ( const Exception &e) {
        NITRO_EXC(e,NULL);
    }
}
PyObject* nitro_USBDevice_GetDeviceAddress(nitro_USBDeviceObject* self, PyObject* args) {
    uint32 vid,pid, idx=0;
    if (!PyArg_ParseTuple( args, "II|I", &vid, &pid,&idx )) {
        return NULL;
    }
    try {
       uint16 addr =((USBDevice*)self->dev_base.nitro_device)->get_device_address(vid,pid,idx);
       return Py_BuildValue("H",addr);
    } catch ( const Exception &e) {
        NITRO_EXC(e,NULL);
    }
}
PyObject* nitro_USBDevice_GetAddress(nitro_USBDeviceObject* self, PyObject* args) {
   try {
       uint16 v =((USBDevice*)self->dev_base.nitro_device)->get_device_address();
       return Py_BuildValue("I",v);
    } catch ( const Exception &e) {
        NITRO_EXC(e,NULL);
    }

}

PyObject* nitro_USBDevice_GetVid(nitro_USBDeviceObject* self, PyObject* args) {
   try {
       uint32 v =((USBDevice*)self->dev_base.nitro_device)->get_vid();
       return Py_BuildValue("I",v);
    } catch ( const Exception &e) {
        NITRO_EXC(e,NULL);
    }
}

PyObject* nitro_USBDevice_GetPid(nitro_USBDeviceObject* self, PyObject* args) {
   try {
       uint32 v =((USBDevice*)self->dev_base.nitro_device)->get_pid();
       return Py_BuildValue("I",v);
    } catch ( const Exception &e) {
        NITRO_EXC(e,NULL);
    }
}
