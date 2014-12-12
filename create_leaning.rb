class Leaning
  TOP   = 2
  TDIFF = 7
  LDIFF = 7
  DIVI  = 7
  TURN  = 9
  POINT = 6 # 0 - 5
  HIDE  = 6
  SCORE = 4 # 3 - 6

  def initialize

    puts TOP * TDIFF * DIVI * HIDE * TURN * SCORE * POINT * LDIFF

    return

    File.open('leaning.txt', 'w') do |file|
      TOP.times do
        DIVI.times do
          HIDE.times do
            TDIFF.times do
              LDIFF.times do
                TURN.times do
                  SCORE.times do
                    POINT.times do
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
