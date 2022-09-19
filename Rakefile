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

task :default do
  fail "This Rakefile is only for project management, not for compilation of source code. Use cmake for that."
end
