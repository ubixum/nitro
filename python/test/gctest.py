import gc
# Recursively expand slist's objects
# into olist, using seen to track
# already processed objects.
def _getr(slist, olist, seen):
  for e in slist:
      if id(e) in seen:
          continue
      seen[id(e)] = None
      olist.append(e)
      tl = gc.get_referents(e)
      if tl:
          _getr(tl, olist, seen)
# The public function.
def get_all_objects():
  """Return a list of all live Python objects, not including the
list itself."""
  gc.collect()
  gcl = gc.get_objects()
  olist = []
  seen = {}
  # Just in case:
  seen[id(gcl)] = None
  seen[id(olist)] = None
  seen[id(seen)] = None
  # _getr does the real work.
  _getr(gcl, olist, seen)
  return olist


def test(f, repeat=1):
 o1=len(get_all_objects())
 for i in range(repeat): f()
 del i
 o2=len(get_all_objects())
 # r2 includes 1 ref for r1
 if o1 != o2-1:
    print "possible leak in %s, %s to %s" % ( f, o1, o2 )    


