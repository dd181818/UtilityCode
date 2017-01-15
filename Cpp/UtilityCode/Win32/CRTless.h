//
// Collection of code that does not make
// use of the MSVC runtime, hence "CRTless".
//

#pragma warning(push)
#pragma warning(disable : 4577)
#include <cstdarg>
#include <type_traits>
#include <utility>
#include <Windows.h>
#pragma warning(pop)

#include "CRTless_runtime.h"

