File.open('result.txt', 'w') do |file|
  200.times do |i|
    puts "#{i+1} times"
    res = `ruby battle.rb`
    file.puts res.chomp
  end
end
