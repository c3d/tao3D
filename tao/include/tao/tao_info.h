#ifndef TAO_INFO_H
#define TAO_INFO_H

#include "info.h"

namespace Tao {
  // Tao::Info pointers attached to the source tree are deleted
  // when the document is reloaded or the program exits.
  struct Info : XL::Info {};
}

#endif
