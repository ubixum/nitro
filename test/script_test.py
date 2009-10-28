import sys
import nitro

def MyFunction ( val ):
    """
        @param val int
    """
    return val * 3


def DevFunction ( dev, term, reg ):
    """
        @param dev device
        @param term int
        @param reg int
    """
    return dev.get(term,reg)

def TestFloat ( f ):
	return f/7.2;

def broken_func ( ):
    """
        Raises an exception is all
    """
    raise nitro.Exception ( 5, "Hello Exception" , {'test1':'hi'} )


def buffer_function(buf):
    """
        Tests reading and writing to a buffer (nitro.Buffer)
    """
    for i in range(len(buf)):
        buf[i] = buf[i]+1


def buffer_function2(dev,buf):
    dev.read(0,5,buf)



def bigint_test(dev):
    for int_tests in [ ( 0x12345678123456783ff, "wide_reg" ), 
                       ( 0x12d0865d5fe, "wide_reg" ),
                       ( 0x123456789, "wide_reg.big_sub3" ) ]:

        dev.set ( "int_term", int_tests[1], int_tests[0]  )
        r=dev.get ( "int_term", int_tests[1] )
        if r != int_tests[0]: 
            print "wide_reg", hex(r), "expect", hex(int_tests[0])
            raise nitro.Exception ( 1, "return value not correct.", int_tests[0] )

