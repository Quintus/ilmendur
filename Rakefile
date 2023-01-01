# coding: utf-8
require "rake/clean"

CLEAN.include("*-prefix", "build", "build-dir", "repo")
CLOBBER.include("ilmendur.flatpak")

def run(cmd)
  puts(cmd)
  system(cmd) || fail("Command failed with status #{$?.exitstatus}")
end

# Generate one file download task per dependency listed in cmake/dependencies.cmake
depstr = File.read("cmake/dependencies.cmake")
deps = {}
while md = depstr.match(/ExternalProject_Add\((.*?)\n.*?URL "(.*?)"\n/m)
  deps[md[1] + "-prefix/src/" + File.basename(md[2])] = md[2]
  depstr = md.post_match
end
deps.each_pair do |filename, url|
  file(filename) do
    mkdir_p File.dirname(filename)
    run "curl -o '#{filename}' -L '#{url}'"
  end
end

desc "Build ilmendur.flatpak Flatpak bundle"
file "ilmendur.flatpak" => FileList["src/**/*.cpp", "src/**/*.hpp", "eu.guelker.ilmendur.yml"] + deps.keys do |t|
  run "flatpak-builder --repo=repo --gpg-sign=E22572789229F6266BBF327C58932C3C6385CD3B --force-clean build-dir eu.guelker.ilmendur.yml"
  run "flatpak build-bundle repo '#{t.name}' eu.guelker.ilmendur"
end

namespace "i18n" do

  desc "Update data/translations/Ilmendur.pot from the C++ source files."
  task :potfile do
    sources = FileList["src/**/*.cpp"]
    run "xgettext -o data/translations/Ilmendur.pot \
--add-comments='TRANS:' \
--language=C++ \
--escape \
--from-code=UTF-8 \
--keyword=_ \
--keyword=PL_:1,2 \
--keyword=C_:1c,2 \
--copyright-holder='Marvin Gülker and the Ilmendur team' \
--package-name='Ilmendur' \
#{sources.join(' ')}"
  end

  rule ".po" => "data/translations/Ilmendur.pot" do |t|
    target = "data/translations/#{t.name.split(":")[1]}"
    if File.file?("data/translations/#{t.source}")
      run "msgmerge -U #{t.prereqs.first} #{target}"
    else
      cp t.prereqs.first, target
    end
  end

end

task :default do
  fail "This Rakefile is only for project management, not for compilation of source code. Use cmake for that."
end
