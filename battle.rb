require 'pp'

class AI
  attr_accessor :id, :score, :name

  def initialize(ai, id, name)
    @ai = ai
    @name = name
    @score = 0.0
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
  def initialize
    @participant = ['siman', 'sample2', 'sample3', 'sample4']
    @ais = []
    @hidden_point = Array.new(6, 0)
    @lang_points = Array.new(6){ Array.new(4, 0) }
    @real_points = Array.new(4){ Array.new(6,0) }

    @participant.each_with_index do |name, id|
      `g++ #{name}.cpp -O2 -o #{name}`
      @ais << AI.new(IO.popen("./#{name}", 'r+'), id, name)
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
      @real_points.each do |point|
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

  def update(id, response)
    data = response.split(' ').map(&:to_i)

    if holiday?
      data.each do |i|
        @real_points[id][i] += 2
      end
    else
      data.each do |i|
        @lang_points[i][id] += 1
        @real_points[id][i] += 1
        @hidden_point[i] += 1
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

      best_member.each do |member|
        @ais[member].score += @attention_list[index] / best_member.size.to_f
      end

      worst_member.each do |member|
        @ais[member].score -= @attention_list[index] / worst_member.size.to_f
      end
    end
  end

  def show_result
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
        update(ai.id, response)
      end

      calc_score if @turn == 5
    end

    calc_score
    show_result
  end
end

Battle.new.run
