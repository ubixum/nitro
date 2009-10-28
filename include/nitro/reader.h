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

#ifndef NITRO_READER_H
#define NITRO_READER_H


#include <nitro/types.h>

namespace Nitro {


class DLL_API NodeRef;

/**
 * \ingroup devif
 * \brief Abstract class for reading device interface definition files.
 **/
class DLL_API Reader {
   public:
        /**
         *  Read reader contents into tree.  Normally a tree should
         *  be empty but it is possible to use the same tree to 
         *  load new nodes as long as the nodes have unique names and
         *  addresses.
         **/
        virtual void read( NodeRef tree )=0;
        virtual ~Reader(){}
};


} // end namespace

#endif
