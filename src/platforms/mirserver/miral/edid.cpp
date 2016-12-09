/*
 * Copyright © 2016 Canonical Ltd.
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
 *
 * Authored by: Nick Dedekind <nick.dedekind@canonical.com>
 */

#include "edid.h"

#include <sstream>

#define RETURN_ERROR(err) { error = err; return *this; }

miral::Edid& miral::Edid::parse_data(std::vector<uint8_t> const& data)
{
    if (data.size() != 128 && data.size() != 256) {
        RETURN_ERROR(Error::incorrect_size);
    }

    uint8_t sum = 0;
    int i, j;

    // check the checksum
    for (size_t i = 0; i < data.size(); i++) {
        sum += data[i];
    }
    if (sum) {
        RETURN_ERROR(Error::invalid_checksum);
    }

    // check header
    for (i = 0; i < 8; i++) {
        if (!(((i == 0 || i == 7) && data[i] == 0x00) || (data[i] == 0xff))) { //0x00 0xff 0xff 0xff 0xff 0xff 0x00
            RETURN_ERROR(Error::invalid_header);
        }
    }

    vendor = { (char)((data[8] >> 2 & 0x1f) + 'A' - 1),
               (char)((((data[8] & 0x3) << 3) | ((data[9] & 0xe0) >> 5)) + 'A' - 1),
               (char)((data[9] & 0x1f) + 'A' - 1) };

    product_code = *(uint16_t*)&data[10];
    serial_number = *(uint32_t*)&data[12];

    size.width = (int)data[21] * 10;
    size.height = (int)data[22] * 10;

    int descriptor_index;
    for (descriptor_index = 0, i = 54; descriptor_index < 4; descriptor_index++, i += 18) { //read through descriptor blocks... 54-71, 72-89, 90-107, 108-125
        if (data[i] == 0x00) { //not a timing descriptor

            auto& desctiptor = descriptors[descriptor_index];
            auto& value = desctiptor.value;

            desctiptor.type = static_cast<Edid::Descriptor::Type>(data[i+3]);

            switch (desctiptor.type) {
            case Descriptor::Type::monitor_name:
                for (j = 5; j < 18; j++) {
                    if (data[i+j] == 0x0a) {
                        break;
                    } else {
                        value.monitor_name[j-5] = data[i+j];
                    }
                }
                break;
            case Descriptor::Type::monitor_limits:
                break;
            case Descriptor::Type::unspecified_text:
                for (j = 5; j < 18; j++) {
                    if (data[i+j] == 0x0a) {
                        break;
                    } else {
                        value.unspecified_text[j-5] = data[i+j];
                    }
                }
                break;
            case Descriptor::Type::serial_number:
                for (j = 5; j < 18; j++) {
                    if (data[i+j] == 0x0a) {
                        break;
                    } else {
                        value.serial_number[j-5] = data[i+j];
                    }
                }
                break;
            default:
                break;
            }
        }
    }

    return *this;
}

std::string miral::Edid::Descriptor::string_value() const
{
    switch(type) {
    case Type::monitor_name:
        return std::string(&value.monitor_name[0]);
        break;
    case Type::unspecified_text:
        return std::string(&value.unspecified_text[0]);
        break;
    case Type::serial_number:
        return std::string(&value.serial_number[0]);
        break;
    default:
        return {};
    }
}
