scores = Array.new(3, 0)

File.open('test.txt', 'r') do |file|
  file.readlines.each_with_index do |line, index|
    line.chomp!

    scores[index] = line.to_i
  end
end

File.open('test.txt', 'w') do |file|
  scores.each do |s|
    file.puts(s + 1)
  end
end
