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


#include <pynitro/xml.h>

#include <pynitro/nitro_pyutil.h>

#include <pynitro/node.h>


static PyObject* 
nitro_XmlReader_new ( PyTypeObject* type, PyObject *args, PyObject *kwds ) {
    nitro_XmlReaderObject *self = (nitro_XmlReaderObject*)type->tp_alloc(type,0);

    self->m_reader = NULL; 
    return (PyObject*)self;
}

static void nitro_XmlReader_dealloc (nitro_XmlReaderObject* self) {
    if (self->m_reader) delete self->m_reader;
#if PY_MAJOR_VERSION >= 3
    self->ob_base.ob_type->tp_free((PyObject*)self);
#else
    self->ob_type->tp_free((PyObject*)self);
#endif
}


// ********************** init *****************************

static int
nitro_XmlReader_init ( nitro_XmlReaderObject* self, PyObject *args, PyObject *kwds ) {

  const char* filename;
  if (!PyArg_ParseTuple(args,"s",&filename)) {
     PyErr_SetString( PyExc_Exception, "XmlReader(filename)");
     return -1;
  }

  try {
  self->m_reader = new Nitro::XmlReader ( filename );
  } catch ( const Nitro::Exception &e ) {
    NITRO_EXC(e, -1);
 }

 return 0;
}



static
PyObject* nitro_XmlReader_Read ( nitro_XmlReaderObject* self, PyObject* node ) {
    if ( !PyObject_TypeCheck ( node , &nitro_NodeType ) ) {
        PyErr_SetString ( PyExc_Exception,"read(node) node must be nitro.Node or a subtype" );
        return NULL;
    } 

    try {
        nitro_NodeObject* n = (nitro_NodeObject*)node;
        self->m_reader->read(*n->m_node); 
        Py_RETURN_NONE;
    } catch ( const Nitro::Exception &e ) {
       NITRO_EXC(e,NULL);
    }
    
}


static PyMethodDef nitro_XmlReader_Methods[] = {
    {"read", (PyCFunction)nitro_XmlReader_Read, METH_O, "read(Node)" },
    { NULL }
};




#if PY_MAJOR_VERSION >= 3
PyTypeObject nitro_XmlReaderType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "nitro.XmlReader", // char *tp_name; For printing, in format "<module>.<name>"
    sizeof(nitro_XmlReaderObject),//Py_ssize_t tp_basicsize, 
    NULL, //tp_itemsize; /* For allocation */

    /* Methods to implement standard operations */
    (destructor)nitro_XmlReader_dealloc, //destructor tp_dealloc;
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
    Py_TPFLAGS_DEFAULT|Py_TPFLAGS_BASETYPE,//unsigned long tp_flags;

    "Nitro XmlReader Object",//const char *tp_doc; /* Documentation string */

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
    nitro_XmlReader_Methods, //struct PyMethodDef *tp_methods;
    NULL, //struct PyMemberDef *tp_members;
    NULL, //struct PyGetSetDef *tp_getset;
    NULL, //struct _typeobject *tp_base;
    NULL, //PyObject *tp_dict;
    NULL, //descrgetfunc tp_descr_get;
    NULL, //descrsetfunc tp_descr_set;
    NULL, //Py_ssize_t tp_dictoffset;
    (initproc)nitro_XmlReader_init,//initproc tp_init;
    NULL, //allocfunc tp_alloc;
    nitro_XmlReader_new, //newfunc tp_new;
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
PyTypeObject nitro_XmlReaderType = {

    PyObject_HEAD_INIT(NULL)
    0,                         /*ob_size*/
    "nitro.XmlReader",            /*tp_name*/
    sizeof(nitro_XmlReaderObject),  /*tp_basicsize*/
    0,                         /*tp_itemsize*/
    (destructor)nitro_XmlReader_dealloc,   /*tp_dealloc*/
    0,                         /*tp_print*/
    0,                         /*tp_getattr*/
    0,                         /*tp_setattr*/
    0,                         /*tp_compare*/
    0,                         /*tp_repr*/
    0,                         /*tp_as_number*/
    0,                         /*tp_as_sequence*/
    0,                     /*tp_as_mapping*/
    0,                         /*tp_hash */
    0,                         /*tp_call*/
    0,                        /*tp_str*/
    0,                      /*tp_getattro*/
    0,                          /*tp_setattro*/
    0,                         /*tp_as_buffer*/
    Py_TPFLAGS_DEFAULT|Py_TPFLAGS_BASETYPE,   /*tp_flags*/
    "Nitro XmlReader Object",           /* tp_doc */
    0,                    /* tp_traverse */
    0,                     /* tp_clear */
    0,                     /* tp_richcompare */
    0,                     /* tp_weaklistoffset */
    0,                      /* tp_iter */
    0,                     /* tp_iternext */
    nitro_XmlReader_Methods,     /* tp_methods */
    0,                       /* tp_members */
    0,                         /* tp_getset */
    0,                         /* tp_base */
    0,                         /* tp_dict */
    0,                         /* tp_descr_get */
    0,                         /* tp_descr_set */
    0,                         /* tp_dictoffset */
    (initproc)nitro_XmlReader_init, /* tp_init */
    0,                         /* tp_alloc */
    nitro_XmlReader_new,            /* tp_new */
};
#endif
/// ******************************** xml writer ************************************

static PyObject* 
nitro_XmlWriter_new ( PyTypeObject* type, PyObject *args, PyObject *kwds ) {
    nitro_XmlWriterObject *self = (nitro_XmlWriterObject*)type->tp_alloc(type,0);

    self->m_writer = NULL; 
    return (PyObject*)self;
}

static void nitro_XmlWriter_dealloc (nitro_XmlWriterObject* self) {
    if (self->m_writer) delete self->m_writer;
#if PY_MAJOR_VERSION >= 3
    self->ob_base.ob_type->tp_free((PyObject*)self);
#else
    self->ob_type->tp_free((PyObject*)self);
#endif
}


// ********************** init *****************************

static int
nitro_XmlWriter_init ( nitro_XmlWriterObject* self, PyObject *args, PyObject *kwds ) {

const char* filename;
  if (!PyArg_ParseTuple(args,"s",&filename)) {
     PyErr_SetString( PyExc_Exception, "XmlWriter(filename)");
     return -1;
  }

  try {
      self->m_writer = new Nitro::XmlWriter ( filename );
  } catch ( const Nitro::Exception &e ) {
    NITRO_EXC(e, -1);
 }

 return 0;
}



static
PyObject* nitro_XmlWriter_Write ( nitro_XmlWriterObject* self, PyObject* node ) {
    if ( !PyObject_TypeCheck ( node , &nitro_NodeType ) ) {
        PyErr_SetString ( PyExc_Exception,"write(node) node must be nitro.Node or a subtype" );
        return NULL;
    } 

    try {
        nitro_NodeObject* n = (nitro_NodeObject*)node;
        self->m_writer->write(*n->m_node); 
        Py_RETURN_NONE;
    } catch ( const Nitro::Exception &e ) {
       NITRO_EXC(e,NULL);
    }
    
}


static PyMethodDef nitro_XmlWriter_Methods[] = {
    {"write", (PyCFunction)nitro_XmlWriter_Write, METH_O, "write(Node)" },
    { NULL }
};



#if PY_MAJOR_VERSION >= 3
PyTypeObject nitro_XmlWriterType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "nitro.XmlWriter", // char *tp_name; For printing, in format "<module>.<name>"
    sizeof(nitro_XmlWriterObject),//Py_ssize_t tp_basicsize, 
    NULL, //tp_itemsize; /* For allocation */

    /* Methods to implement standard operations */
    (destructor)nitro_XmlWriter_dealloc,//destructor tp_dealloc;
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
    Py_TPFLAGS_DEFAULT|Py_TPFLAGS_BASETYPE,//unsigned long tp_flags;

    "Nitro XmlWriter Object",//const char *tp_doc; /* Documentation string */

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
    nitro_XmlWriter_Methods,//struct PyMethodDef *tp_methods;
    NULL, //struct PyMemberDef *tp_members;
    NULL, //struct PyGetSetDef *tp_getset;
    NULL, //struct _typeobject *tp_base;
    NULL, //PyObject *tp_dict;
    NULL, //descrgetfunc tp_descr_get;
    NULL, //descrsetfunc tp_descr_set;
    NULL, //Py_ssize_t tp_dictoffset;
    (initproc)nitro_XmlWriter_init,//initproc tp_init;
    NULL, //allocfunc tp_alloc;
    nitro_XmlWriter_new, //newfunc tp_new;
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
PyTypeObject nitro_XmlWriterType = {

    PyObject_HEAD_INIT(NULL)
    0,                         /*ob_size*/
    "nitro.XmlWriter",            /*tp_name*/
    sizeof(nitro_XmlWriterObject),  /*tp_basicsize*/
    0,                         /*tp_itemsize*/
    (destructor)nitro_XmlWriter_dealloc,   /*tp_dealloc*/
    0,                         /*tp_print*/
    0,                         /*tp_getattr*/
    0,                         /*tp_setattr*/
    0,                         /*tp_compare*/
    0,                         /*tp_repr*/
    0,                         /*tp_as_number*/
    0,                         /*tp_as_sequence*/
    0,                     /*tp_as_mapping*/
    0,                         /*tp_hash */
    0,                         /*tp_call*/
    0,                        /*tp_str*/
    0,                      /*tp_getattro*/
    0,                          /*tp_setattro*/
    0,                         /*tp_as_buffer*/
    Py_TPFLAGS_DEFAULT|Py_TPFLAGS_BASETYPE,   /*tp_flags*/
    "Nitro XmlWriter Object",           /* tp_doc */
    0,                    /* tp_traverse */
    0,                     /* tp_clear */
    0,                     /* tp_richcompare */
    0,                     /* tp_weaklistoffset */
    0,                      /* tp_iter */
    0,                     /* tp_iternext */
    nitro_XmlWriter_Methods,     /* tp_methods */
    0,                       /* tp_members */
    0,                         /* tp_getset */
    0,                         /* tp_base */
    0,                         /* tp_dict */
    0,                         /* tp_descr_get */
    0,                         /* tp_descr_set */
    0,                         /* tp_dictoffset */
    (initproc)nitro_XmlWriter_init, /* tp_init */
    0,                         /* tp_alloc */
    nitro_XmlWriter_new,            /* tp_new */
};
#endif

