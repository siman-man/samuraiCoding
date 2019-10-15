@filename='siman'

task :default do
  `g++ -W -Wall -Wno-sign-compare -O2 -o #{@filename} #{@filename}.cpp`
end

task :zip do
  `ruby zip.rb`
end

task :b do
  res = `ruby battle.rb`
  puts res
end

task :battle do
  res = `ruby battle.rb`
  puts res
end

task :simulate do
  system('ruby simulation.rb')
  system('ruby analyze.rb')
end
