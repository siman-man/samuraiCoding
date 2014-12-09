class Leaning
  TOP   = 2
  DIVI  = 7
  TYPE  = 6
  TURN  = 8
  POINT = 6 # 0 - 5
  DIFF  = 8
  HIDE  = 7
  SCORE = 4 # 3 - 6

  def initialize

    puts DIFF * DIVI * HIDE * TURN * TYPE * SCORE * POINT * TOP

    File.open('leaning.txt', 'w') do |file|
      DIVI.times do
        HIDE.times do
          DIFF.times do
            TURN.times do
              TYPE.times do
                SCORE.times do
                  POINT.times do
                    TOP.times do
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
  end
end

Leaning.new
