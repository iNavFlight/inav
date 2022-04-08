##
## This file is part of iNav.
##
## iNav is free software. You can redistribute this software
## and/or modify this software under the terms of the
## GNU General Public License as published by the Free Software
## Foundation, either version 3 of the License, or (at your option)
## any later version.
##
## iNav is distributed in the hope that they will be
## useful, but WITHOUT ANY WARRANTY; without even the implied
## warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
## See the GNU General Public License for more details.
##
## You should have received a copy of the GNU General Public License
## along with this software.
##
## If not, see <http://www.gnu.org/licenses/>.
##

class IntelHex

    class Section < Struct.new(:address, :io); end

    def initialize address = 0, bin_io = nil, start_address = nil
        @sections = Array.new
        @start_address = start_address == true ? address : start_address
        add_section address, bin_io if bin_io
    end

    def add_section address, io
        sections << Section.new(address, io)
    end

    def write io
        sections.each { |section| write_section section, io }
        write_start_linear_address start_address, io if start_address
        write_end_of_file io
        nil
    end

    attr_reader :sections
    attr_accessor :start_address

    private

    def calc_crc address, record_type, data
        sum = 0
        sum += (address & 0xFF00) >> 8
        sum += (address & 0x00FF)
        sum += record_type

        if data.is_a? Integer
            sum += data > 0xFFFF ? 4 : 2
            sum += (data & 0xFF000000) >> 24
            sum += (data & 0x00FF0000) >> 16
            sum += (data & 0x0000FF00) >> 8
            sum += (data & 0x000000FF)
        else
            sum += data.bytesize
            sum += data.each_byte.sum
        end

        ((sum ^ 0xFF) + 1) & 0xFF
    end

    def write_end_of_file io
        io.puts ":00000001FF"
    end

    def write_start_linear_address address, io
        crc = calc_crc 0, 5, address
        io.printf ":04000005%08X%02X\n", address, crc
    end

    def write_extended_linear_address address, io
        address_msw = (address & 0xffff0000) >> 16
        crc = calc_crc 0, 4, address_msw
        io.printf ":02000004%04X%02X\n", address_msw, crc
    end

    def write_data address, data, io
        address &= 0xFFFF
        hex_data = data.each_byte.map { |byte| "%02X" % byte }.join
        crc = calc_crc address, 0, data
        io.printf ":%02X%04X00%s%02X\n", data.bytesize, address, hex_data, crc
    end

    def write_section section, io
        previous_address = 0
        address = section.address
        while data = section.io.read(16)
            write_extended_linear_address address, io if (previous_address & 0xFFFF0000) != (address & 0xFFFF0000)
            previous_address = address
            write_data address, data, io
            address += data.bytesize
        end
    end

end
