@filename='tutorial'

task :default do
  `g++ -W -Wall -Wno-sign-compare -O2 #{@filename}.cpp`
end

task :zip do
  `ruby zip.rb`
end

task :battle do
  res = `ruby battle.rb`
  puts res
end

