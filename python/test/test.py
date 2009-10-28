
from gctest import *
import nitro


def test_node():
  n=nitro.DeviceInterface('di')
  n.test="hello world"
  n.test2="some other string"
  n.test3={'key':[10932,928342]}


