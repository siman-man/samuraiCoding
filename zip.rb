require 'zip'
require 'fileutils'

zipfile_name = File.expand_path('../siman.zip', __FILE__)
input_filenames = ['sample8.cpp', 'compile.sh', 'run.sh', 'icon.png', 'leaning.txt', 'first_selection.txt']

FileUtils.rm_r zipfile_name if File.exist?(zipfile_name)

Zip::File.open(zipfile_name, Zip::File::CREATE) do |zipfile|
  input_filenames.each do |filename|
    # Two arguments:
    # - The name of the file as it will appear in the archive
    # - The original file, including the path to find it
    zipfile.add(filename, File.expand_path("../#{filename}", __FILE__))
  end
end
