#pragma once

#include <memory>

#define Ref std::shared_ptr
#define RefMake std::make_shared
#define WeakRef std::weak_ptr
#define RefCast std::dynamic_pointer_cast
#define RefSwap std::swap