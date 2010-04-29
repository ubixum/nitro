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

#include <pynitro/node.h>


#include <structmember.h>

#include <iostream>
#include <string>

#include <pynitro/nitro_pyutil.h>

using namespace std;
using namespace Nitro;


// predef
PyObject* nitro_Node_GetIter ( PyObject* s ); 
PyObject* nitro_Node_AddChild(nitro_NodeObject* self, PyObject* arg);
static int nitro_Node_SetAttr(PyObject* s, PyObject* attr, PyObject* v);

// **************** new/delete *********************

static PyObject* 
nitro_Node_new ( PyTypeObject* type, PyObject *args, PyObject *kwds ) {
    nitro_NodeObject *self = (nitro_NodeObject*)type->tp_alloc(type,0);

    self->m_node = new Nitro::NodeRef();
    return (PyObject*)self;
}

static void nitro_Node_dealloc (nitro_NodeObject* self) {
    delete self->m_node;
    self->ob_type->tp_free((PyObject*)self);
}


// ********************** init *****************************

static int
node_init_help ( nitro_NodeObject* self, PyObject *args, PyObject *kwds, NodeRef target_node ) {

  PyObject* init;
  PyObject* arg2=NULL;
  PyObject* arg3=NULL; 
  // one arg is a string for a node name or 
  // a cobject node ref
  *(self->m_node) = target_node;
  if (!PyArg_ParseTuple(args,"O|OO",&init, &arg2, &arg3)) {
    return -1;
  }
    
  if ( PyString_Check(init) ) {
      const char* name = PyString_AsString(init);
      target_node->set_name ( name );
  } else if ( PyCObject_Check(init) ) {
      *(self->m_node) = *(Nitro::NodeRef*)PyCObject_AsVoidPtr(init);
  } else {
      PyErr_SetObject ( PyExc_Exception, init );
      return -1;
  }

  // two args is the attrs and the children nodes.
  if (arg2 != NULL && arg3 != NULL) {
    // ((attr,1)..), [child1, child2,..] 
    // set the attrs
    PyObject* iter = PyObject_GetIter(arg2);
    PyObject* item;
    if (NULL==iter) {
      PyErr_SetObject ( PyExc_Exception, init ); 
      return -1;
    }
    while ( (item = PyIter_Next(iter)) ) {
       // tuple key, value
       PyObject* key = PySequence_GetItem ( item, 0 );
       if ( NULL == key ) {
          PyErr_SetObject ( PyExc_Exception, item );
          Py_DECREF(iter);
          return -1;
       }
       PyObject* val = PySequence_GetItem ( item, 1 );
       if (NULL == val) {
          PyErr_SetObject ( PyExc_Exception, item );
          Py_DECREF(iter);
          return -1;
       }
       if (nitro_Node_SetAttr((PyObject*)self,key,val)) {
         Py_DECREF(iter);
         return -1;
       }
    }
    Py_DECREF(iter);
    if (PyErr_Occurred()) return -1; 


    // ok, now the children
    //
    iter = PyObject_GetIter(arg3);
    if (NULL==iter) {
        PyErr_SetObject ( PyExc_Exception , arg2 );
        return -1;
    }

    while ( (item = PyIter_Next(iter)) ) {
         if (NULL==nitro_Node_AddChild ( self, item )){
            Py_DECREF(iter);
            return -1; 
         }
    }
    Py_DECREF(iter);
    if (PyErr_Occurred()) return -1;
  }
    
  return 0;
}

static int
nitro_Node_init ( nitro_NodeObject* self, PyObject *args, PyObject *kwds ) {

    return node_init_help( self, args, kwds, Node::create("tmp") );
}


// *********** class methods ****************************



PyObject* nitro_Node_Keys ( nitro_NodeObject* self ) {
   try {
       NodeRef &n = (*self->m_node);
       PyObject* keys = PyList_New(n->num_children());
       int i=0;
       for (DITreeIter ti = n->child_begin(); ti != n->child_end(); ++ti ) {
            NodeRef c = *ti;
            PyList_SetItem ( keys, i++, PyString_FromString ( c->get_name().c_str() ) ); 
       }
       return keys; 
   } catch ( const Exception &e) {
    NITRO_EXC(e,NULL);
   }
 
}

PyObject* nitro_Node_Attr_Keys ( nitro_NodeObject* self ) {
   try {
       NodeRef &n = (*self->m_node);
       PyObject* keys = PyList_New(n->num_attrs());
       int i=0;
       for (DIAttrIter ti = n->attrs_begin(); ti != n->attrs_end(); ++ti ) {
	 PyList_SetItem ( keys, i++, PyString_FromString ( ti->first.c_str()));
       }
       return keys; 
   } catch ( const Exception &e) {
    NITRO_EXC(e,NULL);
   }
 
}


PyObject* nitro_Node_Values( nitro_NodeObject* self) {
   try {
       NodeRef &n = (*self->m_node);
       PyObject* values = PyList_New(n->num_children());
       int i=0;
       for (DITreeIter ti = n->child_begin(); ti != n->child_end(); ++ti ) {
            NodeRef c = *ti;
            PyList_SetItem ( values, i++, nitro_BuildNode ( c ) ); 
       }
       return values; 
   } catch ( const Exception &e) {
    NITRO_EXC(e,NULL);
   } 
}

PyObject* nitro_Node_Attr_Values( nitro_NodeObject* self) {
   try {
       NodeRef &n = (*self->m_node);
       PyObject* values = PyList_New(n->num_attrs());
       int i=0;
       for (DIAttrIter ti = n->attrs_begin(); ti != n->attrs_end(); ++ti ) {
	 PyObject *v = from_datatype(ti->second);
	 if(!v) { Py_DECREF(values); return NULL; }
	 PyList_SetItem ( values, i++, v);
       }
       return values; 
   } catch ( const Exception &e) {
    NITRO_EXC(e,NULL);
   } 
}

PyObject* nitro_Node_Clone(nitro_NodeObject* self) {
    try {
        NodeRef &n = (*self->m_node);
        NodeRef copy = n->clone();
        PyObject* pycopy = from_datatype(copy);
        return pycopy;
    } catch ( const Exception &e) {
        NITRO_EXC(e,NULL);
    }
}

PyObject* nitro_Node_AddChild(nitro_NodeObject* self, PyObject* arg) {

    if (!PyObject_TypeCheck(arg, &nitro_NodeType)) {
        PyErr_SetObject(PyExc_Exception, arg);
        return NULL;
    }

    try {
        NodeRef *ref = ((nitro_NodeObject*)arg)->m_node;
        (*self->m_node)->add_child(*ref);
        Py_RETURN_NONE;
    } catch ( const Exception &e) {
        NITRO_EXC(e,NULL);
    }
}    

PyObject* nitro_Node_IterKeys(nitro_NodeObject* self ) {
    return nitro_Node_GetIter( (PyObject*)self );
}


PyObject* nitro_Node_NumChildren(nitro_NodeObject* self ) {
    try {
        return Py_BuildValue("i", (*((nitro_NodeObject*)self)->m_node)->num_children() );
    } catch ( const Exception& e ) {
        NITRO_EXC(e,NULL);
    }
}


PyObject* nitro_Node_Items(nitro_NodeObject* self) {
    try {
       NodeRef node = *(self->m_node);
       PyObject* items = PyList_New(node->num_children());
       int i=0;
       for (DITreeIter ti = node->child_begin(); ti != node->child_end(); ++ti ) {
           NodeRef c = *ti;
           PyObject* tuple = PyTuple_New(2);
           PyTuple_SetItem(tuple, 0, PyString_FromString ( c->get_name().c_str() )); 
           PyTuple_SetItem(tuple, 1, nitro_BuildNode ( c )); 
           PyList_SetItem(items, i++, tuple ); // tuple ref stolen
       }
       return items;
    } catch ( const Exception& e ) {
        NITRO_EXC(e,NULL);
    }
}

PyObject* nitro_Node_Attr_Items(nitro_NodeObject* self) {
    try {
       NodeRef node = *(self->m_node);
       PyObject* items = PyList_New(node->num_attrs());
       int i=0;
       for (DIAttrIter ti = node->attrs_begin(); ti != node->attrs_end(); ++ti ) {
           PyObject* tuple = PyTuple_New(2);
           PyTuple_SetItem(tuple, 0, PyString_FromString ( ti->first.c_str() )); 
	   PyObject *v = from_datatype(ti->second);
	   if(!v) { Py_DECREF(tuple); Py_DECREF(items); return NULL; }
           PyTuple_SetItem(tuple, 1, v );
           PyList_SetItem(items, i++, tuple ); // tuple ref stolen
       }
       return items;
    } catch ( const Exception& e ) {
        NITRO_EXC(e,NULL);
    }
}

PyObject* nitro_Node_reduce (nitro_NodeObject* self) {
    // list  of attrs
    // and of children
    return Py_BuildValue ( "O(sNN)",
        self->ob_type,
        (*self->m_node)->get_name().c_str(),
        nitro_Node_Attr_Items ( self ),
        nitro_Node_Values ( self ) );
}

static PyMethodDef nitro_Node_Methods[] = {
    {"keys", (PyCFunction)nitro_Node_Keys, METH_NOARGS, "keys() -> list" },
    {"iterkeys", (PyCFunction)nitro_Node_IterKeys, METH_NOARGS, "iterkeys() -> key iterator" },
    {"values", (PyCFunction)nitro_Node_Values, METH_NOARGS, "values() -> list" },
    {"add_child", (PyCFunction)nitro_Node_AddChild, METH_O, "add_child(node)" },
    {"num_children", (PyCFunction)nitro_Node_NumChildren, METH_NOARGS, "num_children()->i" },
    {"items", (PyCFunction)nitro_Node_Items, METH_NOARGS, "items()-> list of tuples" },
    {"attr_keys", (PyCFunction)nitro_Node_Attr_Keys, METH_NOARGS, "attribute keys" },
    {"attr_values", (PyCFunction)nitro_Node_Attr_Values, METH_NOARGS, "attribute values" },
    {"attr_items", (PyCFunction)nitro_Node_Attr_Items, METH_NOARGS, "attribute items" },
    {"clone", (PyCFunction)nitro_Node_Clone, METH_NOARGS, "create clone of node" },
    {"__reduce__", (PyCFunction)nitro_Node_reduce, METH_NOARGS, "Reduce for pickling" },
    {NULL}
};


// *********** attr functions **************

static PyObject* nitro_Node_GetAttr(PyObject* s, PyObject* attr) {

    // check for class attrs 1st
  PyObject *tmp;
  if (!(tmp = PyObject_GenericGetAttr(s, attr))) {
      if (!PyErr_ExceptionMatches(PyExc_AttributeError))
         return NULL; // some error occurred, propagate it.
      PyErr_Clear();
   } else {
        // found a class attribute
        return tmp;
   }

   // no class attr found.  
   nitro_NodeObject* self=(nitro_NodeObject*)s;
   const char* a;
   if (!PyString_Check(attr)) {
    PyErr_SetObject(PyExc_Exception, attr);
    return NULL;
   }
   a=PyString_AsString(attr);


   if (!strcmp(a,"name")) {
       return Py_BuildValue("s",(*self->m_node)->get_name().c_str());
   }

   try {
       DataType dt = (*self->m_node)->get_attr( string(a) );
       return from_datatype ( dt );
   } catch ( const Exception &e) {
    NITRO_EXC(e,NULL);
   }
}

static int nitro_Node_SetAttr(PyObject* s, PyObject* attr, PyObject* v) {
    nitro_NodeObject* self=(nitro_NodeObject*)s;
    const char* a;
    if (!PyString_Check(attr)) {
        PyErr_SetObject ( PyExc_Exception, attr );
        return -1;
    }


    a=PyString_AsString(attr);
    try {
        DataType dt(0);
        if ( !to_datatype ( v, &dt ) ) return -1;
        if (!strcmp(a,"name")) {
            (*self->m_node)->set_name ( (std::string)dt ) ;
        } else {
            (*self->m_node)->set_attr( a, dt );
        }
        return 0;
    } catch ( const Exception &e) {
        NITRO_EXC(e,-1);
    }
}


// ********** mapping functions *************

static Py_ssize_t nitro_Node_Len (PyObject* self) {
    try {
        return (*((nitro_NodeObject*)self)->m_node)->num_children();
    } catch ( const Exception &e) {
       NITRO_EXC(e,-1); 
    }
}

static PyObject* nitro_Node_GetItem (PyObject* s, PyObject* key) {
    nitro_NodeObject* self=(nitro_NodeObject*)s;

    const char* a;
    if (!PyString_Check(key)) {
        PyErr_SetObject(PyExc_Exception, key);
        return NULL;
    }
    a=PyString_AsString(key);

    try {

        NodeRef ref = (*self->m_node)->get_child(a);
        return nitro_BuildNode(ref);
        
    } catch (const Exception &e) {
        NITRO_EXC(e,NULL);
    }

}

static int nitro_Node_SetItem(PyObject* s, PyObject* key, PyObject *v) {
    if (!PyString_Check(key)) {
        PyErr_SetObject(PyExc_Exception, key );
        return -1;
    }
    const char* k = PyString_AsString(key);
    nitro_NodeObject* self = (nitro_NodeObject*)s;
    try {
        if (NULL==v) {
           // delete item
           (*self->m_node)->del_child(k); 
        } else {
           if (!PyObject_TypeCheck(v, &nitro_NodeType )) {
             PyErr_SetObject(PyExc_Exception, v );
             return -1;
           }

           DataType new_dt(0);
           if (to_datatype(v,&new_dt)) {
               NodeRef new_child = (NodeRef)new_dt;
               new_child->set_name(k); // overwrite name if they passed in a different key
               (*self->m_node)->add_child(new_child);
           }
        }
    } catch ( Exception &e ) {
        NITRO_EXC(e,-1);
    }
    return 0;
}


PyMappingMethods nitro_Node_Mapping = {
   nitro_Node_Len,
   nitro_Node_GetItem,
   nitro_Node_SetItem
};


// ************************ Iter **************************

PyObject* nitro_Node_GetIter ( PyObject* s ) {
    PyObject* l = nitro_Node_Keys((nitro_NodeObject*)s); 
    if (!l) return NULL; // error set by nitro_Node_Keys

    PyObject* itr = PyList_Type.tp_iter ( l );
    Py_DECREF ( l );
    return itr;
}


// ************************ str function *******************


PyObject* raw_str(const NodeRef& n) {
    PyObject* str = PyString_FromString ( "{'name':'" );
    PyString_ConcatAndDel ( &str, PyString_FromString ( n->get_name().c_str() ) );
    PyString_ConcatAndDel ( &str, PyString_FromString ( "', " ) );
    for (DIAttrIter ai=n->attrs_begin(); ai != n->attrs_end(); ++ai ) {
       PyString_ConcatAndDel ( &str, PyString_FromString ( "'" ) );
       PyString_ConcatAndDel ( &str, PyString_FromString ( ai->first.c_str() ) );
       PyString_ConcatAndDel ( &str, PyString_FromString ( "':" ) );
       bool isint = ai->second.get_type() != STR_DATA;
       if (!isint) PyString_ConcatAndDel (&str, PyString_FromString ( "'" ) );
       PyString_ConcatAndDel ( &str, PyString_FromString ( ((string)ai->second).c_str() ) );
       if (!isint) PyString_ConcatAndDel ( &str, PyString_FromString ( "'" ) );
       PyString_ConcatAndDel ( &str, PyString_FromString ( ", " ) );
    }
    if (n->has_children()) {
        PyString_ConcatAndDel ( &str, PyString_FromString ( "children: [\n" ) );
        for (DITreeIter ti = n->child_begin(); ti != n->child_end(); ++ti ) {
            NodeRef c=*ti;
            PyString_ConcatAndDel ( &str, raw_str(c) ); 
            PyString_ConcatAndDel ( &str, PyString_FromString ( ", \n" ) );
        }
        PyString_ConcatAndDel ( &str, PyString_FromString("]") );
    }
    PyString_ConcatAndDel ( &str, PyString_FromString("}") );
    return str;
}

PyObject* nitro_Node_Repr(PyObject* s) {

   try {
    NodeRef &n = *((nitro_NodeObject*)s)->m_node;
    return raw_str(n);
    
   } catch ( const Exception &e) {
    NITRO_EXC(e,NULL);
   }

}


PyTypeObject nitro_NodeType = {

    PyObject_HEAD_INIT(NULL)
    0,                         /*ob_size*/
    "nitro.Node",            /*tp_name*/
    sizeof(nitro_NodeObject),  /*tp_basicsize*/
    0,                         /*tp_itemsize*/
    (destructor)nitro_Node_dealloc,   /*tp_dealloc*/
    0,                         /*tp_print*/
    0,                         /*tp_getattr*/
    0,                         /*tp_setattr*/
    0,                         /*tp_compare*/
    nitro_Node_Repr,           /*tp_repr*/
    0,                         /*tp_as_number*/
    0,                         /*tp_as_sequence*/
    &nitro_Node_Mapping,       /*tp_as_mapping*/
    0,                         /*tp_hash */
    0,                         /*tp_call*/
    0,                        /*tp_str*/
    nitro_Node_GetAttr,        /*tp_getattro*/
    nitro_Node_SetAttr,        /*tp_setattro*/
    0,                         /*tp_as_buffer*/
    Py_TPFLAGS_DEFAULT|Py_TPFLAGS_BASETYPE,   /*tp_flags*/
    "Nitro Node Object",           /* tp_doc */
    0,                    /* tp_traverse */
    0,                     /* tp_clear */
    0,                     /* tp_richcompare */
    0,                     /* tp_weaklistoffset */
    nitro_Node_GetIter,    /* tp_iter */
    0,                     /* tp_iternext */
    nitro_Node_Methods,     /* tp_methods */
    0,                       /* tp_members */
    0,                         /* tp_getset */
    0,                         /* tp_base */
    0,                         /* tp_dict */
    0,                         /* tp_descr_get */
    0,                         /* tp_descr_set */
    0,                         /* tp_dictoffset */
    (initproc)nitro_Node_init,  /* tp_init */
    0,                         /* tp_alloc */
    nitro_Node_new,                 /* tp_new */
};


// ************************** extending classes ***********************************



static int
nitro_DI_init ( nitro_DeviceInterfaceObject* self, PyObject *args, PyObject *kwds ) {
  return node_init_help ( (nitro_NodeObject*)self, args, kwds, DeviceInterface::create("tmp") );
}

PyTypeObject nitro_DeviceInterfaceType = {

    PyObject_HEAD_INIT(NULL)
    0,                         /*ob_size*/
    "nitro.DeviceInterface",            /*tp_name*/
    sizeof(nitro_DeviceInterfaceObject),  /*tp_basicsize*/
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
    Py_TPFLAGS_DEFAULT|Py_TPFLAGS_BASETYPE,   /*tp_flags*/
    "Nitro Device Interface Object", /* tp_doc */
    0,                    /* tp_traverse */
    0,                     /* tp_clear */
    0,                     /* tp_richcompare */
    0,                     /* tp_weaklistoffset */
    0,                     /* tp_iter */
    0,                     /* tp_iternext */
    0,                       /* tp_methods */
    0,                       /* tp_members */
    0,                         /* tp_getset */
    &nitro_NodeType,           /* tp_base */
    0,                         /* tp_dict */
    0,                         /* tp_descr_get */
    0,                         /* tp_descr_set */
    0,                         /* tp_dictoffset */
    (initproc)nitro_DI_init,  /* tp_init */
    0,                         /* tp_alloc */
    0,                       /* tp_new */
};

// ******************* Terminal *********************************8

static int
nitro_Term_init ( nitro_TerminalObject* self, PyObject *args, PyObject *kwds ) {

   return node_init_help ( (nitro_NodeObject*)self, args, kwds, Terminal::create("tmp") );
}

PyTypeObject nitro_TerminalType = {

    PyObject_HEAD_INIT(NULL)
    0,                         /*ob_size*/
    "nitro.Terminal",            /*tp_name*/
    sizeof(nitro_TerminalObject),  /*tp_basicsize*/
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
    Py_TPFLAGS_DEFAULT|Py_TPFLAGS_BASETYPE,   /*tp_flags*/
    "Nitro Terminal Object", /* tp_doc */
    0,                    /* tp_traverse */
    0,                     /* tp_clear */
    0,                     /* tp_richcompare */
    0,                     /* tp_weaklistoffset */
    0,                     /* tp_iter */
    0,                     /* tp_iternext */
    0,                       /* tp_methods */
    0,                       /* tp_members */
    0,                         /* tp_getset */
    &nitro_NodeType,           /* tp_base */
    0,                         /* tp_dict */
    0,                         /* tp_descr_get */
    0,                         /* tp_descr_set */
    0,                         /* tp_dictoffset */
    (initproc)nitro_Term_init,  /* tp_init */
    0,                         /* tp_alloc */
    0,                       /* tp_new */
};

// ******************* Register *********************************8

static int
nitro_Register_init ( nitro_RegisterObject* self, PyObject *args, PyObject *kwds ) {

    return node_init_help ( (nitro_NodeObject*)self, args, kwds, Register::create("tmp") );

}

PyTypeObject nitro_RegisterType = {

    PyObject_HEAD_INIT(NULL)
    0,                         /*ob_size*/
    "nitro.Register",            /*tp_name*/
    sizeof(nitro_RegisterObject),  /*tp_basicsize*/
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
    Py_TPFLAGS_DEFAULT|Py_TPFLAGS_BASETYPE,   /*tp_flags*/
    "Nitro Register Object", /* tp_doc */
    0,                    /* tp_traverse */
    0,                     /* tp_clear */
    0,                     /* tp_richcompare */
    0,                     /* tp_weaklistoffset */
    0,                     /* tp_iter */
    0,                     /* tp_iternext */
    0,                       /* tp_methods */
    0,                       /* tp_members */
    0,                         /* tp_getset */
    &nitro_NodeType,           /* tp_base */
    0,                         /* tp_dict */
    0,                         /* tp_descr_get */
    0,                         /* tp_descr_set */
    0,                         /* tp_dictoffset */
    (initproc)nitro_Register_init,  /* tp_init */
    0,                         /* tp_alloc */
    0,                       /* tp_new */
};

// ******************* Subegister *********************************8

static int
nitro_Subregister_init ( nitro_SubregisterObject* self, PyObject *args, PyObject *kwds ) {
    return node_init_help ( (nitro_NodeObject*)self, args, kwds, Subregister::create("tmp") );
}

PyTypeObject nitro_SubregisterType = {

    PyObject_HEAD_INIT(NULL)
    0,                         /*ob_size*/
    "nitro.Subregister",            /*tp_name*/
    sizeof(nitro_SubregisterObject),  /*tp_basicsize*/
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
    Py_TPFLAGS_DEFAULT|Py_TPFLAGS_BASETYPE,   /*tp_flags*/
    "Nitro Subregister Object", /* tp_doc */
    0,                    /* tp_traverse */
    0,                     /* tp_clear */
    0,                     /* tp_richcompare */
    0,                     /* tp_weaklistoffset */
    0,                     /* tp_iter */
    0,                     /* tp_iternext */
    0,                       /* tp_methods */
    0,                       /* tp_members */
    0,                         /* tp_getset */
    &nitro_NodeType,           /* tp_base */
    0,                         /* tp_dict */
    0,                         /* tp_descr_get */
    0,                         /* tp_descr_set */
    0,                         /* tp_dictoffset */
    (initproc)nitro_Subregister_init,  /* tp_init */
    0,                         /* tp_alloc */
    0,                       /* tp_new */
};

// ******************* Valuemap *********************************8

static int
nitro_Valuemap_init( nitro_ValuemapObject* self, PyObject *args, PyObject *kwds ) {
    return node_init_help ( (nitro_NodeObject*)self, args, kwds, Valuemap::create("tmp") );
}

PyTypeObject nitro_ValuemapType = {

    PyObject_HEAD_INIT(NULL)
    0,                         /*ob_size*/
    "nitro.Valuemap",            /*tp_name*/
    sizeof(nitro_ValuemapObject),  /*tp_basicsize*/
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
    Py_TPFLAGS_DEFAULT|Py_TPFLAGS_BASETYPE,   /*tp_flags*/
    "Nitro Valuemap Object", /* tp_doc */
    0,                    /* tp_traverse */
    0,                     /* tp_clear */
    0,                     /* tp_richcompare */
    0,                     /* tp_weaklistoffset */
    0,                     /* tp_iter */
    0,                     /* tp_iternext */
    0,                       /* tp_methods */
    0,                       /* tp_members */
    0,                         /* tp_getset */
    &nitro_NodeType,           /* tp_base */
    0,                         /* tp_dict */
    0,                         /* tp_descr_get */
    0,                         /* tp_descr_set */
    0,                         /* tp_dictoffset */
    (initproc)nitro_Valuemap_init,  /* tp_init */
    0,                         /* tp_alloc */
    0,                       /* tp_new */
};

// *************** helper ***********************
//

PyObject* nitro_BuildNode ( const NodeRef & node ) { 
        PyObject* node_obj = PyCObject_FromVoidPtr((void*)&node, NULL);
        PyObject* arg = Py_BuildValue ( "(O)", node_obj );
        PyObject* ret = PyObject_CallObject ( (PyObject*)&nitro_NodeType, arg );
        Py_DECREF(arg);
        Py_DECREF(node_obj);
        return ret;
}
