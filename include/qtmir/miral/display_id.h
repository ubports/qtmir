#ifndef MIRAL_DISPLAY_H
#define MIRAL_DISPLAY_H

#include <mir/int_wrapper.h>

#include "edid.h"

namespace mir { namespace graphics { namespace detail { struct GraphicsConfOutputIdTag; } } }

// Prototyping namespace for later incorporation in MirAL
namespace miral
{
using OutputId = mir::IntWrapper<mir::graphics::detail::GraphicsConfOutputIdTag>;

struct DisplayId
{
    Edid edid;
    OutputId output_id; // helps to identify a monitor if we have two of the same.
};

} // namespace miral


#endif // MIRAL_DISPLAY_H
