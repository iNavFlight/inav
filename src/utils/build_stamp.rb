#!/usr/bin/env ruby

# This file is part of INAV.
#
# author: Alberto Garcia Hierro <alberto@garciahierro.com>
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this file,
# You can obtain one at http://mozilla.org/MPL/2.0/.
#
# Alternatively, the contents of this file may be used under the terms
# of the GNU General Public License Version 3, as described below:
#
# This file is free software: you may copy, redistribute and/or modify
# it under the terms of the GNU General Public License as published by the
# Free Software Foundation, either version 3 of the License, or (at your
# option) any later version.
#
# This file is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
# Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program. If not, see http://www.gnu.org/licenses/.

require 'fileutils'
require 'stringio'

require_relative 'compiler'

class Stamper
    def initialize(settings_file, stamp_file)
        @settings_file = settings_file
        @stamp_file = stamp_file
        @stamp_dir = File.dirname(@stamp_file)
        @compiler = Compiler.new
    end

    def stamp
        buf = StringIO.new
        buf << "CFLAGS = " << ENV["CFLAGS"] << "\n"
        buf << "TARGET = " << ENV["TARGET"] << "\n"
        buf << "CONFIG = " << hash_config() << "\n"

        stamp = buf.string

        old_stamp = nil
        if File.file?(@stamp_file)
            old_stamp = File.open(@stamp_file, 'rb') { |f| f.read }
        end
        if old_stamp != stamp
            File.open(@stamp_file, 'w') {|f| f.write(stamp)}
        end
    end

    def hash_config
        buf = StringIO.new
        ["target.h", "platform.h", "cstddef"].each do |h|
            buf << "#include \"#{h}\"\n"
        end
        FileUtils.mkdir_p @stamp_dir
        input = File.join(@stamp_dir, "stamp.cpp")
        File.open(input, 'w') {|file| file.write(buf.string)}
        output = File.join(@stamp_dir, "stamp")
        stdout, stderr = @compiler.run(input, output, ["-dM", "-E"])
        File.delete(input)
        if File.file?(output)
            File.delete(output)
        end
        return Digest::SHA1.hexdigest(stdout)
    end
end

def usage
    puts "Usage: ruby #{__FILE__} <settings_file> <stamp_file>"
end

if __FILE__ == $0

    settings_file = ARGV[0]
    stamp_file = ARGV[1]
    if settings_file.nil? || stamp_file.nil?
        usage()
        exit(1)
    end

    stamper = Stamper.new(settings_file, stamp_file)
    stamper.stamp()
end