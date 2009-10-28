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

#include "bihelp.h"

#include <cassert>
#include <vector>

#include <nitro/error.h>

using namespace Nitro;

mpz_class bi_from_dt ( const DataType &data ) {
   if ( data.get_type() == INT_DATA ) return mpz_class ( (int32) data ); 
   if ( data.get_type() == UINT_DATA ) return mpz_class ( (uint32) data ); 
   if ( data.get_type() == STR_DATA ) return mpz_class ( (std::string) data ); 
   if ( data.get_type() == BIGINT_DATA ) {
      mpz_class res(0); 
      std::vector<DataType> ints = DataType::as_bigints ( data ); 
      while (ints.size()) {
        res <<= 32;
        res |= mpz_class ( (uint32) ints.back() );
        ints.pop_back();
      }
      return res;
   }
   throw Exception ( INVALID_CAST , "Unsupported cast to bigint from datatype." , data );
}

DataType dt_from_bi ( const mpz_class &bi ) {
    if (bi.fits_sint_p()) {
        long si = bi.get_si();
        assert ( sizeof(si) == 8 ? ((uint64)si & 0xffffffff00000000ul) == 0 : true);
        return (int32) si;
    } else if ( bi.fits_uint_p()) {
        unsigned long ui = bi.get_ui();
        assert ( sizeof(ui) == 8 ? (ui & 0xffffffff00000000ul) == 0 : true );
        return (uint32) ui;
    } else {
        if ( sgn ( bi ) < 0) throw Exception ( INVALID_TYPE, "Unsupported negative large integer.", bi.get_str() );
        
        mpz_class cpy = bi;
        std::vector<DataType> ints;
        while ( sgn ( cpy ) ) {
            mpz_class lsi = cpy & mpz_class(0xfffffffful);
            assert ( lsi.fits_uint_p() );
            ints.push_back( (uint32) lsi.get_ui() );
            cpy >>= 32; 
        }

        return DataType::as_bigint_datatype ( ints, bi.get_str() );
    }
}

