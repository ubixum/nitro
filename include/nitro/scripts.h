// Copyright (C) 2009 Ubixum, Inc. 
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

#ifndef NITRO_SCRIPTCONFIG_H
#define NITRO_SCRIPTCONFIG_H

#include <string>
#include <vector>
#include "types.h"



namespace Nitro {

/**
 * \defgroup scripts Scripting Interface
 *
 * Nitro provides scripting language support for devices.
 *
 * To run a script:
 *
 *  \li Instantiate a Scripts object.
 *  \li import the scripts file (Scripts::import)
 *  \li Optionally retrieve script meta data with Scripts::get_params, Scripts::get_comment.
 *  \li Add any parameters you want to call to pass to the scripted function.
 *  \li Execute the function (Scripts::exec)
 *  \li Optionally process the return type. 
 *
 *  Example: 
 *  \code
 *   ---- test.cpp ----
 *      Scripts s;
 *      s.import ( "test", "mytests.py" );
 *      USBDevice dev ( 0x1fe1, 0x7848 );
 *      NodeRef params = Node::create("params");
 *      params.set_attr("dev", dev);
 *      DataType ret = s.exec( "test", "mytest", params );
 *      if (ret == 1) { 
 *          printf ("It worked\n");
 *      }
 *
 *   ---- mytest.py ----
 *
 *      def mytest(dev):
 *          dev.set(0,5,1)
 *          dev.set(1,3,4)
 *          dev.get(0,1)
 *          return 1
 *  \endcode
 **/



/**
 *  \ingroup scripts
 *  \brief Run scripts on devices.
 *
 *
 **/
class DLL_API Scripts {
    private:
        struct impl;
        impl* m_impl;
    public:
        Scripts();
        ~Scripts() throw();

        /**
         * \brief Import script specified by script_path.
         * \param module A module name to associate loaded functions.  Loading
         *      a new script with the same module name as previous script will
         *      deallocate the old module functions.
         * \param script_path Fully qualified or relative path to the script
         *      file to import.
         **/
        void import(const std::string& module, const std::string& script_path);
        /**
         *  \brief Call function with specified arguments
         *  \param module The module prefix associated with the function.
         *  \param func_name Function name
         *  \param params Each argument will be converted to
         *      native type before passing to function.  The attributes are 
         *      the parameter names and values respectively. 
         **/
        DataType exec ( const std::string& module, const std::string& func_name, const NodeRef& params ) const;

        /**
         *  \brief Returns list of functions available in a module.
         *
         **/
        std::vector<std::string> func_list(const std::string& module) const ;

        /**
         * \brief Return the comment string for the specified function.
         * \param module The module prefix associated with the function.
         * \param func_name name of function.
         **/
        std::string get_comment( const std::string &module, const std::string &func_name ) const ;

        /**
         * \brief Return the argument types.
         * \return NodeRef.  The attribute names are the names of the parameters
         *          and the attribute values are the parameter types.
         **/
        NodeRef get_params ( const std::string& module, const std::string &func_name ) const;
};


} // end namespace

#endif



