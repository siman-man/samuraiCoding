class Leaning
  TYPE  = 6
  TURN  = 9
  POINT = 6 # 0 - 5
  DIFF  = 10
  HIDE  = 12
  SCORE = 4 # 3 - 6

  def initialize

    puts TYPE * TURN * POINT * DIFF * HIDE * SCORE

    File.open('leaning.txt', 'w') do |file|
      TYPE.times do
        TURN.times do
          DIFF.times do
            POINT.times do
              SCORE.times do
                HIDE.times do
                  file.puts(rand(100))
                end
              end
            end
          end
        end
      end
    end
  end
end

Leaning.new
