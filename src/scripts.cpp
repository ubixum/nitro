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

#if PY_MINOR_VERSION <= 4
typedef int Py_ssize_t;
#endif


#include <map>
#include <string>
#include <sstream>

#include <nitro/scripts.h>
#include <nitro/error.h>
#include <nitro/types.h>
#include <nitro/node.h>

#include <python_nitro.h>

#if __APPLE__
// basename needs additional include
#include <libgen.h>
#endif

#include "xutils.h"

#ifdef DEBUG_PY
#include <iostream>
#define py_debug(x) std::cout << x << " (" << __FILE__ << ':' << __LINE__ << ')' << std::endl;
#else
#define py_debug(x)
#endif



// simple stack helper to decrement python objects when the function goes out of scope.
struct PyObjCleanup {
    PyObject* to_dec;
    PyObjCleanup(PyObject* pobj) : to_dec(pobj) {}
    ~PyObjCleanup() throw() { Py_XDECREF(to_dec); }
};





namespace Nitro {


typedef std::map<std::string,PyObject*> pyfuncs_t;
typedef std::map<std::string,pyfuncs_t> pymodules_t;

struct Scripts::impl {

    static int m_init_count;
    static PyObject* m_base_obj;
//    PyThreadState* m_py_thread_state;
//
    pymodules_t m_modules;
	PyObject* m_sys;
    
    impl () ; 
    ~impl() throw();

    void cleanup();

	/**
	 * \return the absolute directory path to the file
	 **/
	std::string modname ( const std::string &path );
	void add_path ( const std::string &path );
};


int Scripts::impl::m_init_count=0;
PyObject* Scripts::impl::m_base_obj=NULL;

Scripts::impl::impl() : m_sys(NULL) {

    if (!m_init_count) {
        Py_Initialize();
        if (!Py_IsInitialized()) throw Exception ( SCRIPTS_INIT  ); 
    }
    ++m_init_count;

	// sys needed for setting paths.
	m_sys = PyImport_ImportModule("sys");
	if ( !m_sys ) throw Exception ( SCRIPTS_INIT, "Unable to import sys module." );

	// On Windows, we need to add the path to our local nitro_py directory"
	#ifdef WIN32
    std::string inst_dir = get_inst_dir();
    add_path(xjoin(inst_dir, "nitro_py"));
	#endif

    if (import_nitro() < 0) throw Exception ( SCRIPTS_INIT, "Unable to import nitro module." ); 
    py_debug ( "Imported nitro module." );

        
    py_debug ("Scripts:impl: init count: " << m_init_count );
}

Scripts::impl::~impl() throw() {

    cleanup();

    --m_init_count;
    py_debug ( "Scripts::~impl:python init count: " << m_init_count );
    if (!m_init_count) {
        PyErr_Clear();
        Py_Finalize();
    }

   }

void Scripts::impl::cleanup() {
    
    for ( pymodules_t::iterator mitr = m_modules.begin(); mitr != m_modules.end(); ++mitr ) {
        pyfuncs_t &funcs = mitr->second;
        for ( pyfuncs_t::iterator itr = funcs.begin(); itr != funcs.end(); ++itr ) {
            Py_DECREF( itr->second );
        }
    }
	Py_XDECREF(m_sys);
}

std::string Scripts::impl::modname ( const std::string &path ) {
	
#ifdef WIN32
	char fname[_MAX_FNAME];
	_splitpath ( path.c_str(), NULL, NULL, fname, NULL );
	return std::string(fname);
#else

    char* path_tmp = strdup ( path.c_str() );
    char* base = basename(path_tmp);
    std::string filename ( base );
    free(path_tmp);
    if ( filename.length() < 4 ) throw Exception ( SCRIPTS_PATH, std::string("Invalid path to script file: ")+ path );
    if ( filename.substr ( filename.length() - 3 ) != ".py" ) throw Exception ( SCRIPTS_PATH, std::string("Invalid script file name: ") + path );
    return filename.substr ( 0, filename.length() - 3 );

#endif

}

void Scripts::impl::add_path ( const std::string &path ) {
	// set the path correctly for this module	
	PyObject* sys_path = PyObject_GetAttrString( m_sys, "path" );
	if (!sys_path) throw Exception ( SCRIPTS_ERR, "Unable to set module path" );
	PyObjCleanup sys_path_cleanup(sys_path);

	PyObject* dir_str = PyBytes_FromString ( path.c_str() );
	if ( !dir_str ) throw Exception ( SCRIPTS_ERR, "Unable to set module path (string creation)" );
	PyObjCleanup dir_str_cleanup( dir_str );

	if (!PySequence_Contains ( sys_path, dir_str ) ) {		
		if (PyList_Insert ( sys_path, 0, dir_str )<0) throw Exception ( SCRIPTS_ERR, "Unable to set module path." );

		#ifdef DEBUG_PY
		PyObject* path_str = PyObject_Str(sys_path);
		PyObjCleanup path_str_cleanup( path_str );
		py_debug ( "New Path: " << PyBytes_AsString( path_str ) );
		#endif
	}
}

Scripts::Scripts() : m_impl(new impl()) {}
Scripts::~Scripts() throw() { delete m_impl; }


void Scripts::import ( const std::string &module, const std::string &script_path ) {


	// add the script path to the module python path
	std::string dir = xdirname ( script_path );
	// for python, the strings need escaped
	std::string modname = m_impl->modname ( script_path );

	m_impl->add_path ( dir );	

    // the module name.
	py_debug ( "Attempt to load " << script_path << " module: " << modname );

    // the module
	PyObject* pymodule = PyImport_ImportModule ( (char*)modname.c_str() );
    if (!pymodule) {
        PyErr_Print();
        throw Exception ( SCRIPTS_SCRIPT, "Error loading script." );
    }
    PyObjCleanup pymodule_cleanup ( pymodule );

    PyObject* dict = PyObject_GetAttrString( pymodule, "__dict__");
    if (!dict) {
        PyErr_Print();
        throw Exception ( SCRIPTS_ERR, "Error reading script.  __dict__ attribute failure." );
    }
    PyObjCleanup dict_cleanup ( dict );

    if (!PyDict_Check(dict)) throw Exception ( SCRIPTS_ERR, "Error reading script.  __dict__ instance." );
    
    pyfuncs_t new_funcs;
    PyObject *key, *value;
    Py_ssize_t pos=0;
    while (PyDict_Next(dict, &pos, &key, &value)) {
        if (PyBytes_Check(key)) {
            const char* key_str = PyBytes_AsString(key);
            if (PyCallable_Check( value )) {
                Py_INCREF(value);
                new_funcs[std::string(key_str)] = value ;
                py_debug ( "Added funtion: " << key_str );
            }            
        }
    }

    pymodules_t::iterator mitr = m_impl->m_modules.find ( module );
    if (mitr != m_impl->m_modules.end()) {
        pyfuncs_t &funcs = mitr->second; 
        for (pyfuncs_t::iterator itr=funcs.begin();itr!=funcs.end();++itr) {
            Py_DECREF ( itr->second );
        }
    }
    m_impl->m_modules[module] = new_funcs;

}


std::vector<std::string> Scripts::func_list(const std::string &module) const  {
   
   pymodules_t::iterator mitr = m_impl->m_modules.find ( module );
   if (mitr == m_impl->m_modules.end()) throw Exception ( SCRIPTS_ERR, std::string("Module not found: ")+ module );
   pyfuncs_t &funcs = mitr->second;

   std::vector<std::string> func_names;
   for (pyfuncs_t::iterator itr=funcs.begin(); itr != funcs.end(); ++itr ){
     func_names.push_back ( itr->first );
   }
   return func_names;
}

NodeRef Scripts::get_params (const std::string &module, const std::string &func_name ) const {

   pymodules_t::iterator mitr = m_impl->m_modules.find ( module );
   if (mitr == m_impl->m_modules.end()) throw Exception ( SCRIPTS_SCRIPT, std::string("Module not found: ") + module );
   pyfuncs_t &funcs = mitr->second;
   pyfuncs_t::iterator itr = funcs.find ( func_name );
   if (itr == funcs.end() ) throw Exception ( SCRIPTS_SCRIPT, std::string("Function not found: ") + func_name );
   PyObject* func = itr->second;

   NodeRef params = Node::create("params");

   PyObject* doc_string = PyObject_GetAttrString(func,"__doc__");
   if (!doc_string) throw Exception ( SCRIPTS_SCRIPT, "Unable to read function doc string." );
   PyObjCleanup doc_string_cleanup(doc_string);
   
   const char* doc_cstr = PyBytes_AsString(doc_string);
   py_debug ( "Func Docs: " << doc_cstr );

   std::stringstream ss;
   ss << doc_cstr;
   std::string param, name, type;
   while ( !ss.eof() ) {
       ss >> param;
       if ( !ss.eof() && param== "@param" ) {
            ss >> name;
            if (ss.eof()) throw Exception ( SCRIPTS_SCRIPT, "Invalid doc string" );
            ss >> type; 
            if (ss.eof()) throw Exception ( SCRIPTS_SCRIPT, "Invalid doc string" );
            params->set_attr ( name, type );
            py_debug ( "Added param: " << name << ", " << type );
       }
   }
   
   return params; 

}

std::string Scripts::get_comment(const std::string &module, const std::string &func_name) const {
	pymodules_t::iterator mitr = m_impl->m_modules.find ( module );
    if (mitr == m_impl->m_modules.end()) throw Exception ( SCRIPTS_SCRIPT, std::string("Module not found: ") + module );
    pyfuncs_t &funcs = mitr->second;
    pyfuncs_t::iterator itr = funcs.find ( func_name );
    if (itr == funcs.end() ) throw Exception ( SCRIPTS_SCRIPT, std::string("Function not found: ") + func_name );
    PyObject* func = itr->second;

	PyObject* doc_string = PyObject_GetAttrString(func,"__doc__");
    if (!doc_string) throw Exception ( SCRIPTS_SCRIPT, "Unable to read function doc string." );
    PyObjCleanup doc_string_cleanup(doc_string);
   
    const char* doc_cstr = PyBytes_AsString(doc_string);
	return std::string(doc_cstr);
}


DataType Scripts::exec(const std::string &module, const std::string &func_name, const NodeRef& params ) const {

    pymodules_t::iterator mitr = m_impl->m_modules.find ( module );
    if (mitr == m_impl->m_modules.end()) throw Exception ( SCRIPTS_SCRIPT, std::string("Module not found: ") + module );
    pyfuncs_t &funcs = mitr->second;

    pyfuncs_t::iterator itr=funcs.find ( func_name );
    if (itr == funcs.end() ) throw Exception ( SCRIPTS_SCRIPT, std::string("Function not found: ") + func_name );
    PyObject* func = itr->second;

    PyObject* kw = PyDict_New();
    if (!kw) throw Exception ( SCRIPTS_ERR, "Unable to execute function. Unable to create param arguments." );
    PyObjCleanup kw_cleanup(kw);

    for (DIAttrIter aitr=params->attrs_begin(); aitr != params->attrs_end(); ++aitr ) {
       DataType d = aitr->second; 
       PyObject* arg_data = nitro_from_datatype(d);
       if (!arg_data ) {
        PyErr_Print();
        throw Exception ( SCRIPTS_SCRIPT, std::string("Unable to convert data type to script argument: ") + aitr->first );
       }
       PyObjCleanup arg_data_cleanup( arg_data ); // PyDict_SetItemString doesn't say it steals the ref 
       if ( PyDict_SetItemString ( kw, aitr->first.c_str(), arg_data ) < 0 ) throw Exception ( SCRIPTS_ERR, "Unable to create function arguments." );
    }

    #ifdef DEBUG_PY 
        PyObject* dstr = PyObject_Str(kw);
        const char* dstr_str = PyBytes_AsString(dstr);
        py_debug ( "keywords: " << dstr_str );
        Py_DECREF(dstr);
    #endif

    PyObject* empty_args = PyTuple_New(0);
    PyObjCleanup empty_args_cleanup(empty_args);
    PyObject *ret = PyObject_Call( func , empty_args, kw ); 
    if (!ret) {
        PyObject *ptype, *pvalue, *ptraceback;
        PyErr_Fetch(&ptype,&pvalue,&ptraceback);
        PyObjCleanup ptype_cleanup( ptype );
        PyObjCleanup pvalue_cleanup( pvalue );
        PyObjCleanup ptraceback_cleanup( ptraceback );

        if ( PyErr_GivenExceptionMatches ( pvalue, nitro_Exception ) ) {
            PyObject* eargs = PyObject_GetAttrString(pvalue,"args");
            PyObjCleanup eargs_cleanup(eargs);
            #ifdef DEBUG_PY
                PyObject* str_args = PyObject_Str(eargs);
                PyObjCleanup str_args_cleanup(str_args);
                py_debug ( "Exception Args: " << PyBytes_AsString(str_args) );
            #endif
            int code;
            const char* msg=NULL;
            PyObject* pdt=NULL;
            if (PyArg_ParseTuple ( eargs, "i|sO", &code, &msg, &pdt )) {
                if (msg) {
                    if ( pdt != NULL ) {
                        DataType d(0);
                        nitro_to_datatype ( pdt, &d );
                        throw Exception ( code, msg, d );
                    } else {
                        throw Exception ( code, msg );
                    }
                } else {
                    throw Exception ( (NITRO_ERROR)code );
                }
            }
            
        } else {
            
            PyObject* str_value = PyObject_Str( pvalue );
            PyObjCleanup str_value_clenaup(str_value);
            const char* exc = PyBytes_AsString(str_value);		    
            throw Exception ( SCRIPTS_SCRIPT, exc );
        }

        
    }
    PyObjCleanup ret_cleanup(ret);

    DataType dt(0);
    if (!nitro_to_datatype(ret,&dt)) {
        PyErr_Print();
        throw Exception ( SCRIPTS_ERR, "Unable to convert return value to DataType." );
    }
    return dt;
    
}

} // end namespace

