require 'pp'

class AI
  attr_accessor :id, :score, :name, :selection, :middle_rank

  def initialize(ai, id, name)
    @ai = ai
    @name = name
    @middle_rank = -1
    @score = 0.0
    @lang_rank = Array.new(10){ Array.new(6) }
    @selection = Hash.new
    @id = id
    @ready = false
  end

  def ready?
    @ready if @ready
    @ready = "READY" == @ai.gets.chomp
  end

  def input(str)
    @ai.puts(str)
  end

  def response
    res = @ai.gets
    res
  end
end

class Battle
  def initialize(cflag)
    @participant = ['siman', 'sample8', 'sample9', 'sample6']
    @ais = []
    @hidden_point = Array.new(6, 0)
    @lang_points = Array.new(6){ Array.new(4, 0) }
    @real_points = Array.new(4){ Array.new(6,0) }
    @leader_board = Array.new(10){ Array.new(4, 0) }
    @top_rank = Array.new(10){ Array.new(6) }
    @worst_rank = Array.new(10){ Array.new(6) }
    @id_history = Array.new
    @values = Array.new

    @participant.each_with_index do |name, id|
      `g++ #{name}.cpp -O2 -o #{name}` if cflag
      @ais << AI.new(IO.popen("./#{name}", 'r+'), id, name)
    end

    File.open('leaning.txt', 'r') do |file|
      file.readlines.each_with_index do |line, index|
        line.chomp!
        @values[index] = line.to_i
      end
    end
  end

  def create_attention
    res = []
    6.times{ res << [3,4,5,6].sample }
    res
  end

  def show_current_state
    "#{@turn} #{holiday? ? 'H' : 'W'}"
  end

  def show_open_point(id)
    res = ""

    if @turn == 6
      @real_points.transpose.each do |point|
        res += point.join(' ')
        res += "\n"
      end
    else
      @lang_points.each do |point|
        res += point.rotate(id).join(' ')
        res += "\n"
      end
    end

    res.chomp
  end

  def show_real_point(id)
    @real_points[id].join(' ')
  end

  def show_hidden_point
    @hidden_point.join(' ')
  end

  def holiday?
    @turn % 2 == 0
  end

  def lastday?
    @turn == 5 || @turn == 9
  end

  def update(id, response)
    data = response.split(' ').map(&:to_i)

    #$stderr.puts "#{id}: #{response.inspect}"

    if holiday?
      selection = data.first(2)
      @id_list =  data.last(6) if id == 0
    else
      selection = data.first(5)
      @id_list =  data.last(6) if id == 0
    end

    #$stderr.puts @id_list.inspect

    if id == 0
      @id_history += @id_list
    end

    if holiday?
      selection.each do |i|
        @real_points[id][i] += 2
        @hidden_point[i] += 1
        @ais[id].selection[i] += 1
      end
    else
      selection.each do |i|
        @lang_points[i][id] += 1
        @real_points[id][i] += 1
        @ais[id].selection[i] += 1 
      end
    end
  end

  def calc_score
    @real_points.transpose.each_with_index do |point, index|
      best_score    = -1
      best_member   = []
      worst_score   = 100
      worst_member  = []

      point.each_with_index do |score, id|
        if best_score <= score
          if best_score == score
            best_member << id
          else
            best_member = [id]
          end

          best_score = score
        end

        if worst_score >= score
          if worst_score == score
            worst_member << id
          else
            worst_member = [id]
          end

          worst_score = score
        end
      end

      if lastday?
        best_member.each do |member|
          @ais[member].score += @attention_list[index] / best_member.size.to_f
          @leader_board[@turn][member] += @attention_list[index] / best_member.size.to_f
        end

        worst_member.each do |member|
          @ais[member].score -= @attention_list[index] / worst_member.size.to_f
          @leader_board[@turn][member] -= @attention_list[index] / worst_member.size.to_f
        end
      else 
        best_member.each do |member|
          @leader_board[@turn][member] += @attention_list[index] / best_member.size.to_f
        end

        worst_member.each do |member|
          @leader_board[@turn][member] -= @attention_list[index] / worst_member.size.to_f
        end
      end
    end

    if lastday?
      if @leader_board[@turn].max == @leader_board[@turn][0]
        @id_history.each do |idx|
          @values[idx] += (@turn == 5)? 5 : 10
        end
      elsif @leader_board[@turn].min == @leader_board[@turn][0]
        @id_history.each do |idx|
          @values[idx] -= (@turn == 5)? 5 : 10
        end
      elsif @leader_board[@turn][0] > 5
        @id_history.each do |idx|
          if @turn == 5
            @values[idx] += 2
          else
            @values[idx] += (@ais[0].middle_rank == 1)? 1 : 2
          end
        end
      elsif @leader_board[@turn][0] > 1
        @id_history.each do |idx|
          @values[idx] += (@turn == 5)? 1 : 2
        end
      elsif @leader_board[@turn][0] < -5
        @id_history.each do |idx|
          if @turn == 5
            @values[idx] -= 2
          else
            @values[idx] -= (@ais[0].middle_rank == 1)? 4 : 1
          end
        end
      elsif @leader_board[@turn][0] < 0
        @id_history.each do |idx|
          if @turn == 5
            @values[idx] -= 1
          else
            @values[idx] -= (@ais[0].middle_rank == 1)? 3 : 1
          end
        end
      end

      @id_history = Array.new
    elsif @turn > 1
      diff = @leader_board[@turn][0] - @leader_board[@turn-1][0]

      if diff <= -1
        @id_list.each_with_index do |idx, index|
          @values[idx] -= [1, @ais[0].selection[index]].max
          #$stderr.puts "update #{idx}: #{@values[idx]} to #{@values[idx] - [1,@ais[0].selection[index]].max}"
        end
      end

      if diff > 0
        @id_list.each do |idx|
          @values[idx] += 1
          #$stderr.puts "update #{idx}: #{@values[idx]} to #{@values[idx] + 1}"
        end
      end
    end

    #pp @leader_board
  end

  def show_result
    $stderr.puts "total: #{@attention_list.inject(:+)} - #{@attention_list.sort.join(' ')}"
    @ais.sort_by{|ai| -ai.score }.each.with_index(1) do |ai, rank|
      puts "name:#{ai.name}\tscore:#{ai.score}\tranking:#{rank}"
    end
  end

  def input_params(id)
    request = []

    request << show_current_state
    request << show_open_point(id)
    request << show_real_point(id)
    request << show_hidden_point unless holiday?

    request.join("\n")
  end

  def init
    @attention_list = create_attention

    @ais.each do |ai|
      ai.input('9 4 6')
      ai.input(@attention_list.join(' '))
    end
  end

  def ready?
    @ais.all?{|ai| ai.ready? }
  end

  def run
    init

    loop do
      break if ready?
      sleep(0.1)
    end

    1.upto(9) do |t|
      @turn = t

      if holiday?
        @hidden_point = Array.new(6, 0)
      end


      @ais.each do |ai|
        ai.input(input_params(ai.id))
        response = ai.response.chomp
        ai.selection = Hash.new(0)
        update(ai.id, response)
      end

      calc_score


      if @turn == 5
        @ais.sort_by{|ai| -ai.score }.each.with_index(1) do |ai, rank|
          ai.middle_rank = rank
        end
      end
      #pp @real_points
    end

    calc_score
    show_result

    File.open('leaning.txt', 'w') do |file|
      @values.each do |value|
        file.puts(value)
      end
    end
  end
end

Battle.new(ARGV.any?).run
