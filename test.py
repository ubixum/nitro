
from nitro_parts.BrooksEE.BR9728_DB5 import get_dev

dev=get_dev()
import numpy

for i in range(18,27,1): 
    print "read 2**%d" % i
    z=numpy.zeros(2**i,dtype=numpy.uint8)
    dev.read("DRAM", 0, z, 5000)
    print "read 2**%d ok" % i
