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


#include "ihx.h"

#include <sstream>

#include <iostream>
#include <iomanip>
#include <iterator>

using namespace Nitro;

IhxFile parse_ihx ( const char* bytes, size_t length ) {

    IhxFile ihxfile;

    std::stringstream file;
    std::string file_bytes ( bytes, length );
    file.str( file_bytes );

    std::string line;
    uint32 cur_line=0;
    while ( file.good() ) {
      std::string line;
      ++cur_line;
      getline(file,line);
      if (!line.size()) continue;
      if (line.size()<11 || line.at(0) != ':') throw Exception ( USB_FIRMWARE );

      IhxRecord record;
      // record format
      // :<length><addr_msb><addr_lsb><type><data0><data1>...<dataN><sum>
      uint8 sum=0;
      for (std::string::iterator itr = line.begin() + 1; itr != line.end(); ++itr ) {
          std::stringstream converter; 
          converter << *itr; 
          ++itr;
          if (itr == line.end()) throw Exception ( USB_FIRMWARE , "Unexpected end of line", cur_line ); 
          converter << *itr;
          int byte; 
          converter >> std::setbase(16) >> byte;
          record.bytes.push_back((uint8)byte);
          sum += (uint8)byte;
      }
      if (sum) throw Exception ( USB_FIRMWARE, "Checksum", sum );

      if (!record.bytes.at(3)) {
          uint8 length = record.bytes.at(0);
          record.addr = (record.bytes.at(1) << 8) + record.bytes.at(2);
    
          record.bytes.erase(0,4);
          record.bytes.erase(record.bytes.end()-1); // the sum
          if (length != record.bytes.size()) throw Exception ( USB_FIRMWARE, "Invalid record length at line", cur_line );
          ihxfile.push_back(record);
      }

    }

    return ihxfile;

}
