player = Hash.new{|h,k| h[k] = Hash.new(0)}

line_num = 0

File.open('result.txt', 'r') do |file|
  file.readlines.each do |line|
    res = line.chomp.split("\t").map{|e| e.split(':')}.to_h
    name = res['name']
    score = res['score'].to_f
    ranking = res['ranking'].to_i

    player[name]['score'] += score
    player[name]['ranking'] += ranking

    line_num += 1
  end
end

line_num = line_num / 4.0

player.each do |name, data|
  puts "#{name}:\tAvarage Score - #{data['score']/line_num},\tAvarage Ranking - #{data['ranking']/line_num}"
end
