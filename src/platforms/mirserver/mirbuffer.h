/*
 * Copyright © 2017 Canonical Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 3,
 * as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef QTMIR_MIR_BUFFER_H
#define QTMIR_MIR_BUFFER_H

#include <mir/geometry/size.h>

#include <memory>

namespace mir { namespace graphics { class Buffer; }}

namespace qtmir
{
class MirBuffer
{
public:
    MirBuffer();
    ~MirBuffer();
    explicit MirBuffer(std::shared_ptr<mir::graphics::Buffer> const &buffer);
    MirBuffer& operator=(std::shared_ptr<mir::graphics::Buffer> const &buffer);

    bool hasBuffer() const;
    bool hasAlphaChannel() const;
    mir::geometry::Size size() const;

    void reset();
    void glBindToTexture();

private:
    std::shared_ptr<mir::graphics::Buffer> m_mirBuffer;
};
}

#endif //QTMIR_MIR_BUFFER_H
