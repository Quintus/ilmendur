# coding: utf-8
require "rake/clean"
require "rexml/document"
require "rexml/xpath"

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

  # Parse all to-be translated strings out of the TMX map files.
  file "src/external_strings.cpp" => FileList["data/maps/*.tmx"] do |t|
    puts "Generating src/external_strings.cpp from TMX files"
    extstrings = []
    t.prereqs.sort.each do |tmxfile|
      File.open(tmxfile, "r") do |f|
        doc = REXML::Document.new(f)
        REXML::XPath.each(doc, "//property[@name='text']") do |propnode|
          extstrings << propnode["value"]
        end
      end
    end

    File.open(t.name, "w") do |file|
      file.puts(<<EOF)
/* external_strings.cpp is automatically generated. Do not edit this file.
 * It exists solely for the purpose of Gettext's xgettext(1) command to
 * pick them up for translation purposes. Run rake i18n:potfile to update
 * this file. */
#if 0
EOF

      # Note: Do not sort `extstrings' so that the translator can infer some
      # context for each string.
      extstrings.each_with_index do |extstr, i|
        file.puts "static const char* exttrans#{i+1} = _(\"#{extstr}\");"
      end

      file.puts("#endif")
    end
  end

  desc "Update data/translations/Ilmendur.pot from the C++ source files."
  task :potfile => "src/external_strings.cpp" do
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
    if File.file?(target)
      run "msgmerge -U #{target} #{t.prereqs.first}"
    else
      cp t.prereqs.first, target
    end
  end

  desc "Check all PO translation files for valid syntax."
  task :checkpo do
    FileList["data/translations/*.po"].sort.each do |pofile|
      run "msgfmt -o /dev/null -c --statistics #{pofile}"
    end
  end

end

task :default do
  fail "This Rakefile is only for project management, not for compilation of source code. Use cmake for that."
end
