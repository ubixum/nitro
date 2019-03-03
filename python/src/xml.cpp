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
    .tp_name="nitro.XmlReader",            /*tp_name*/
    .tp_basicsize=sizeof(nitro_XmlReaderObject),  /*tp_basicsize*/
    .tp_dealloc=(destructor)nitro_XmlReader_dealloc,   /*tp_dealloc*/
    .tp_flags=Py_TPFLAGS_DEFAULT|Py_TPFLAGS_BASETYPE,   /*tp_flags*/
    .tp_doc="Nitro XmlReader Object",           /* tp_doc */
    .tp_methods=nitro_XmlReader_Methods,     /* tp_methods */
    .tp_init=(initproc)nitro_XmlReader_init, /* tp_init */
    .tp_new=nitro_XmlReader_new,            /* tp_new */
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
    .tp_name="nitro.XmlWriter",            /*tp_name*/
    .tp_basicsize=sizeof(nitro_XmlWriterObject),  /*tp_basicsize*/
    .tp_dealloc=(destructor)nitro_XmlWriter_dealloc,   /*tp_dealloc*/
    .tp_flags=Py_TPFLAGS_DEFAULT|Py_TPFLAGS_BASETYPE,   /*tp_flags*/
    .tp_doc="Nitro XmlWriter Object",           /* tp_doc */
    .tp_methods=nitro_XmlWriter_Methods,     /* tp_methods */
    .tp_init=(initproc)nitro_XmlWriter_init, /* tp_init */
    .tp_new=nitro_XmlWriter_new,            /* tp_new */
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

