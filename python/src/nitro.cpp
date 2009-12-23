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
#include <structmember.h>

#include <string>
#include <iostream> // tmp

#include <pynitro/device.h>
#include <pynitro/usb.h>
#include <pynitro/userdevice.h>
#include <pynitro/node.h>
#include <pynitro/xml.h>
#include <pynitro/buffer.h>

#include <pynitro/nitro_pyutil.h>


#define NITRO_MODULE
#include <python_nitro.h>


static PyObject* nitro_LoadDi(PyObject*, PyObject*);

static PyMethodDef core_methods[] = {
   {"load_di", (PyCFunction)nitro_LoadDi, METH_VARARGS, "load_di(filename) -> Load a device interface." }, 
   {NULL}
};

//#ifndef PyMODINIT_FUNC  /* declarations for DLL import/export */
//#define PyMODINIT_FUNC void
//#endif

PyObject* nitro_Exception;


// tmp
void obj_print ( const char* prefix, PyObject* obj ) {
    PyObject* str = PyObject_Str(obj);
    char* cstr = PyString_AsString(str);
    printf ( "%s: %s\n", prefix, cstr );
    Py_DECREF(str);
}


PyMODINIT_FUNC init_nitro(void) {

    #ifdef _DEBUG
     printf ("Debug Nitro Module!\n" );
    #endif

    PyObject* m;

    if (PyType_Ready(&nitro_DeviceType) < 0 )
        return;


    nitro_USBDeviceType.tp_base = &nitro_DeviceType;
    if (PyType_Ready(&nitro_USBDeviceType) < 0 ) 
        return;

    // because base type and type struct are in different cpp files.
    nitro_UserDeviceType.tp_base = &nitro_DeviceType;
    if (PyType_Ready(&nitro_UserDeviceType) < 0)
        return;


    if (PyType_Ready(&nitro_NodeType)<0) {
        return;
    }
    if (PyType_Ready(&nitro_BufferType)<0) { return; }
    if (PyType_Ready(&nitro_DeviceInterfaceType)<0) {
        return;
    }
    if (PyType_Ready(&nitro_XmlReaderType)<0) {
        return;
    }

    if (PyType_Ready(&nitro_TerminalType)<0) {
        return;
    }
    if (PyType_Ready(&nitro_RegisterType)<0) {
        return;
    }
    if (PyType_Ready(&nitro_SubregisterType)<0) {
        return;
    }
    if (PyType_Ready(&nitro_ValuemapType)<0) {
        return;
    }

    if (PyType_Ready(&nitro_XmlWriterType)<0) { return; }
   
    nitro_Exception = PyErr_NewException("nitro.Exception", NULL, NULL);

    m = Py_InitModule3("_nitro", core_methods,
                       "");

    Py_INCREF(&nitro_DeviceType);
    PyModule_AddObject(m, "Device", (PyObject*)&nitro_DeviceType); 
    Py_INCREF(&nitro_USBDeviceType);
    PyModule_AddObject(m, "USBDevice", (PyObject*)&nitro_USBDeviceType);
    Py_INCREF(&nitro_UserDeviceType);
    PyModule_AddObject(m, "UserDevice", (PyObject*)&nitro_UserDeviceType);
    Py_INCREF(&nitro_NodeType);
    PyModule_AddObject(m, "Node", (PyObject*)&nitro_NodeType);
    Py_INCREF(&nitro_DeviceInterfaceType);
    PyModule_AddObject(m, "DeviceInterface", (PyObject*)&nitro_DeviceInterfaceType);
    Py_INCREF(&nitro_TerminalType);
    PyModule_AddObject(m, "Terminal", (PyObject*)&nitro_TerminalType);
    Py_INCREF(&nitro_RegisterType);
    PyModule_AddObject(m, "Register", (PyObject*)&nitro_RegisterType);
    Py_INCREF(&nitro_SubregisterType);
    PyModule_AddObject(m, "Subregister", (PyObject*)&nitro_SubregisterType);
    Py_INCREF(&nitro_ValuemapType);
    PyModule_AddObject(m, "Valuemap", (PyObject*)&nitro_ValuemapType);
    Py_INCREF(&nitro_XmlReaderType);
    PyModule_AddObject(m, "XmlReader", (PyObject*)&nitro_XmlReaderType);
    Py_INCREF(&nitro_XmlWriterType);
    PyModule_AddObject(m, "XmlWriter", (PyObject*)&nitro_XmlWriterType);
    Py_INCREF(&nitro_BufferType);
    PyModule_AddObject(m, "Buffer", (PyObject*)&nitro_BufferType);

    PyModule_AddObject(m, "Exception", nitro_Exception );


    // constants
    PyModule_AddIntConstant(m, "GETSET_VERIFY", Nitro::Device::GETSET_VERIFY);
    PyModule_AddIntConstant(m, "DOUBLEGET_VERIFY", Nitro::Device::DOUBLEGET_VERIFY);
    PyModule_AddIntConstant(m, "STATUS_VERIFY", Nitro::Device::STATUS_VERIFY);
    PyModule_AddIntConstant(m, "RETRY_ON_FAILURE", Nitro::Device::RETRY_ON_FAILURE);
    PyModule_AddStringConstant(m, "str_version", (char*)Nitro::str_version().c_str() ); 
    PyModule_AddIntConstant(m, "version", Nitro::get_version() );


    // finally, a shared C-API
    static void* NitroCAPI[NITRO_CAPI_N_POINTERS];
    NitroCAPI[NITRO_TO_DATATYPE] = (void*) to_datatype;
    NitroCAPI[NITRO_FROM_DATATYPE] = (void*) from_datatype;
    NitroCAPI[NITRO_EXCEPTION] = (void*) nitro_Exception;
    NitroCAPI[NITRO_BUILD_BUFFER] = (void*) nitro_BuildBuffer;

    PyObject* capi = PyCObject_FromVoidPtr ( (void*) NitroCAPI , NULL );
    if (capi) {
        PyModule_AddObject ( m, "_NITRO_API", capi );
    }    

}

// windows python26 debug distutils doesn't seem to know how to handle nameing things very well.
PyMODINIT_FUNC init_nitro_d(void) { init_nitro(); }


// **************** util functions ************************

static PyObject* nitro_LoadDi(PyObject* self, PyObject* arg) {

    char* filename;
    Nitro::DataType di_obj(0);

    if (!PyArg_ParseTuple(arg, "s|O&", &filename, to_datatype, &di_obj) ) {
        PyErr_SetString( PyExc_Exception, "load_di(filename, di=None)" );
        return 0;
    }

    try {
        if (di_obj.get_type() == Nitro::NODE_DATA) {
            Nitro::NodeRef di = Nitro::load_di(filename, (Nitro::NodeRef)di_obj);
            return from_datatype(di);
        } else {
            Nitro::NodeRef di = Nitro::load_di(filename);
            return from_datatype(di); 
        }
    } catch ( Nitro::Exception &e ) {
        NITRO_EXC(e,0);
    }

}


int to_datatype(PyObject *object, void *address) {


    if (!object) {
        PyErr_SetString ( PyExc_Exception , "Unsupported convert NULL to DataType" );
        return 0;
    }

    Nitro::DataType* dt=(Nitro::DataType*)address; 

    if ( PyString_Check(object) ) {
        char* term = PyString_AsString(object);
        *dt = term;
    } else if ( PyFloat_Check ( object ) ) {
        // check float first to not loose precision
        *dt = PyFloat_AsDouble(object);
    }else if ( PyNumber_Check(object) ) {
        PyObject* int_val = PyNumber_Int(object);
        if (NULL==int_val) {
            PyErr_SetString ( PyExc_Exception, "Can't convert object to integer." );
            return 0;
        }
        if (PyInt_Check(int_val)) {
           // convert it to a long

           PyObject* long_args = Py_BuildValue("(O)",int_val);
           PyObject* long_val = PyObject_CallObject( (PyObject*) &PyLong_Type, long_args);
           Py_DECREF(int_val); 
           Py_DECREF(long_args);
           if (!long_val ) return 0; // NOTE err needed?

           int_val = long_val;
        }
        if (PyLong_Check(int_val)) {
            
            std::vector<Nitro::DataType> ints;
            
            PyObject* shift_cnt = PyLong_FromLong(32);
            PyObject* cmp = PyLong_FromLong(0);
            PyObject* ander = PyLong_FromUnsignedLong(0xffffffff);
            // while int > 0
            PyObject* res; // tmp
            int ret;
            do {
                //obj_print ( "int val", int_val );
                PyObject* lsi = PyNumber_And ( int_val, ander ); 
                //obj_print ( "lsi", lsi );
                unsigned long l = PyLong_AsUnsignedLong(lsi);
                ints.push_back ( (uint32) l );
                res=PyNumber_Rshift ( int_val, shift_cnt );
                Py_DECREF(int_val);
                int_val=res;
                Py_DECREF(lsi);
            } while ( (ret=PyObject_RichCompareBool ( int_val, cmp, Py_GT )) == 1 );
           
            Py_DECREF(shift_cnt);
            Py_DECREF(cmp);
            Py_DECREF(ander);

            if ( -1 == ret ) {
                if (PyErr_Occurred()) {
                    PyErr_Print();
                }
                PyErr_SetString( PyExc_Exception , "Error converting to bigint data." ); 
                return 0; 
            }

            if (ints.size()==1) {
                *dt = (uint32) ints.front();
            } else {
                *dt = Nitro::DataType::as_bigint_datatype(ints);
            }

        } // else I don't think there is an else.

        Py_DECREF(int_val);



    } else if ( PyObject_TypeCheck ( object, &nitro_NodeType ) ) {
        *dt = *((nitro_NodeObject*)object)->m_node;
    } else if ( PyList_Check(object) ) {
        std::vector<Nitro::DataType> new_list;
        PyObject* itr = PyObject_GetIter(object);
        PyObject* item = NULL;
        if (NULL==itr) {
            PyErr_SetString ( PyExc_Exception, "Can't iterate list." );
            return 0;
        }
        while ( (item = PyIter_Next ( itr )) ) {
            Nitro::DataType dt_item(0);
            if (to_datatype ( item, &dt_item ) ){
                new_list.push_back(dt_item);
            }
            Py_DECREF(item);
        }
        Py_DECREF(itr);
        if (PyErr_Occurred()) {
            return 0;
        } else {
            *dt = new_list;
        }
    } else if ( PyMapping_Check ( object ) ) {
        Nitro::NodeRef node = Nitro::Node::create("mapping");
        PyObject* items = PyMapping_Items(object);
        if (!items) {
            PyErr_SetString(PyExc_Exception,"Can't iterate mappings.");
            return 0;
        }
        PyObject* itr = PyObject_GetIter(items);
        if (!itr) {
            PyErr_SetString(PyExc_Exception,"Can't iterate mappings.");
            Py_DECREF(items);
            return 0;
        }
        PyObject *item;
        while ( (item = PyIter_Next(itr)) != NULL ) {

            PyObject *first = PySequence_GetItem ( item, 0 );
            if ( !PyString_Check (first)) {
                PyErr_SetString(PyExc_Exception,"Mapping key must be a string.");
                Py_DECREF(first);
                Py_DECREF(item);
                return 0;
            }

            PyObject *second = PySequence_GetItem(item,1);
            
            Nitro::DataType dt_item(0);
            int ret;
            if( (ret=to_datatype ( second, &dt_item )) ) {
                char* key = PyString_AsString(first);
                node->set_attr ( key, dt_item ); 
            } 
            
            Py_DECREF(first);
            Py_DECREF(second);
            Py_DECREF(item);
            if (!ret) return 0; // err set by this function to_datatype
        }
        
        Py_DECREF(itr);
        Py_DECREF(items);

        *dt=node;

    } else if ( PyObject_TypeCheck(object,&nitro_BufferType )) {		
        nitro_BufferObject* b = (nitro_BufferObject*)object;
        *dt = Nitro::DataType::as_datatype( b->m_buf, b->m_len );
    } else if (object == Py_None) {
       *dt= 0; 	   
    } else {
        PyErr_SetObject ( PyExc_Exception, object );
        return 0;
    }
        
    return 1;
}


PyObject* from_datatype(const Nitro::DataType& dt) {
   switch ( dt.get_type() ) {
        case Nitro::INT_DATA: 
                {
                 int32 i = dt;
                 return PyLong_FromLong(i);
                }
        case Nitro::UINT_DATA:
                {
                 uint32 i = (uint32)dt;
                 return PyLong_FromUnsignedLong(i);
                }
        case Nitro::FLOAT_DATA:
            {
                double i = (double) dt;
                return PyFloat_FromDouble(i);
            }
        case Nitro::STR_DATA:
                {
                 std::string s = (std::string)dt;
                 return PyString_FromString(s.c_str());
                }
        case Nitro::NODE_DATA:
            {
                return nitro_BuildNode ( (Nitro::NodeRef)dt );
            }
        case Nitro::LIST_DATA:
            {
                std::vector<Nitro::DataType> list = dt ;
                std::vector<Nitro::DataType>::iterator itr = list.begin();
                PyObject* new_list = PyList_New( list.size() );
                int i=0;
                while ( itr != list.end() ) {
                    PyObject* obj = from_datatype( *itr ) ;
                    if (obj) {
                       PyList_SetItem( new_list, i++, obj );  // steals item ref
                       ++itr;
                    } else {
                        Py_DECREF(new_list);
                        return NULL;
                    }
                }

                return new_list;
            }
        case Nitro::DEV_DATA:
            {
               PyObject* dev = PyObject_CallObject ( (PyObject*)&nitro_DeviceType, NULL );
                   if (!dev ) {
                    //PyErr_SetString ( PyExc_Exception, "Unable to create device object." );
                    return NULL;
                }
               Nitro::Device& d = dt ;
               ((nitro_DeviceObject*)dev)->nitro_device = &d ; 
               return dev;
            }
        case Nitro::BUF_DATA:
            {
                uint8* buf = Nitro::DataType::as_buffer( dt );

                PyObject* b = nitro_BuildBuffer( buf, (uint32) dt );
                return b;
            } 
        case Nitro::BIGINT_DATA:
            {
                PyObject* i = PyLong_FromUnsignedLong(0);
                std::vector<Nitro::DataType> ints = Nitro::DataType::as_bigints(dt);
                PyObject* shift_cnt = PyLong_FromLong(32);
                PyObject* res; // tmp
                while (ints.size()) {
                    uint32 msi = ints.back();
                    ints.pop_back();
                    
                    res=PyNumber_InPlaceLshift ( i, shift_cnt );
                    Py_DECREF(i);
                    i=res;
                    PyObject* lsi = PyLong_FromUnsignedLong(msi);
                    
                    res = PyNumber_InPlaceOr ( i, lsi );
                    Py_DECREF (i);
                    i=res;
                    Py_DECREF ( lsi );
                }

                Py_DECREF(shift_cnt);
                return i;
            }
        default:
            PyErr_SetString ( PyExc_Exception, "Unsupported Nitro::DataType" );
            return NULL;
   }
}

