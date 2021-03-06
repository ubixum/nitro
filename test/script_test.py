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
    print "Buffer function called."
    for i in range(len(buf)):
        buf[i] = buf[i]+1
    print "Buffer function returning buf", buf


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

def get_subreg_test(dev):
    " was sigsegv on some platforms "
    dev.get_subregs ( "int_term" , "many_subregs" );

retry_count=0;
def callback(dev):
   """
        Suppose we want to execute a retry 10 times.
   """  
   def retry(d, term, reg, count, exc):
        global retry_count
        if count < 10:
            retry_count += 1
            return True
        return False

   dev.set_retry_func ( retry )
   try:
       try:
           dev.get(1,1)
       except Exception, _inst:
           if retry_count == 10: 
               return
   finally:
       dev.set_retry_func(None)

   # hm, must not have worked
   print "Retry Count: ", retry_count
   raise nitro.Exception ( 1, "Code didn't retry 10 times.", retry_count )

    
