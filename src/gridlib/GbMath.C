#ifdef USE_RCSID
static const char RCSid_GbMath[] = "$Id: GbMath.C,v 1.3 2003/03/06 17:01:52 prkipfer Exp $";
#endif

/*----------------------------------------------------------------------
|
|
| $Log: GbMath.C,v $
| Revision 1.3  2003/03/06 17:01:52  prkipfer
| changed default template instantiation target to be GNU flavour
|
| Revision 1.2  2002/12/13 09:32:21  prkipfer
| changed memory pool scoping to work around Visual C++ bug
|
| Revision 1.1  2001/01/02 14:51:24  prkipfer
| introduced new classes
|
|
|
+---------------------------------------------------------------------*/

#ifdef OUTLINE

#include "GbMath.hh"
#include "GbMath.in"
//#include "GbMath.T"

template class GRIDLIB_API GbMath<float>;
template class GRIDLIB_API GbMath<double>;

#endif 
