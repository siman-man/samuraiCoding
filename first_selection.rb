hash = Hash.new(0)

[3,4,5,6].repeated_combination(6).each do |list|
  puts list.reverse.join(' ')
  puts "1 1 1 1 1 0"
end
