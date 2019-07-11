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

#include <pynitro/device.h>

#include <structmember.h>

//#include <numpy/arrayobject.h>

#include <iostream>
#include <string>


#include <pynitro/nitro_pyutil.h>

#include <pynitro/node.h>

using namespace std;
using namespace Nitro;



class PyRetryFunc : public Device::RetryFunc {
    private:
        PyObject* m_pyfunc;
 
    public:
        PyRetryFunc(PyObject* func) : RetryFunc(), m_pyfunc(func) {
            Py_INCREF(m_pyfunc);
        }
        ~PyRetryFunc() {
            Py_DECREF(m_pyfunc);
        }

        bool operator() ( Nitro::Device &dev, uint32 term_addr, uint32 reg_addr, uint32 retries, const Exception &exc ) {

            PyGILState_STATE gstate; // GIL state
            gstate = PyGILState_Ensure();
            // safe to call python stuff now
            
            DataType devdt(dev);
            PyObject* pyret = PyObject_CallFunction (
                m_pyfunc, 
                "O&IIIs", 
                from_datatype, &devdt, term_addr, reg_addr,
                retries, exc.str_error().c_str() );

            bool ret = pyret==Py_True;
            
            char* str_error=NULL;
            if (pyret == NULL) {
               PyObject *ptype, *pvalue, *ptraceback;
               PyErr_Fetch(&ptype,&pvalue,&ptraceback);
               PyObject* str_value = PyObject_Str( pvalue );
               str_error = PyBytes_AsString(str_value);
               Py_XDECREF(ptype);
               Py_XDECREF(pvalue);
               Py_XDECREF(ptraceback);
               Py_XDECREF(str_value);
            }
            Py_XDECREF(pyret);
            PyGILState_Release(gstate); 

            // no more python
            if (str_error)
                throw Exception ( SCRIPTS_SCRIPT, str_error );
            return ret;
        }


};


PyMethodDef nitro_Device_methods[] = {
    {"load_xml", (PyCFunction)nitro_Device_LoadXML, METH_VARARGS, "load_xml( xml_path )" },
    {"write_xml", (PyCFunction)nitro_Device_WriteXML, METH_O, "write_xml ( xml_path )" },
    {"get_di", (PyCFunction)nitro_Device_GetDi, METH_NOARGS, "get_di()-> Device Interface Nodes"},
    {"set_di", (PyCFunction)nitro_Device_SetDi, METH_O, "set_di(di)" },
    {"get_tree", (PyCFunction)nitro_Device_GetDi, METH_NOARGS, "Deprecated->Use get_di"},
    {"set_tree", (PyCFunction)nitro_Device_SetDi, METH_O, "Deprecated->Use set_di(di)" },
    {"lock", (PyCFunction)nitro_Device_Lock, METH_NOARGS, "lock()"},
    {"unlock", (PyCFunction)nitro_Device_Unlock, METH_NOARGS, "lock()"},
    {"get", (PyCFunction)nitro_Device_Get, METH_VARARGS, "Wrapped C++ API member function" },
    {"get_subregs", (PyCFunction)nitro_Device_GetSubregs, METH_VARARGS, "get_subregs(term,reg,timeout=None)" },
    {"set", (PyCFunction)nitro_Device_Set, METH_VARARGS, "Wrapped C++ API member function" },
    {"read", (PyCFunction)nitro_Device_Read, METH_VARARGS, 
        "read( term, reg, data, timeout=1000 )\n"
        "\tdata can be either string or array data." },
    {"write", (PyCFunction)nitro_Device_Write, METH_VARARGS, "Wrapped C++ API member function" },
    {"close", (PyCFunction)nitro_Device_Close, METH_NOARGS, "Wrapped C++ API member function" },
    {"enable_mode",(PyCFunction)nitro_Device_EnableMode, METH_VARARGS, "enable_mode(mode,term=None)" },
    {"disable_mode",(PyCFunction)nitro_Device_DisableMode, METH_VARARGS, "disable_mode(mode,term=None)" },
    {"set_modes",(PyCFunction)nitro_Device_SetModes, METH_VARARGS, "set_modes(mode,term=None)" },
    {"get_modes",(PyCFunction)nitro_Device_GetModes, METH_VARARGS, "get_modes(term=None)" },
    {"set_retry_func",(PyCFunction)nitro_Device_SetRetryFunc, METH_O, "set_retry_func(func)\n\n"
        "The retry funtion should have the following signature:\n"
        "retry_func(dev,term_addr,reg_addr,retries,exc)\n\n"
        "dev: The device.\n"
        "term_addr: Terminal Address.\n"
        "reg_addr: Register Address.\n"
        "retries: Number of times retried so far.\n"
        "exc: The exception that occurred."},
    {"set_timeout",(PyCFunction)nitro_Device_SetTimeout, METH_O, "set_timeout(timeout)" },
    {NULL}
};

static PyObject* 
nitro_Device_new ( PyTypeObject* type, PyObject *args, PyObject *kwds ) {

    nitro_DeviceObject *self = (nitro_DeviceObject*)type->tp_alloc(type,0);
    if (self != NULL) {
        self->nitro_device = NULL;
        if (self->retry_func) {
            self->retry_func = NULL;
        }
    }
    return (PyObject*)self;

}

static void nitro_Device_dealloc (nitro_DeviceObject* self) {
    if (self->retry_func) {
        delete self->retry_func;
    }
    // we never de-allocate the nitro_device.  That is done
    // by the extending classes.
#if PY_MAJOR_VERSION >= 3
    self->ob_base.ob_type->tp_free((PyObject*)self);
#else
    self->ob_type->tp_free((PyObject*)self);
#endif
}

#if PY_MAJOR_VERSION >= 3
PyTypeObject nitro_DeviceType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "nitro.Device",// char *tp_name; For printing, in format "<module>.<name>"
    sizeof(nitro_DeviceObject), //Py_ssize_t tp_basicsize, 
    NULL, //tp_itemsize; /* For allocation */

    /* Methods to implement standard operations */
    (destructor)nitro_Device_dealloc,	//destructor tp_dealloc;
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
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,	//unsigned long tp_flags;

    "Basic Nitro Device Object",//const char *tp_doc; /* Documentation string */

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
    nitro_Device_methods, //struct PyMethodDef *tp_methods;
    NULL, //struct PyMemberDef *tp_members;
    NULL, //struct PyGetSetDef *tp_getset;
    NULL, //struct _typeobject *tp_base;
    NULL, //PyObject *tp_dict;
    NULL, //descrgetfunc tp_descr_get;
    NULL, //descrsetfunc tp_descr_set;
    NULL, //Py_ssize_t tp_dictoffset;
    NULL, //initproc tp_init;
    NULL, //allocfunc tp_alloc;
    nitro_Device_new, //newfunc tp_new;
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
PyTypeObject nitro_DeviceType = {

    PyObject_HEAD_INIT(NULL)
    0,                         /*ob_size*/
    "nitro.Device",            /*tp_name*/
    sizeof(nitro_DeviceObject),  /*tp_basicsize*/
    0,                         /*tp_itemsize*/
    (destructor)nitro_Device_dealloc, /*tp_dealloc*/
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
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,        /*tp_flags*/
    "Basic Nitro Device Object",           /* tp_doc */
    0,                    /* tp_traverse */
    0,                     /* tp_clear */
    0,                     /* tp_richcompare */
    0,                     /* tp_weaklistoffset */
    0,                     /* tp_iter */
    0,                     /* tp_iternext */
    nitro_Device_methods,   /* tp_methods */
    0,                       /* tp_members */
    0,                         /* tp_getset */
    0,                         /* tp_base */
    0,                         /* tp_dict */
    0,                         /* tp_descr_get */
    0,                         /* tp_descr_set */
    0,                         /* tp_dictoffset */
    0,                        /* tp_init */
    0,                         /* tp_alloc */
    nitro_Device_new,                 /* tp_new */
};
#endif

#define CHECK_ABSTRACT() if (!self->nitro_device) { \
        PyErr_SetString(PyExc_Exception,"Device is abstract and cannot be uses directly."); \
        return NULL; \
    }



PyObject* nitro_Device_Lock(nitro_DeviceObject* self) {
    CHECK_ABSTRACT();
    
    Py_BEGIN_ALLOW_THREADS  
    self->nitro_device->lock();
    Py_END_ALLOW_THREADS  
    
    Py_RETURN_NONE;
}

PyObject* nitro_Device_Unlock(nitro_DeviceObject* self) {
    CHECK_ABSTRACT();
    
    Py_BEGIN_ALLOW_THREADS  
    self->nitro_device->unlock();
    Py_END_ALLOW_THREADS  
    
    Py_RETURN_NONE;
}



PyObject* nitro_Device_Get(nitro_DeviceObject* self, PyObject* args) {
      CHECK_ABSTRACT();
      DataType term(0);
      DataType reg(0);
      int32 timeout=-1;
      uint32 width=0;
      PyObject obj1, obj2;
      if (!PyArg_ParseTuple ( args, "O&O&|II", to_datatype, &term, to_datatype, &reg, &timeout, &width ) ) {
        return NULL;
      }

      DataType ret(0);
      Exception* saveme=NULL;
      Py_BEGIN_ALLOW_THREADS
      try {
        ret = self->nitro_device->get (term,reg,timeout, width);
      } catch (const Exception& e) {
	   // can't use NITRO_EXC here.
       saveme=new Exception(e);
      }
      Py_END_ALLOW_THREADS

      if (saveme) {
        SET_NITRO_EXC(*saveme);
        delete saveme;
        return NULL;
      }
      return from_datatype(ret);
}

PyObject* nitro_Device_GetSubregs(nitro_DeviceObject* self, PyObject* args) {

      CHECK_ABSTRACT();

      DataType term(0);
      DataType reg(0);
      int32 timeout=-1;
      if (!PyArg_ParseTuple ( args, "O&O&|I", to_datatype, &term, to_datatype, &reg, &timeout ) ) {
        return NULL;
      }

      NodeRef subregs;
      Exception* saveme=NULL;
      Py_BEGIN_ALLOW_THREADS
      try {
        subregs = self->nitro_device->get_subregs (term,reg,timeout);
      } catch (const Exception& e) {
	   // can't use NITRO_EXC here.
       saveme=new Exception(e);
      }
      Py_END_ALLOW_THREADS

      if (saveme) {
        SET_NITRO_EXC(*saveme);
        delete saveme;
        return NULL;
      }

      return from_datatype(subregs);

}

PyObject* nitro_Device_Set(nitro_DeviceObject* self, PyObject *args) {
    CHECK_ABSTRACT();

    DataType term(0);
    DataType reg(0);
    DataType val(0);
    int32 timeout=-1;
    int32 width=0;
    if (!PyArg_ParseTuple ( args, "O&O&O&|II", to_datatype, &term, to_datatype, &reg, to_datatype, &val, &timeout, &width ) ) {
        return NULL;
    }
    
    Exception* saveme=NULL;
    Py_BEGIN_ALLOW_THREADS
    try {
      self->nitro_device->set(term,reg,val,timeout,width);

    } catch (const Exception& e) {
        saveme=new Exception(e);
    }
    Py_END_ALLOW_THREADS

    if (saveme) {
        SET_NITRO_EXC(*saveme);
        delete saveme;
        return NULL;
    }
    Py_RETURN_NONE;
}

PyObject* nitro_Device_Read(nitro_DeviceObject* self, PyObject *args) {

    CHECK_ABSTRACT();

    DataType term(0);
    DataType reg(0);
    PyObject* pydata = NULL;;
    uint8* data = NULL;
    uint32 timeout=1000;

    if (!PyArg_ParseTuple( args, "O&O&O|I",
                    to_datatype, &term, to_datatype, &reg, &pydata, &timeout )) {
        return NULL;
    }
    uint32 length;
    if (PyBytes_Check(pydata)) {
        data = (uint8*)PyBytes_AsString(pydata);
        length = PyBytes_Size(pydata);
    } else if(PyObject_CheckReadBuffer(pydata)) {
      void *d;
      Py_ssize_t l;
      if(PyObject_AsWriteBuffer(pydata, &d, &l) == -1) {
	return NULL;
      }
      data = (uint8 *) d;
      length = (uint32) l;
	} else {
        PyErr_SetString(PyExc_Exception, "Data must be a string, single-segment buffer, or array object." );
        return NULL;
    }
    
         
    Exception* saveme=NULL;
    Py_BEGIN_ALLOW_THREADS  
        try {
            
            self->nitro_device->read ( term, reg, data, length, timeout );
    
        } catch ( const Exception& e) {
            // can't use NITRO_EXC here.
            saveme=new Exception(e);
        }
    Py_END_ALLOW_THREADS

    if (saveme) {
        SET_NITRO_EXC(*saveme);
        delete saveme;
        return NULL;
    }
    Py_RETURN_NONE;

}
PyObject* nitro_Device_Write(nitro_DeviceObject* self, PyObject *args) {
   CHECK_ABSTRACT();


    DataType term(0);
    DataType reg(0);
    PyObject* pydata = NULL;;
    uint8* data = NULL;
    uint32 timeout=1000;

    if (!PyArg_ParseTuple( args, "O&O&O|I",  
                    to_datatype, &term, to_datatype, &reg, &pydata, &timeout )) {
        return NULL;
    }

    uint32 length;
    if (PyBytes_Check(pydata)) {
        data = (uint8*)PyBytes_AsString(pydata);
        length = PyBytes_Size(pydata);
    } else if(PyObject_CheckReadBuffer(pydata)) {
      const void *d;
      Py_ssize_t l;
      if(PyObject_AsReadBuffer(pydata, &d, &l) == -1) {
	return NULL;
      }
      data = (uint8 *) d;
      length = (uint32) l;
	} else {
        PyErr_SetString(PyExc_Exception, "Data must be a string or array object." );
        return NULL;
    }
    
         
    Exception* saveme=NULL;
    Py_BEGIN_ALLOW_THREADS  
        try {
            
            self->nitro_device->write ( term, reg, data, length, timeout );
    
        } catch ( const Exception& e) {
            // can't use NITRO_EXC here.
            saveme=new Exception(e);
        }
    Py_END_ALLOW_THREADS

    if (saveme) {
        SET_NITRO_EXC(*saveme);
        delete saveme;
        return NULL;
    }
    Py_RETURN_NONE;

}
PyObject* nitro_Device_Close(nitro_DeviceObject* self) {

    CHECK_ABSTRACT(); 

    Py_BEGIN_ALLOW_THREADS
    self->nitro_device->close();
    Py_END_ALLOW_THREADS

    Py_RETURN_NONE;
}

PyObject* nitro_Device_LoadXML(nitro_DeviceObject* self, PyObject *args) {
    CHECK_ABSTRACT();

    const char* s;
    if (!PyArg_ParseTuple( args, "s", &s )) {
        return NULL;
    }

    try { 
        Nitro::load_di( s, self->nitro_device->get_di() );
        Py_RETURN_NONE;
    } catch ( const Exception &e ) {
        NITRO_EXC(e,NULL);
    }

}

PyObject* nitro_Device_WriteXML(nitro_DeviceObject* self, PyObject *arg ){
    CHECK_ABSTRACT();
    const char* s;
    if (!PyBytes_Check( arg )) {
        PyErr_SetString( PyExc_Exception, "write_xml(file_path)" );
        return NULL;
    }

    s=PyBytes_AsString(arg);
    try {

        XmlWriter w ( s );
        w.write ( self->nitro_device->get_di() );
        Py_RETURN_NONE;

    } catch ( const Exception &e ) {
        NITRO_EXC(e,NULL);
    }
}

PyObject* nitro_Device_GetDi(nitro_DeviceObject* self ) {

    try {

        Nitro::NodeRef di = self->nitro_device->get_di();
        return nitro_BuildNode(di); 

    } catch ( const Exception &e) {
        NITRO_EXC(e,NULL);
    }

}

PyObject* nitro_Device_SetDi(nitro_DeviceObject* self, PyObject *arg) {

    if (!PyObject_TypeCheck(arg, &nitro_NodeType)) {
        PyErr_SetObject(PyExc_Exception, arg );
        return NULL;
    }
    try {
         self->nitro_device->set_di ( *((nitro_NodeObject*)arg)->m_node ) ;
         Py_RETURN_NONE;
    } catch ( const Exception &e) {
        NITRO_EXC(e,NULL);
    }
}

PyObject* nitro_Device_EnableMode(nitro_DeviceObject* self, PyObject *arg) {
    CHECK_ABSTRACT();
    uint32 m;
    DataType term(-1);
    if (!PyArg_ParseTuple(arg, "I|O&", &m, to_datatype, &term)) {
      PyErr_SetObject(PyExc_Exception, arg);
      return NULL;
    }
    Exception* saveme=NULL;
    Py_BEGIN_ALLOW_THREADS  
        try {
            
            if (term != -1) {
                self->nitro_device->enable_mode ( term, m );
            } else {
                self->nitro_device->enable_mode ( m );
            }
    
        } catch ( const Exception& e) {
            // can't use NITRO_EXC here.
            saveme=new Exception(e);
        }
    Py_END_ALLOW_THREADS

    if (saveme) {
        SET_NITRO_EXC(*saveme);
        delete saveme;
        return NULL;
    }
    Py_RETURN_NONE;


   
}

PyObject* nitro_Device_SetModes(nitro_DeviceObject* self, PyObject *arg) {
    CHECK_ABSTRACT();
    uint32 m;
    DataType term(-1);
    if (!PyArg_ParseTuple(arg, "I|O&", &m, to_datatype, &term)) {
      PyErr_SetObject(PyExc_Exception, arg);
      return NULL;
    }
    Exception* saveme=NULL;
    Py_BEGIN_ALLOW_THREADS  
        try {
            
            if (term != -1) {
                self->nitro_device->set_modes ( term, m );
            } else {
                self->nitro_device->set_modes ( m );
            }
    
        } catch ( const Exception& e) {
            // can't use NITRO_EXC here.
            saveme=new Exception(e);
        }
    Py_END_ALLOW_THREADS

    if (saveme) {
        SET_NITRO_EXC(*saveme);
        delete saveme;
        return NULL;
    }
    Py_RETURN_NONE;
   
}

PyObject* nitro_Device_DisableMode(nitro_DeviceObject* self, PyObject *arg) {
    CHECK_ABSTRACT();
    uint32 m;
    DataType term(-1);
    if (!PyArg_ParseTuple(arg, "I|O&", &m, to_datatype, &term)) {
      PyErr_SetObject(PyExc_Exception, arg);
      return NULL;
    }
    Exception* saveme=NULL;
    Py_BEGIN_ALLOW_THREADS  
        try {
            
            if (term != -1) {
                self->nitro_device->disable_mode ( term, m );
            } else {
                self->nitro_device->disable_mode ( m );
            }
    
        } catch ( const Exception& e) {
            // can't use NITRO_EXC here.
            saveme=new Exception(e);
        }
    Py_END_ALLOW_THREADS

    if (saveme) {
        SET_NITRO_EXC(*saveme);
        delete saveme;
        return NULL;
    }
    Py_RETURN_NONE;

    
}
PyObject* nitro_Device_GetModes(nitro_DeviceObject* self, PyObject *arg) {
    CHECK_ABSTRACT();
    DataType term(-1);
    if (!PyArg_ParseTuple(arg, "|O&", to_datatype, &term)) {
      PyErr_SetObject(PyExc_Exception, arg);
      return NULL;
    }
    Exception* saveme=NULL;
    uint32 ret=0;
    Py_BEGIN_ALLOW_THREADS  
        try {
            
            if (term != -1) {
                ret=self->nitro_device->get_modes ( term );
            } else {
                ret=self->nitro_device->get_modes ( );
            }
    
        } catch ( const Exception& e) {
            // can't use NITRO_EXC here.
            saveme=new Exception(e);
        }
    Py_END_ALLOW_THREADS

    if (saveme) {
        SET_NITRO_EXC(*saveme);
        delete saveme;
        return NULL;
    }

    return Py_BuildValue ( "I", ret );
}

PyObject* nitro_Device_SetRetryFunc(nitro_DeviceObject* self, PyObject *arg) {
    CHECK_ABSTRACT();
    
    // clear any references in the driver before deleting func
    Py_BEGIN_ALLOW_THREADS
    self->nitro_device->set_retry_func(NULL);
    Py_END_ALLOW_THREADS

    if (self->retry_func) {
        delete self->retry_func;
        self->retry_func=NULL;
    }
    if (arg == Py_None) {
       Py_RETURN_NONE; // jobs done 
    }

    self->retry_func = new PyRetryFunc(arg);
    Py_BEGIN_ALLOW_THREADS
    self->nitro_device->set_retry_func(self->retry_func);
    Py_END_ALLOW_THREADS

    Py_RETURN_NONE;

}

PyObject* nitro_Device_SetTimeout(nitro_DeviceObject* self, PyObject *arg) {
  
  CHECK_ABSTRACT();

  unsigned long timeout = PyLong_AsUnsignedLongMask(arg);
  if (PyErr_Occurred()) {
    PyErr_SetObject ( PyExc_Exception, arg );
    return NULL;
  }

  Exception* saveme=NULL;
  Py_BEGIN_ALLOW_THREADS  
     try {
       self->nitro_device->set_timeout(timeout);
     } catch ( const Exception& e) {
         // can't use NITRO_EXC here.
         saveme=new Exception(e);
     }
  Py_END_ALLOW_THREADS

  if (saveme) {
      SET_NITRO_EXC(*saveme);
      delete saveme;
      return NULL;
  }

  Py_RETURN_NONE;

}
