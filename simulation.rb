File.open('result.txt', 'w') do |file|
  100.times do |i|
    puts "#{i+1} times"
    if i.zero?
      res = `ruby battle.rb COMPILE`
    else
      res = `ruby battle.rb`
    end
    file.puts res.chomp
  end
end
