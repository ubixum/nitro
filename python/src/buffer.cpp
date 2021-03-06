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


#include <pynitro/buffer.h>
#include <pynitro/nitro_pyutil.h>

static PyObject* 
nitro_Buffer_new ( PyTypeObject* type, PyObject *args, PyObject *kwds ) {
    nitro_BufferObject *self = (nitro_BufferObject*)type->tp_alloc(type,0);

    return (PyObject*)self;
}

static void nitro_Buffer_dealloc (nitro_BufferObject* self) {
#if PY_MAJOR_VERSION >= 3
  self->ob_base.ob_type->tp_free((PyObject*)self);
#else
  self->ob_type->tp_free((PyObject*)self);
#endif
}




static int
nitro_Buffer_init(nitro_BufferObject* self, PyObject* args, PyObject* kwds ) {
	PyObject* cobj=NULL;
	uint32 len = 0;
	if (!PyArg_ParseTuple(args, "OI", &cobj, &len ) ) {
		PyErr_SetString ( PyExc_Exception, "Buffer(CObject,len)" );
		return -1;
	}
	if (!PyCapsule_CheckExact(cobj)) {
		PyErr_SetString ( PyExc_Exception, "Buffer(CObject,len)" );
		return -1;
	}
	self->m_buf = (uint8*) PyCapsule_GetPointer(cobj, NULL);
	self->m_len = len;

	return 0;
	
} 

Py_ssize_t buffer_readproc( PyObject* self, Py_ssize_t seg, void** buf ) {
    
	nitro_BufferObject* b = (nitro_BufferObject*)self;
 	*buf = (void*)b->m_buf;
	return b->m_len;
}

Py_ssize_t buffer_seg_count ( PyObject* self, Py_ssize_t* len ) {
    if (len) *len = ((nitro_BufferObject*)self)->m_len;
    return 1;
}


Py_ssize_t buffer_charbuffer ( PyObject* self, Py_ssize_t seg, char** buf ) {
    
	nitro_BufferObject* b = (nitro_BufferObject*)self;
 	*buf = (char*)b->m_buf;
	return b->m_len;
}


static 
#if PY_MAJOR_VERSION >= 3
PyBufferProcs buffer_procs = {
// TODO: needs implemented for python3
//	buffer_readproc, // read buffer
//	buffer_readproc, // write buffer ( they are both the same )
//	buffer_seg_count,
//	(charbufferproc)buffer_charbuffer
};
#else
PyBufferProcs buffer_procs = {
	buffer_readproc, // read buffer
	buffer_readproc, // write buffer ( they are both the same )
	buffer_seg_count,
	(charbufferproc)buffer_charbuffer
};
#endif

Py_ssize_t nitro_Buffer_Length(PyObject *o) {
    return ((nitro_BufferObject*)o)->m_len;
}

PyObject* nitro_Buffer_GetItem(PyObject *o, Py_ssize_t i) {
    nitro_BufferObject* self = (nitro_BufferObject*)o;
    if ( i > self->m_len-1 ) {
        PyErr_SetString(PyExc_Exception,"Out of bounds." );
        return NULL;
    }
    return Py_BuildValue( "I", ((nitro_BufferObject*)o)->m_buf[i] );
}

int nitro_Buffer_SetItem(PyObject *o, Py_ssize_t i, PyObject *v) {

    nitro_BufferObject* self = (nitro_BufferObject*)o;

    if ( i > self->m_len-1 ) {
        PyErr_SetString(PyExc_Exception,"Out of bounds." );
        return -1;
    }
 
    long l = PyLong_AsLong(v); 
    if (PyErr_Occurred()) { return -1; }
    self->m_buf[i] = (uint8) l;

    return 0;
}

static 
PySequenceMethods buffer_sequence= {
    nitro_Buffer_Length,
    NULL, //binaryfunc PySequenceMethods.sq_concat 
    NULL, //ssizeargfunc PySequenceMethods.sq_repeat 
    nitro_Buffer_GetItem, //ssizeargfunc PySequenceMethods.sq_item 
    NULL, // slice
    nitro_Buffer_SetItem, //ssizeobjargproc PySequenceMethods.sq_ass_item 
    NULL, // set slice
    NULL, //objobjproc PySequenceMethods.sq_contains 
    NULL, //binaryfunc PySequenceMethods.sq_inplace_concat 
    NULL //ssizeargfunc PySequenceMethods.sq_inplace_repeat 
};
#if PY_MAJOR_VERSION >= 3
PyTypeObject nitro_BufferType = {
    PyVarObject_HEAD_INIT(NULL, 0)
	"nitro.Buffer", //const char *tp_name; For printing, in format "<module>.<name>"
	sizeof(nitro_BufferObject), //Py_ssize_t tp_basicsize, 
	NULL, //tp_itemsize; /* For allocation */

	/* Methods to implement standard operations */
	(destructor)nitro_Buffer_dealloc,	//destructor tp_dealloc;
	NULL, //printfunc tp_print;
	NULL, //getattrfunc tp_getattr;
	NULL, //setattrfunc tp_setattr;
	NULL, //PyAsyncMethods *tp_as_async; formerly known as tp_compare (Python 2) or tp_reserved (Python 3)
	NULL, //reprfunc tp_repr;

	/* Method suites for standard classes */

	NULL, //PyNumberMethods *tp_as_number;
	&buffer_sequence, //PySequenceMethods *tp_as_sequence;
	NULL, //PyMappingMethods *tp_as_mapping;

	///* More standard operations (here for binary compatibility) */
	NULL, //hashfunc tp_hash;
	NULL, //ternaryfunc tp_call;
	NULL, //reprfunc tp_str;
	NULL, //getattrofunc tp_getattro;
	NULL, //setattrofunc tp_setattro;

	///* Functions to access object as input/output buffer */
	&buffer_procs, //PyBufferProcs *tp_as_buffer;

	///* Flags to define presence of optional/expanded features */
	Py_TPFLAGS_DEFAULT,	//unsigned long tp_flags;

	"Nitro Buffer Object", //const char *tp_doc; /* Documentation string */

	///* call function for all accessible objects */
	NULL, //traverseproc tp_traverse;

	///* delete references to contained objects */
	NULL, //inquiry tp_clear;

	///* rich comparisons */
	NULL, //richcmpfunc tp_richcompare;

	///* weak reference enabler */
	NULL, //Py_ssize_t tp_weaklistoffset;

	///* Iterators */
	NULL, //getiterfunc tp_iter;
	NULL, //iternextfunc tp_iternext;

	///* Attribute descriptor and subclassing stuff */
	NULL, //struct PyMethodDef *tp_methods;
	NULL, //struct PyMemberDef *tp_members;
	NULL, //struct PyGetSetDef *tp_getset;
	NULL, //struct _typeobject *tp_base;
	NULL, //PyObject *tp_dict;
	NULL, //descrgetfunc tp_descr_get;
	NULL, //descrsetfunc tp_descr_set;
	NULL, //Py_ssize_t tp_dictoffset;
	(initproc)nitro_Buffer_init, //initproc tp_init;
	NULL, //allocfunc tp_alloc;
	nitro_Buffer_new, //newfunc tp_new;
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
PyTypeObject nitro_BufferType = {

    PyObject_HEAD_INIT(NULL)
    0,                         /*ob_size*/
    "nitro.Buffer",            /*tp_name*/
    sizeof(nitro_BufferObject),  /*tp_basicsize*/
    0,                         /*tp_itemsize*/
    (destructor)nitro_Buffer_dealloc,   /*tp_dealloc*/
    0,                         /*tp_print*/
    0,                         /*tp_getattr*/
    0,                         /*tp_setattr*/
    0,                         /*tp_compare*/
    0,		           /*tp_repr*/
    0,                         /*tp_as_number*/
    &buffer_sequence,   /*tp_as_sequence*/
    0,			       /*tp_as_mapping*/
    0,                         /*tp_hash */
    0,                         /*tp_call*/
    0,                        /*tp_str*/
    0,		        /*tp_getattro*/
    0,       		 /*tp_setattro*/
    &buffer_procs,  /*tp_as_buffer*/
    Py_TPFLAGS_DEFAULT|Py_TPFLAGS_HAVE_GETCHARBUFFER,   /*tp_flags*/
    "Nitro Buffer Object",           /* tp_doc */
    0,                    /* tp_traverse */
    0,                     /* tp_clear */
    0,                     /* tp_richcompare */
    0,                     /* tp_weaklistoffset */
    0,    /* tp_iter */
    0,                     /* tp_iternext */
    0,     /* tp_methods */
    0,                       /* tp_members */
    0,                         /* tp_getset */
    0,                         /* tp_base */
    0,                         /* tp_dict */
    0,                         /* tp_descr_get */
    0,                         /* tp_descr_set */
    0,                         /* tp_dictoffset */
    (initproc)nitro_Buffer_init,  /* tp_init */
    0,                         /* tp_alloc */
    nitro_Buffer_new,                 /* tp_new */
};
#endif

PyObject* nitro_BuildBuffer ( uint8* buf, uint32 len ) { 
  PyObject* buf_obj = PyCapsule_New((void*)buf, NULL,NULL);
        PyObject* arg = Py_BuildValue ( "OI", buf_obj, len );
        PyObject* ret = PyObject_CallObject ( (PyObject*)&nitro_BufferType, arg );
        Py_DECREF(arg);
        Py_DECREF(buf_obj);
        return ret;
}

