//
// Mini Template Library
//

#include <cstring>

#pragma warning(push)
#pragma warning(disable : 4005)

#define MTL_namespace mtl
#define MTL_memcpy memcpy
#define MTL_memset memset
#define MTL_strlen(s) static_cast<size_t>(strlen(s))
#define MTL_new(type) new type
#define MTL_new_array(type, n) new type [n]
#define MTL_delete(p) delete p
#define MTL_delete_array(p) delete[] p

#pragma warning(pop)

#include "string.inl"
