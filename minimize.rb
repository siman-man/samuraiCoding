list = []
File.open('mini.txt','w') do |file|
  File.open('leaning.txt','r') do |file2|
    file2.readlines.each do |line|
      line.chomp!
      list << line
    end
  end

  file.puts(list.join(' '))
end
