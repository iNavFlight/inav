#!/usr/bin/env ruby
# coding: utf-8

##
## This file is part of INAV.
##
## INAV is free software. You can redistribute this software
## and/or modify this software under the terms of the
## GNU General Public License as published by the Free Software
## Foundation, either version 3 of the License, or (at your option)
## any later version.
##
## INAV is distributed in the hope that they will be
## useful, but WITHOUT ANY WARRANTY; without even the implied
## warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
## See the GNU General Public License for more details.
##
## You should have received a copy of the GNU General Public License
## along with this software.
##
## If not, see <http://www.gnu.org/licenses/>.
##

#########################################################################
#
# Generates a formatted (Markdown) report of CLI settings differences
# between two INAV branches or tags. Primarily intended for inclusion
# in release notes.
#
# Note that the report will require review / editing before inclusing
# in the RN, but it will jabe done the vast majority of the work for
# you.
#
# Example:
#
# draft_rn_settings.rb -f 5.1.0 -t release_6.0.0 > /tmp/draft-set6.0RN.md
# # now review  /tmp/draft-set6.0RN.md for inclusion in the offical RN
#
# Requirments:
# * ruby (v3 recommended)
# * git
#
#########################################################################

require 'yaml'
require 'optparse'
require 'tmpdir'

base = File.join(ENV["HOME"], "Projects/fc/inav")
ntag = nil
otag = nil

ARGV.options do |opt|
  opt.banner = "#{File.basename($0)} [options]"
  opt.on('-t','--tag=TAG',  'New tag or branch') {|o|ntag=o}
  opt.on('-f','--from-tag=TAG', 'previous release tag or branch'){|o|otag=o}
  opt.on('-b','--base=DIR',  "Base of the inav repo [#{base}]" ) {|o|base=o}
  opt.on('-?', "--help", "Show this message") {puts opt.to_s; exit}
  begin
    opt.parse!
  rescue
    puts opt
    exit
  end
end

abort 'New tag / branch is mandatory' unless ntag
abort 'Old tag / branch is mandatory' unless otag

begin
  Dir.chdir(base)
rescue
  abort "Cannot chdir to #{base}"
end

oldyaml = File.join(Dir.tmpdir, ".setting_old.yaml")
newyaml = File.join(Dir.tmpdir, ".setting_new.yaml")

system "> #{oldyaml} git show #{otag}:src/main/fc/settings.yaml"
system "> #{newyaml} git show #{ntag}:src/main/fc/settings.yaml"

h_old = {}
begin
  h_old = YAML.load_file(oldyaml)
rescue
  abort "Error reading old settings yaml"
end

h_new = {}
begin
  h_new = YAML.load_file(newyaml)
rescue
  abort "Error reading new settings yaml"
end

ot_names = []
h_old["tables"].each do |t|
  ot_names << t["name"]
end

nt_names = []
h_new["tables"].each do |t|
  nt_names << t["name"]
end

puts "# Draft Rel Notes -- settings helper"
puts
puts "The following should make a reasonable first pass at new / changed / removed settings"
puts "Will require manual checking / refining / formatting."

d = nt_names.difference(ot_names).sort
unless d.empty?
  puts
  puts "# New (Tables) -- to update group values"
  puts
  puts "| Name | Values |"
  puts "| ---- | ------ |"
  d.each do |dn|
    t = h_new["tables"].select{|k| k["name"] == dn}.first
    unless t.nil?
      vals = t["values"]
      if vals[0].class == String
        vals.map!{|a| "`#{a}`"}
      end
      vs = vals.join(", ")
      puts "| #{dn} | #{vs} |"
    end
  end
end
puts

d= ot_names.difference(nt_names).sort
unless d.empty?
  puts
  puts "## Removed (tables -- check for group value later)"
  puts
  puts "| Name | Comment |"
  puts "| ---- | ------ |"
  d.each do |dn|
    t = h_old["tables"].select{|k| k["name"] == dn}.first
    unless t.nil?
      puts "| #{dn} |  |"
    end
  end
end

d=ot_names.intersection(nt_names).sort
unless d.empty?
  puts
  puts "## Changed (table values -- check for group value later)"
  puts
  puts "| Name | Values |"
  puts "| ---- | ------ |"
  d.each do |dn|
    tnew = h_new["tables"].select{|k| k["name"] == dn}.first
    told = h_old["tables"].select{|k| k["name"] == dn}.first
    unless told.nil? && tnew.nil?
      if told["values"] != tnew["values"]
        vals = tnew["values"].difference(told["values"])
        if !vals.empty?
          if vals[0].class == String
            vals.map!{|a| "`#{a}`"}
          end
          newvals = vals.join(", ")
          puts "| #{dn} | New: #{newvals} |"
        end

        vals = told["values"].difference(tnew["values"])
        if !vals.empty?
          if vals[0].class == String
            vals.map!{|a| "`#{a}`"}
          end
          remvals = vals.join(", ")
          puts "| #{dn} | Removed: #{remvals} |"
        end
      end
    end
  end
end

og_names = []
h_old["groups"].each do |h|
  h["members"].each do |m|
    og_names << m["name"]
  end
end

ng_names = []
h_new["groups"].each do |h|
  h["members"].each do |m|
    ng_names << m["name"]
  end
end

d = ng_names.difference(og_names).sort
unless d.empty?
  puts
  puts "## New Items"
  puts
  puts "| Name | Description |"
  puts "| ---- | ------ |"
  d.each do |dn|
    t=nil
    h_new["groups"].each do |hh|
      t = hh["members"].select{|k| k["name"] == dn}.first
      unless t.nil?
        desc = t["description"]||""
        minmax = []
        if t.has_key?("min")
          case t["min"]
          when Integer
            minmax[0] = t["min"]
          when "INT16_MIN"
            minmax[0] = -32768
          when "INT8_MIN"
            minmax[0] = -128
          end
        end

        if t.has_key?("max")
          case t["max"]
          when Integer
            minmax[1] = t["max"]
          when "UINT16_MAX"
            minmax[1] = 65535
          when "INT16_MAX"
            minmax[1] = 32767
          when "INT8_MAX"
            minmax[1] = 127
          when "UINT8_MAX"
            minmax[1] = 255
          end
          minmax[0] = 0 if !minmax[1].nil? && minmax[0].nil?
        end
        if minmax.size == 2
          desc = [desc," Values: #{minmax[0]} - #{minmax[1]}"].join
        end

        default = nil
        if t.has_key?("default_value")
          default = t["default_value"].to_s.upcase
          unless default.nil?
            desc = "#{desc} Default: #{default}"
          end
        end
        desc.gsub!('|',',')
        # default_value max min
        puts "| #{dn} | #{desc} |"
        break
      end
    end
  end
end


d = og_names.difference(ng_names).sort
unless d.empty?
  puts
  puts "## Removed Items"
  puts
  puts "| Name | Description |"
  puts "| ---- | ------ |"
  d.each do |dn|
    puts "| #{dn} |  |"
  end
end


d=ot_names.intersection(nt_names).sort
unless d.empty?
  puts
  puts "## Changed Items (desc, range, default -- requires checking)"
  puts
  puts "Most of the content here will be indicated by a change at the table level"
  puts
  puts "| Name | Values |"
  puts "| ---- | ------ |"
  d.each do |dn|
    tnew = told = nil
    h_new["groups"].each do |hh|
      tnew = hh["members"].select{|k| k["name"] == dn}.first
      break unless tnew.nil?
    end
    h_old["groups"].each do |hh|
      told = hh["members"].select{|k| k["name"] == dn}.first
      break unless told.nil?
    end
    # in the following tests we discard just the addtion of an attribute
    unless (told.nil? && tnew.nil?)
      diffs = []
      if told["default_value"] && told["default_value"] != tnew["default_value"]
        diffs << "Default:  #{tnew["default_value"]}"
      end

      if told["description"] && told["description"] != tnew["description"]
        diffs << tnew["description"]
      end

      unless diffs.empty?
        diffstr = diffs.join(" ")
        puts "| #{dn} | #{diffstr} |"
      end

    end
  end
end

puts "Last updated: #{Time.now.utc.strftime("%FT%T%Z")}"
begin
  File.unlink(oldyaml, newyaml)
rescue
end
