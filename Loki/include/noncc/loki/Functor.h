///////////////////////////////
// Generated header: Functor.h
// Forwards to the appropriate code
// that works on the detected compiler
// Generated on Mon Sep 30 23:14:48 2002
///////////////////////////////

#ifdef LOKI_USE_REFERENCE
#	include "../../loki/Functor.h"
#else
#	if (__INTEL_COMPILER)
#		include "../../loki/Functor.h"
#	elif (__MWERKS__)
#		include "../../loki/Functor.h"
#	elif (__BORLANDC__ >= 0x560)
#		include "../Borland/Functor.h"
#	elif (_MSC_VER >= 1301)
#		include "../../loki/Functor.h"
#	elif (_MSC_VER >= 1300)
#		include "../MSVC/1300/Functor.h"
#	elif (_MSC_VER >= 1200)
#		include "../MSVC/1200/Functor.h"
#	else
#		include "../../loki/Functor.h"
#	endif
#endif
