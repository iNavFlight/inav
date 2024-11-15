#!/usr/bin/env ruby

# This script creates a list of **merged** PRs for a release

# gh --repo inavflight/inav pr list --state merged -S "milestone:8.0" -L 500 --json "number,title,author,url" > /tmp/gh.json
# Then process the output `/tmp/gh.json` with this script
# ./generate-prlist-rn.rb /tmp/gh.json > /tmp/rel8prs.md
#
# Then merge the contents of `/tmp/rel8prs.md` into the release notes

require 'json'
abort("Need the JSON file") unless ARGV[0]
text = File.read(ARGV[0])
jsa = JSON.parse(text)
jsa.each do |js|
  puts "* #%d %s by @%s\n" % [js['number'],js['title'],js['author']['login']]
end
