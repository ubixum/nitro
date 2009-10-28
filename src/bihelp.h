
#ifdef WIN32
#pragma warning(push)
#pragma warning (disable:4800)
#include <gmpxx.h>
#pragma warning (pop)
#else
#include <gmpxx.h>
#endif

#include <nitro/types.h>

mpz_class bi_from_dt ( const Nitro::DataType &);
Nitro::DataType dt_from_bi ( const mpz_class &bi );
