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

#ifndef NITRO_WRITER_H
#define NITRO_WRITER_H


#include <nitro/types.h>

namespace Nitro {


class DLL_API NodeRef;

/**
 * \ingroup devif
 * \brief Abstract class for writing device interface definition files.
 **/
class DLL_API Writer {
   public:
        /**
         * \brief Writer device interface contents. 
         **/
        virtual void write( const NodeRef& tree )=0;
        virtual ~Writer() {}
};


} // end namespace

#endif
