#include <iostream>
#include <fstream>
#include <vector>
#include <map>
#include <algorithm>
#include <limits.h>
#include <string>
#include <string.h>
#include <sstream>
#include <set>
#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <stack>
#include <queue>

using namespace std;

typedef long long ll;
const int PLAYER_COUNT  = 4;
const int LANG_COUNT    = 6;
const int TURN_LIMIT    = 9;
const int myId         = 0;
const double BAN = -100000000.0;
const double BIGBAN = -1000000000.0;
int T, P, N;
string D;
int turn = 0;
int fsId = 1;
int card_list[6] = { 0, 1, 2, 3, 4, 5 };
map<int, vector<int> > firstSelection;

//======================================================================================//
//     next_combination
//======================================================================================//
  template <class BidirectionalIterator>
bool next_combination(BidirectionalIterator first1,
    BidirectionalIterator last1,
    BidirectionalIterator first2,
    BidirectionalIterator last2)
{
  if (( first1 == last1 ) || ( first2 == last2 )) {
    return false ;
  }
  BidirectionalIterator m1 = last1 ;
  BidirectionalIterator m2 = last2 ; --m2;
  while (--m1 != first1 && !(* m1 < *m2 )){
  }
  bool result = (m1 == first1 ) && !(* first1 < *m2 );
  if (! result ) {
    while ( first2 != m2 && !(* m1 < * first2 )) {
      ++ first2 ;
    }
    first1 = m1;
    std::iter_swap (first1 , first2 );
    ++first1;
    ++first2;
  }
  if (( first1 != last1 ) && ( first2 != last2 )) {
    m1 = last1; m2 = first2;
    while (( m1 != first1 ) && (m2 != last2 )) {
      std::iter_swap(--m1, m2 );
      ++m2;
    }
    std::reverse (first1, m1);
    std::reverse (first1, last1);
    std::reverse (m2, last2);
    std::reverse (first2, last2);
  }
  return !result;
}

  template <class BidirectionalIterator>
bool next_combination (BidirectionalIterator first,
    BidirectionalIterator middle,
    BidirectionalIterator last)
{
  return next_combination (first ,middle ,middle ,last);
}

template <class BidirectionalIterator>
  inline
bool prev_combination ( BidirectionalIterator first,
    BidirectionalIterator middle,
    BidirectionalIterator last)
{
  return next_combination (middle ,last ,first ,middle);
}

template<class TYPE> class RepeatedCombinationGenerator{
  private:
    std::vector<TYPE> data_;
    unsigned int select_;

  public:
    template<class InputIterator>
      RepeatedCombinationGenerator(InputIterator begin, InputIterator end, const unsigned int n){
        select_ = n;
        // 重複する値を取り除く
        std::set<TYPE> s;
        for(InputIterator iter=begin; iter!=end; ++iter){
          s.insert(*iter);
        }
        // 選択可能な値をn個ずつ持つ配列を作成
        data_.clear();
        for(typename std::set<TYPE>::iterator iter=s.begin(); iter!=s.end(); ++iter){
          for(unsigned int i=0; i<n; ++i){
            data_.push_back(*iter);
          }
        }
      }
    // 最初の組み合わせの生成
    void fast(){
      std::sort(data_.begin(), data_.end());
    }
    // 最後の組み合わせの生成
    void last(){
      std::sort(data_.begin(), data_.end());
      std::vector<TYPE> new_data;
      std::back_insert_iterator<std::vector<TYPE> > iter(new_data);
      std::copy(data_.end()-select_, data_.end(), iter);
      std::copy(data_.begin(), data_.end()-select_, iter);
      data_ = new_data;
    }
    // 次の組み合わせの生成
    bool next(){
      return next_combination(data_.begin(), data_.begin()+select_, data_.end());
    }
    // 前の組み合わせの生成
    bool prev(){
      return prev_combination(data_.begin(), data_.begin()+select_, data_.end());
    }
    // 現在の組み合わせを取得
    std::vector<TYPE> data() const{
      return std::vector<TYPE>(data_.begin(), data_.begin()+select_);
    }
    // 全ての重複組み合わせの数
    unsigned int size() const{
      unsigned int n = data_.size() / select_;
      unsigned int r = select_;
      return factorial(n+r-1, n-1)/factorial(r);
    }

  private:
    // 階乗
    unsigned int factorial(const unsigned int max, const unsigned int min = 1) const{
      unsigned int result = 1;
      for(unsigned int i=min+1; i<=max; ++i){
        result *= i;
      }
      return result;
    }
};

class Player{
  public:
    int id;
    double score;
};

int totalHiddenCount = 0;
int gameTotalPoint = 0;
double expectPoint[4][6];
int myHiddenSelect[LANG_COUNT];
int scoreTypeCount[7];
int highPointCount = 0;

bool isLastDay(){
  return turn == 9 || turn == 5;
}

typedef pair<int,int> cardScore;

class Lang{
  public:
    int     id;
    int     ranking;
    int     point[PLAYER_COUNT];
    int     topPoint;
    int     lastPoint;
    int     playerRank[PLAYER_COUNT];
    int     pointThisTurn[PLAYER_COUNT];
    int     hiddenCount;
    double  attention;
    int  originalPoint;
    double  popularity;

    Lang(){
      ranking = -1;
      hiddenCount = 0;
      popularity = 0.0;
    }

    void update(){
      memset(playerRank, 0, sizeof(playerRank));
      memset(pointThisTurn, 0, sizeof(pointThisTurn));
      topPoint = 0;
      lastPoint = 100;

      for(int i = 1; i <= PLAYER_COUNT; i++){
        int bestPoint = -1;
        int playerId = -1;

        for(int j = 0; j < PLAYER_COUNT; j++){
          if(playerRank[j] == 0 && bestPoint < point[j]){
            bestPoint = point[j];
            playerId = j;
          }
        }

        playerRank[playerId] = i;
        topPoint = max(topPoint, bestPoint);
        lastPoint = min(lastPoint, bestPoint);
      }
    }

    void showRanking(){
      int bestScore      = -1;
      int secondBest     = -1;
      int secondPlayer   = -1;
      int bestPlayer     = -1;
      int bestSameCount  = 1;
      int worstScore     = 100;
      int worstPlayer    = -1;
      int secondWorst    = -1;
      int worstSameCount = 1;

      for(int i = 0; i < PLAYER_COUNT; i++){
        if(bestScore <= point[i]){
          if(bestScore == point[i]){
            bestSameCount++;
          }else{
            secondPlayer = bestPlayer;
            bestPlayer = i;
            bestSameCount = 1;
          }
          secondBest = bestScore;
          bestScore = point[i];
        }else if(secondBest < point[i]){
          secondBest = point[i];
          secondPlayer = i;
        }

        if(worstScore >= point[i]){
          if(worstScore == point[i]){
            worstSameCount++;
          }else{
            worstPlayer = i;
            worstSameCount = 1;
          }
          secondWorst = worstScore;
          worstScore = point[i];
        }else if(secondWorst > point[i]){
          secondWorst = point[i];
        }
      }

      //fprintf(stderr, "card%d: top: %d, score: %d, second: %d, score: %d, sumPoint = %d\n", id, bestPlayer, bestScore, secondPlayer, secondBest, enemyTotalPoint());
    }

    double interest(int id){
      return (double)point[id] / enemyTotalPoint();
    }

    // 相手のポイントの合計値を求める
    int enemyTotalPoint(){
      int openPoint = 0;

      for(int i = 1; i < PLAYER_COUNT; i++){
        openPoint += point[i];
      }

      return openPoint + 2 * hiddenCount;
    }

    double getRatio(){
      if(popularity >= 0.5){
        return 0.1;
      }else if(popularity >= 0.4){
        return 0.2;
      }else if(popularity >= 0.3){
        return 0.3;
      }else if(popularity >= 0.2){
        return 0.6;
      }else if(popularity >= 0.1){
        return 1.2;
      }else{
        return 2.0;
      }
    }

    /*
     * 1位の時の評価値
     * 低いほど悪い
     */
    double fuzzyScore(int diff){
      int sumPoint = enemyTotalPoint();

      /*
       * 2位との差が5pt以上ある FSD5
       */
      if(diff >= 5){
        // 4ターン目の隠しポイント
        if(turn == 4){
          // 4pt割り振った場合
          if(pointThisTurn[myId] == 4){
            if(originalPoint >= 5){
              if(hiddenCount == 0){
                return BAN;
              }else{
                return (hiddenCount <= 1)? -1.0 : 1.0;
              }
            }else{
              return -10.0;
            }
            // 2pt以上割り振った場合
          }else if(pointThisTurn[myId] == 2){
            // 高スコア
            if(originalPoint >= 5){
              return (hiddenCount <= 2)? -1.0 : 1.5;
              // 低スコア
            }else{
              if(hiddenCount == 0){
                if(highPointCount >= 3){
                  return BAN;
                }else{
                  return -100.0;
                }
              }else{
                return (hiddenCount <= 1)? -10.0 : 0.5;
              }
            }
            // 1ptも割り振らない場合 - 5pt
          }else{
            // 高スコア
            if(originalPoint >= 5){
              return (hiddenCount <= 1)? 5.0 : 2.0;
              // 低スコア
            }else{
              return (hiddenCount <= 1)? 10.0 : 1.0;
            }
          }
          // 7ターン目
        }else if(turn == 7){
          // 4pt以上振っている
          if(pointThisTurn[myId] >= 4){
            // 高いポイント
            if(originalPoint >= 5){
              // 元々のランキングが1位ではない
              if(playerRank[myId] != 1){
                return (hiddenCount <= 1)? -100.0 : 2.0;
              }else{
                // 高いポイントが多い
                if(highPointCount >= 4){
                  return (hiddenCount <= 1)? -200.0 : -400.0;
                }else{
                  if(turn == 3 && sumPoint >= 10){
                    return -10.0;
                  }else{
                    return (hiddenCount <= 1)? -200.0 : -400.0;
                  }
                }
              }
              // low point
            }else{
              return (hiddenCount <= 1)? -20.0 : -1.0;
            }
            // 3pt以上振っている
          }else if(pointThisTurn[myId] >= 3){
            // 高いポイント
            if(originalPoint >= 5){
              // 元々のランキングが1位ではない
              if(playerRank[myId] != 1){
                return (hiddenCount <= 1)? -100.0 : 2.0;
              }else{
                // 高いポイントが多い
                if(highPointCount >= 4){
                  return (hiddenCount <= 1)? -200.0 : -400.0;
                }else{
                  if(turn == 3 && sumPoint >= 10){
                    return -10.0;
                  }else{
                    return (hiddenCount <= 1)? -200.0 : -400.0;
                  }
                }
              }
            }else{
              return (hiddenCount <= 1)? -20.0 : -2.0;
            }
            // 1pt以上振っている
          }else if(pointThisTurn[myId] >= 1){
            // 高ポイント
            if(originalPoint >= 5){
              return (hiddenCount <= 1)? -1000.0 : 2.0;
              // 低ポイント
            }else{
              return (hiddenCount <= 2)? -1.0 : 2.0;
            }
            // 1ptも降っていない - 3T-7T-5PT
          }else{
            // 高ポイント
            if(originalPoint >= 5){
              return (hiddenCount <= 1)? 20.0 : -2.0;
              // 低ポイント
            }else{
              return (hiddenCount <= 2)? -0.5 : -1.0;
            }
          }
          // 3ターン目 - 5pt
        }else if(turn == 3){
          // 3pt以上振っている
          if(pointThisTurn[myId] >= 3){
            // 高いポイント
            if(originalPoint >= 5){
              // 元々のランキングが1位ではない
              if(playerRank[myId] != 1){
                return (hiddenCount <= 1)? -100.0 : 2.0;
              }else{
                // 高いポイントが多い
                if(highPointCount >= 4){
                  return (hiddenCount <= 1)? BAN : -200.0;
                }else{
                  if(turn == 3 && sumPoint >= 10){
                    return -10.0;
                  }else{
                    return (hiddenCount <= 1)? -200.0 : -400.0;
                  }
                }
              }
            }else{
              return (hiddenCount <= 2)? -1.0 : -2.0;
            }
            // 1pt以上振っている
          }else if(pointThisTurn[myId] >= 1){
            // 高ポイント
            if(originalPoint >= 5){
              return (hiddenCount <= 1)? 2.0 : -100.0;
              // 低ポイント
            }else{
              return (hiddenCount <= 2)? 1.0 : -200.0;
            }
            // 1ptも降っていない - 3T-7T-5PT
          }else{
            // 高ポイント
            if(originalPoint >= 5){
              return (hiddenCount <= 1)? 20.0 : -2.0;
              // 低ポイント
            }else{
              return (hiddenCount <= 2)? -0.5 : -1.0;
            }
          }
          // 8T 5pt
        }else if(turn == 8){
          // 高ポイント
          if(originalPoint >= 5){
            // 4pt割り振っている
            if(pointThisTurn[myId] == 4){
              return (hiddenCount <= 1 || diff >= 7)? BAN : 1.5;
              // 2pt割り振っている
            }else if(pointThisTurn[myId] == 2){
              return (hiddenCount <= 1 || diff >= 7)? BAN : 1.5;
              // 1ptも割り振っていない
            }else{
              return (hiddenCount <= 1)? 30.0 : 1.0;
            }
            // 低ポイント
          }else{
            // 4pt割り振っている
            if(pointThisTurn[myId] == 4){
              return (hiddenCount <= 1)? BAN : -20.0;
              // 1pt以上割り振っている
            }else if(pointThisTurn[myId] >= 1){
              return (hiddenCount <= 1)? BAN : -10.0;
              // 1ptも割り振っていない
            }else{
              return (hiddenCount <= 1)? 2.0 : 1.0;
            }
          }
          // 6 turn
        }else if(turn == 6){
          // 高ポイント
          if(originalPoint >= 5){
            // 2pt以上割り振っている
            if(pointThisTurn[myId] >= 2){
              return BAN;
              // 1ptも割り振っていない
            }else{
              return 30.0;
            }
            // 低ポイント
          }else{
            // 4pt割り振っている
            if(pointThisTurn[myId] == 4){
              return BAN;
              // 2pt以上割り振っている
            }else if(pointThisTurn[myId] == 2){
              return BAN;
              // 1ptも割り振っていない
            }else{
              return (hiddenCount <= 1)? 2.0 : 1.0;
            }
          }
          // 2T 5pt
        }else{
          // 高ポイント
          if(originalPoint >= 5){
            // 2pt以上割り振っている
            if(pointThisTurn[myId] >= 2){
              return BAN;
              // 1ptも割り振っていない
            }else{
              return 30.0;
            }
            // 低ポイント
          }else{
            // 4pt割り振っている
            if(pointThisTurn[myId] == 4){
              return BAN;
              // 1pt以上割り振っている
            }else if(pointThisTurn[myId] >= 1){
              return (hiddenCount <= 2)? -1.0 : -10.0;
              // 1ptも割り振っていない
            }else{
              return (hiddenCount <= 1)? 2.0 : 1.0;
            }
          }
        }
        /*
         * 2位とのpt差が4 FSD4
         */
      }else if(diff == 4){
        // 3T
        if(turn == 3){
          // 5pt割り振る
          if(pointThisTurn[myId] == 5){
            // 高スコア
            if(originalPoint >= 5){
              if(hiddenCount >= 3){
                return BAN;
              }else{
                return (hiddenCount == 0)? 0.0 : -100.0;
              }
              // 低スコア
            }else{
              if(hiddenCount == 0){
                return -100.0;
              }else{
                return BAN;
              }
            }
            // 3pt以上割り振る
          }else if(pointThisTurn[myId] >= 3){
            // 高スコア
            if(originalPoint >= 5){
              if(highPointCount >= 4){
                return (hiddenCount == 0)? 1.0 : -1.0;
              }else{
                return (hiddenCount == 0)? 1.0 : -1.0;
              }
              // 低スコア
            }else{
              if(hiddenCount == 0){
                return -100.0;
              }else{
                return (hiddenCount == 0)? 1.0 : -10.0;
              }
            }
            // 1pt以上割り振る
          }else if(pointThisTurn[myId] >= 1){
            // 高スコア
            if(originalPoint >= 5){
              if(hiddenCount >= 4){
                return (hiddenCount <= 2)? 1.0 : -1.0;
              }else{
                return (hiddenCount <= 2)? 1.0 : -1.0;
              }
              // 低スコア
            }else{
              if(hiddenCount == 0){
                return -100.0;
              }else{
                return (hiddenCount <= 2)? 1.0 : -1.0;
              }
            }
            // 1ptも割り振らない
          }else{
            // 高スコア
            if(originalPoint >= 5){
              if(hiddenCount == 0){
                return 20.0;
              }else{
                return (hiddenCount <= 1)? 5.0 : 0.1;
              }
              // 低スコア
            }else{
              return (hiddenCount <= 2)? 1.0 : 0.5;
            }
          }
          // 7T FSD4T7
        }else if(turn == 7){
          // 5pt割り振る
          if(pointThisTurn[myId] == 5){
            // 高スコア
            if(originalPoint >= 5){
              if(hiddenCount >= 4){
                return BAN;
              }else{
                return (hiddenCount <= 1)? -1.0 : -300.0;
              }
              // 低スコア
            }else{
              if(hiddenCount == 0){
                return 2.0;
              }else{
                return (hiddenCount <= 1)? -2.0 : -400.0;
              }
            }
            // 4pt以上割り振る
          }else if(pointThisTurn[myId] >= 4){
            // 高スコア
            if(originalPoint >= 5){
              return (hiddenCount <= 1)? -1.0 : -100.0;
              // 低スコア
            }else{
              if(hiddenCount == 0){
                return 2.0;
              }else{
                return (hiddenCount <= 1)? -1.0 : -200.0;
              }
            }
            // 1pt以上割り振る
          }else if(pointThisTurn[myId] >= 1){
            // 高スコア
            if(originalPoint >= 5){
              return (hiddenCount <= 2)? -1.0 : -10.0;
              // 低スコア
            }else{
              if(hiddenCount == 0){
                return -100.0;
              }else{
                return (hiddenCount <= 2)? -1.0 : -10.0;
              }
            }
            // 1ptも割り振らない
          }else{
            // 高スコア
            if(originalPoint >= 5){
              if(hiddenCount == 0){
                return 20.0;
              }else{
                return (hiddenCount <= 1)? 5.0 : 0.1;
              }
              // 低スコア
            }else{
              return (hiddenCount <= 2)? 1.0 : 0.5;
            }
          }
          // 4 turn FSD4T4
        }else if(turn == 4){
          // 4pt割り振っている
          if(pointThisTurn[myId] == 4){
            // high point
            if(originalPoint >= 5){
              return (hiddenCount == 0)? -1.0 : BAN;
              // low point
            }else{
              if(originalPoint == 4){
                return (hiddenCount == 0)? -1.0 : BAN;
              }else{
                return BAN;
              }
            }
            // 2pt割り振っている
          }else if(pointThisTurn[myId] == 2){
            // high piont
            if(originalPoint >= 5){
              return (hiddenCount == 0)? -1.0 : -10.5;
              // low point
            }else{
              return (hiddenCount == 0)? -5.0 : -100.0;
            }
            // 1ptも割り振らない
          }else{
            // high point
            if(originalPoint >= 5){
              return (hiddenCount == 0)? 1.5 : -1.0;
              // low point
            }else{
              return (hiddenCount <= 2)? 0.5 : 1.0;
            }
          }
          // 2T - 6T - 8T - 4PT
        }else{
          // 1pt割り振っている
          if(pointThisTurn[myId] >= 1){
            if(originalPoint >= 5){
              return (hiddenCount <= 1)? -1.0 : -10.5;
            }else{
              if(hiddenCount == 0){
                return -10.0;
              }else{
                return (hiddenCount <= 1)? -1.0 : -10.0;
              }
            }
            // 1ptも割り振らない
          }else{
            if(originalPoint >= 5){
              return (hiddenCount <= 2)? 0.5 : 1.0;
            }else{
              return (hiddenCount <= 2)? 0.5 : 1.0;
            }
          }
        }
        /*
         * 2位との差が3pt FSD3
         */
      }else if(diff == 3){
        // 3T FSD3T3
        if(turn == 3){
          // 5pt割り振る
          if(pointThisTurn[myId] == 5){
            // 高ポイント
            if(originalPoint >= 5){
              if(highPointCount >= 4){
                return (hiddenCount == 0)? -10.0 : -400.0;
              }else{
                return (hiddenCount == 0)? -20.0 : BAN;
              }
              // 低ポイント
            }else{
              return (hiddenCount == 0)? -100.0 : BAN;
            }
            // 4pt以上割り振る
          }else if(pointThisTurn[myId] >= 4){
            // 高ポイント
            if(originalPoint >= 5){
              if(highPointCount >= 4){
                return (sumPoint <= 7)? -20.0 : -400.0;
              }else{
                return (sumPoint <= 7)? -2.0 : -200.0;
              }
              // 低ポイント
            }else{
              return (hiddenCount == 0)? -4.0 : -400.0;
            }
            // 3pt割り振る
          }else if(pointThisTurn[myId] == 3){
            // 高ポイント
            if(originalPoint >= 5){
              return (sumPoint <= 7)? 1.0 : -1.0;
              // 低ポイント
            }else{
              if(highPointCount >= 3){
                return (hiddenCount == 0)? -5.0 : -100.0;
              }else{
                if(originalPoint == 4){
                  return (hiddenCount == 0)? 1.0 : -100.0;
                }else{
                  return (hiddenCount == 0)? 0.0 : -200.0;
                }
              }
            }
            // 2pt以上割り振る
          }else if(pointThisTurn[myId] >= 2){
            // 高ポイント
            if(originalPoint >= 5){
              return (sumPoint <= 7)? 1.0 : -1.0;
              // 低ポイント
            }else{
              return (hiddenCount <= 1)? 1.0 : 0.5;
            }
            // 1pt割り振る
          }else if(pointThisTurn[myId] >= 1){
            // 高スコア
            if(originalPoint >= 5){
              return (hiddenCount <= 1)? -1.0 : -2.0;
            }else{
              return (hiddenCount <= 1)? 1.0 : 0.5;
            }
            // 0pt
          }else{
            // high point
            if(originalPoint >= 5){
              return (hiddenCount <= 1)? -1.0 : -2.0;
              // low point
            }else{
              return (hiddenCount <= 1)? 1.0 : 0.5;
            }
          }
          // 7T FSD3T7
        }else if(turn == 7){
          // 5pt割り振る
          if(pointThisTurn[myId] == 5){
            // 高ポイント
            if(originalPoint >= 5){
              if(highPointCount <= 2){
                return (hiddenCount <= 1)? 1.0 : -100.0;
              }else{
                return (hiddenCount <= 1)? -2.0 : -1000.0;
              }
              // 低ポイント
            }else{
              if(highPointCount <= 2){
                return (hiddenCount == 0)? -10.0 : -200.0;
              }else{
                return (hiddenCount == 0)? -20.0 : -400.0;
              }
            }
            // 4pt以上割り振る
          }else if(pointThisTurn[myId] >= 4){
            // 高ポイント
            if(originalPoint >= 5){
              return (hiddenCount <= 1)? 2.0 : -10.0;
              // 低ポイント
            }else{
              return (hiddenCount <= 1)? 1.0 : -20.0;
            }
            // 2pt以上割り振る
          }else if(pointThisTurn[myId] >= 2){
            // 高ポイント
            if(originalPoint >= 5){
              return (hiddenCount <= 1)? 2.0 : 0.9;
              // 低ポイント
            }else{
              return (hiddenCount <= 1)? 1.0 : 0.5;
            }
            // 1pt割り振る
          }else if(pointThisTurn[myId] == 1){
            // 高スコア
            if(originalPoint >= 5){
              return (hiddenCount <= 1)? 1.0 : -2.0;
            }else{
              return (hiddenCount <= 1)? 1.0 : 0.5;
            }
            // 0pt
          }else{
            // 高スコア
            if(originalPoint >= 5){
              return (hiddenCount == 0)? -1.0 : -2.0;
            }else{
              if(originalPoint == 4){
                return (hiddenCount == 0)? 4.0 : 0.5;
              }else{
                return (hiddenCount == 0)? 5.0 : 0.5;
              }
            }
          }
          // 4ターン目 FSD3T4
        }else if(turn == 4){
          // 4pt振っている
          if(pointThisTurn[myId] == 4){
            // 高スコア
            if(originalPoint >= 5){
              if(hiddenCount >= 3){
                return BAN;
              }else{
                return (hiddenCount <= 1)? 1.0 : -40.0;
              }
              // 低スコア
            }else{
              if(highPointCount <= 2){
                if(originalPoint == 4){
                  return (hiddenCount == 0)? 0.0 : -50.0;
                }else{
                  return (hiddenCount == 0)? -2.0 : -100.0;
                }
              }else{
                return (hiddenCount == 0)? -5.0 : BAN;
              }
            }
            // 2pt振っている
          }else if(pointThisTurn[myId] >= 2){
            if(originalPoint >= 5){
              return (hiddenCount == 0)? 10.0 : 0.5;
            }else{
              return (hiddenCount == 0)? 0.0 : 1.5;
            }
            // 何も降っていない
          }else{
            if(originalPoint >= 5){
              return (hiddenCount <= 1)? -1.0 : -2.0;
            }else{
              return (hiddenCount <= 1)? 1.0 : 0.9;
            }
          }
          // 8 turn FSD3T8
        }else if(turn == 8){
          // 4pt振っている
          if(pointThisTurn[myId] == 4){
            // 高ポイント
            if(originalPoint >= 5){
              return (hiddenCount == 0)? -10 : -50.0;
              // 低ポイント
            }else{
              return (hiddenCount <= 1)? -20.0 : -100.0;
            }
            // 2pt以上振っている
          }else if(pointThisTurn[myId] >= 2){
            // 高ポイント
            if(originalPoint >= 5){
              return (hiddenCount <= 1)? -1.0 : -5.0;
              // 低ポイント
            }else{
              if(highPointCount >= 3){
                return (hiddenCount == 0)? -10.0 : -50.0;
              }else{
                return (hiddenCount == 0)? -2.0 : -10.0;
              }
            }
            // 何も降っていない
          }else{
            // high point
            if(originalPoint >= 5){
              if(highPointCount >= 3){
                return (hiddenCount <= 1)? 5.0 : 0.5;
              }else{
                return (hiddenCount <= 1)? 2.0 : 0.5;
              }
              // low point
            }else{
              if(highPointCount >= 3){
                return (hiddenCount <= 1)? 5.0 : 0.5;
              }else{
                return (hiddenCount <= 1)? 2.0 : 0.5;
              }
            }
          }
          // 2 turn FSD3T2
        }else if(turn == 2){
          // 4pt割り振る
          if(pointThisTurn[myId] == 4){
            // 高ポイント
            if(originalPoint >= 5){
              return -sumPoint;
              // 低ポイント
            }else{
              if(highPointCount >= 3){
                return -100.0;
              }else{
                return -200.0;
              }
            }
            // 2pt以上振っている
          }else if(pointThisTurn[myId] >= 2){
            // 高ポイント
            if(originalPoint >= 5){
              return -1.0;
              // 低ポイント
            }else{
              if(highPointCount >= 3){
                return -10.0;
              }else{
                return -5.0;
              }
            }
            // 何も降っていない
          }else{
            if(originalPoint >= 5){
              if(originalPoint == 6){
                return -2.0;
              }else{
                return -1.0;
              }
            }else{
              return 1.0;
            }
          }
          // 6 turn FSD3T6
        }else{
          // 4pt割り振る
          if(pointThisTurn[myId] == 4){
            // 高ポイント
            if(originalPoint >= 5){
              return -10.0;
              // 低ポイント
            }else{
              if(playerRank[myId] != 1){
                return -300.0;
              }else{
                return 0.0;
              }
            }
            // 何も降っていない
            // 2pt以上振っている
          }else if(pointThisTurn[myId] >= 2){
            // 高ポイント
            if(originalPoint >= 5){
              return -1.0;
              // 低ポイント
            }else{
              if(highPointCount >= 3){
                return -20.0;
              }else{
                return -10.0;
              }
            }
            // 何も降っていない
          }else{
            return (hiddenCount <= 1)? 1.0 : 0.5;
          }
        }
        /*
         * 2位との差が2pt FSD2
         */
      }else if(diff == 2){
        // 3 turn FSD2T3
        if(turn == 3){
          // 4pt以上割り振る
          if(pointThisTurn[myId] >= 4){
            // 高スコア
            if(originalPoint >= 5){
              if(hiddenCount >= 4){
                return -200.0;
              }else{
                if(sumPoint >= 6 && playerRank[myId] != 1){
                  if(highPointCount >= 3){
                    return (hiddenCount == 0)? -10.0 : -200.0;
                  }else{
                    return (hiddenCount == 0)? -5.0 : -100.0;
                  }
                }else{
                  return (hiddenCount == 0)? -5.0 : -20.0;
                }
              }
              // 低スコア
            }else{
              if(highPointCount >= 3){
                return (hiddenCount == 0)? -2.0 : -400.0;
              }else{
                return (hiddenCount == 0)? -4.0 : -800.0;
              }
            }
            // 3pt以上割り振る
          }else if(pointThisTurn[myId] >= 3){
            // 高スコア
            if(originalPoint >= 5){
              if(hiddenCount >= 4){
                return -200.0;
              }else{
                if(sumPoint >= 6 && playerRank[myId] != 1){
                  return (hiddenCount <= 1)? -1.0 : -20.0;
                }else{
                  return (hiddenCount <= 1)? -5.0 : -20.0;
                }
              }
              // 低スコア
            }else{
              if(highPointCount >= 3){
                return (hiddenCount == 0)? -1.0 : -40.0;
              }else{
                return (hiddenCount == 0)? 0.0 : -80.0;
              }
            }
            // 2pt以上割り振っている
          }else if(pointThisTurn[myId] >= 2){
            // 高ポイント
            if(originalPoint >= 5){
              if(hiddenCount >= 4){
                return -20.0;
              }else{
                if(sumPoint >= 6 && turn == 3 && playerRank[myId] != 1){
                  return (hiddenCount <= 1)? 1.0 : -2.0;
                }else{
                  return (hiddenCount <= 1)? 2.0 : -1.0;
                }
              }
              // 低ポイント
            }else{
              if(highPointCount >= 4){
                return (hiddenCount == 0)? -1.0 : -10.0;
              }else{
                return (hiddenCount == 0)? 1.0 : -2.0;
              }
            }
            // 1pt以上割り振っている
          }else if(pointThisTurn[myId] >= 1){
            // 高スコア
            if(originalPoint >= 5){
              if(highPointCount >= 3){
                if(sumPoint - point[myId] >= 8){
                  return -1.0;
                }else{
                  return (hiddenCount <= 1)? 2.0 : -2.0;
                }
              }else{
                return (hiddenCount <= 1)? 2.0 : -5.0;
              }
              // 低スコア
            }else{
              return (hiddenCount <= 1)? 1.0 : 0.0;
            }
            // 1ptも割り振っていない
          }else{
            // 高スコア
            if(originalPoint >= 5){
              return (hiddenCount == 0)? -4.0 : -2.0;
              // 低スコア
            }else{
              return (hiddenCount == 0)? -0.5 : -1.0;
            }
          }
          // 7T FSD2T7
        }else if(turn == 7){
          // 5pt割り振る
          if(pointThisTurn[myId] >= 5){
            // 高ポイント
            if(originalPoint >= 5){
              if(hiddenCount >= 2){
                return BAN;
              }else{
                if(point[myId] - lastPoint >= 6){
                  return (hiddenCount == 0)? -10.0 : -100.0;
                }else{
                  return (hiddenCount == 0)? -5.0 : -100.0;
                }
              }
              // 低ポイント
            }else{
              if(highPointCount <= 2){
                if(originalPoint == 4){
                  return (hiddenCount == 0)? -10.0 : BAN;
                }else{
                  return (hiddenCount == 0)? -20.0 : BAN;
                }
              }else{
                return (hiddenCount == 0)? -50.0 : BAN;
              }
            }
            // 4pt以上割り振る
          }else if(pointThisTurn[myId] >= 4){
            // 高ポイント
            if(originalPoint >= 5){
              if(hiddenCount >= 2){
                return -20.0;
              }else{
                if(point[myId] - lastPoint >= 6){
                  return (hiddenCount <= 1)? -10.0 : -100.0;
                }else{
                  return (hiddenCount <= 1)? -2.0 : -10.0;
                }
              }
              // 低ポイント
            }else{
              if(highPointCount <= 2){
                if(hiddenCount >= 3){
                  return BAN;
                }else{
                  return (hiddenCount == 0)? -5.0 : -50.0;
                }
              }else{
                return BAN;
              }
            }
            // 3pt割り振っている
          }else if(pointThisTurn[myId] == 3){
            // high score
            if(originalPoint >= 5){
              if(hiddenCount >= 4){
                return -20.0;
              }else{
                if(sumPoint - point[myId] >= 15 && playerRank[myId] != 1){
                  return (hiddenCount <= 1)? 1.0 : -20.0;
                }else{
                  return (hiddenCount <= 1)? 2.0 : -10.0;
                }
              }
              // low score
            }else{
              if(highPointCount >= 3){
                return (hiddenCount == 0)? -1.0 : BAN;
              }else{
                if(originalPoint == 4){
                  return (hiddenCount == 0)? 1.0 : -200.0;
                }else{
                  return (hiddenCount == 0)? 0.0 : -400.0;
                }
              }
            }
            // 2pt以上割り振っている
          }else if(pointThisTurn[myId] >= 2){
            if(originalPoint >= 5){
              if(hiddenCount >= 4){
                return -20.0;
              }else{
                if(sumPoint - point[myId] >= 15 && playerRank[myId] != 1){
                  return (hiddenCount <= 1)? 1.0 : -20.0;
                }else{
                  return (hiddenCount <= 1)? 2.0 : -10.0;
                }
              }
            }else{
              return (hiddenCount <= 1)? 1.0 : -2.0;
            }
            // 1pt以上割り振っている
          }else if(pointThisTurn[myId] >= 1){
            // 高スコア
            if(originalPoint >= 5){
              return (hiddenCount <= 1)? 2.0 : 1.0;
              // 低スコア
            }else{
              return (hiddenCount <= 1)? 1.0 : 0.3;
            }
            // 1ptも割り振っていない
          }else{
            // 高スコア
            if(originalPoint >= 5){
              return (hiddenCount <= 1)? -1.0 : -2.0;
              // 低スコア
            }else{
              return (hiddenCount <= 1)? -0.5 : -1.0;
            }
          }
          // 2T FSD2T2
        }else if(turn == 2){
          // 4pt割り振る
          if(pointThisTurn[myId] == 4){
            // 1ターン目誰も置いていない
            if(sumPoint == 0){
              // 高スコア
              if(originalPoint >= 5){
                return 1000.0;
                // 低スコア
              }else{
                return 1.0;
              }
              // 誰かが何かしら置いている
            }else{
              // 1ターン目の合計値が高い
              if(sumPoint >= 8){
                return BAN;
              }else{
                // 高ポイント
                if(originalPoint >= 5){
                  if(highPointCount >= 4){
                    return (sumPoint <= 2)? 0.0 : BAN;
                  }else{
                    return (sumPoint <= 2)? 10.0 : BAN;
                  }
                  // 低いポイント
                }else{
                  if(highPointCount >= 3){
                    return BAN;
                  }else{
                    if(originalPoint == 4){
                      return (sumPoint <= 1)? -1.0 : BAN;
                    }else{
                      return (sumPoint <= 1)? -2.0 : BAN;
                    }
                  }
                }
              }
            }
            // 2pt割り振る
          }else if(pointThisTurn[myId] == 2){
            if(sumPoint == 0){
              // high point
              if(originalPoint >= 5){
                return 100.0;
                // low point
              }else{
                // 4pt
                if(originalPoint == 4){
                  if(highPointCount >= 4){
                    return 4.0;
                  }else{
                    return 40.0;
                  }
                  // 3pt
                }else{
                  if(highPointCount >= 4){
                    return -10.0;
                  }else{
                    return 0.0;
                  }
                }
              }
            }else{
              // 1ターン目の合計値が高い
              if(sumPoint >= 8){
                return -10.0;
              }else{
                // 高ポイント
                if(originalPoint >= 5){
                  return 1.0;
                  // 低いポイント
                }else{
                  if(originalPoint == 4){
                    return 1.0;
                  }else{
                    return 0.0;
                  }
                }
              }
            }
            // 0pt
          }else{
            // 高ポイント
            if(originalPoint >= 5){
              if(originalPoint == 6){
                return -6.0;
              }else{
                return -5.0;
              }
              // 低いポイント
            }else{
              return 1.0;
            }
          }
          // 4 turn FSD2T4
        }else if(turn == 4){
          // 4pt振っている
          if(pointThisTurn[myId] == 4){
            // 高スコア
            if(originalPoint >= 5){
              if(highPointCount >= 4){
                return (hiddenCount == 0)? -1.0 : BAN;
              }else{
                return (hiddenCount == 0)? -2.0 : BAN;
              }
              // 低スコア
            }else{
              if(highPointCount >= 3){
                if(originalPoint == 4){
                  return (hiddenCount == 0)? -100.0 : BAN;
                }else{
                  return (hiddenCount == 0)? -300.0 : BAN;
                }
              }else{
                if(originalPoint == 4){
                  return (hiddenCount == 0)? -10.0 : BAN;
                }else{
                  return (hiddenCount == 0)? -40.0 : BAN;
                }
              }
            }
            // 2ptポイントを振っている
          }else if(pointThisTurn[myId] == 2){
            // 高ポイント
            if(originalPoint >= 5){
              if(sumPoint - point[myId] >= 8){
                return BAN;
              }else{
                return (hiddenCount <= 1)? 1.0 : 2.0;
              }
              // 低ポイント
            }else{
              return (hiddenCount == 0)? 0.2 : -10.0;
            }
            // 1ptも降っていない
          }else{
            // 高ポイント
            if(originalPoint >= 5){
              return (hiddenCount <= 1)? -1.0 : -2.0;
              // 低ポイント
            }else{
              return (hiddenCount <= 1)? 0.0 : -0.5;
            }
          }
          // 8 turn FSD2T8
        }else if(turn == 8){
          // 4pt振っている
          if(pointThisTurn[myId] == 4){
            // high point
            if(originalPoint >= 5){
              return (hiddenCount == 0)? 1.0 : BAN;
            // low point
            }else{
              if(highPointCount >= 3){
                return (hiddenCount == 0)? -40.0 : BAN;
              }else{
                if(originalPoint == 4){
                  return (hiddenCount == 0)? -50.0 : BAN;
                }else{
                  return (hiddenCount == 0)? -100.0 : BAN;
                }
              }
            }
            // 2pt以上ポイントを振っている
          }else if(pointThisTurn[myId] >= 2){
            // 高ポイント
            if(originalPoint >= 5){
              return (hiddenCount == 0)? 1.0 : -20.0;
              // 低ポイント
            }else{
              return (hiddenCount == 0)? 0.0 : -40.0;
            }
            // 1ptも降っていない
          }else{
            // 高ポイント
            if(originalPoint >= 5){
              return (hiddenCount <= 1)? -1.0 : -2.0;
              // 低ポイント
            }else{
              return (hiddenCount <= 1)? 0.0 : -0.5;
            }
          }
          // 6 2PT FSD2T6
        }else{
          // 4pt振っている
          if(pointThisTurn[myId] == 4){
            // high score
            if(originalPoint >= 5){
              return (sumPoint <= 2)? 1.0 : -20.0;
              // low score
            }else{
              if(highPointCount <= 2){
                if(originalPoint == 4){
                  return -200;  
                }else{
                  return BAN;
                }
              }else{
                return BAN;
              }
            }
            // 2pt以上ポイントを振っている
          }else if(pointThisTurn[myId] >= 2){
            // 高ポイント
            if(originalPoint >= 5){
              return 1.0;
              // 低ポイント
            }else{
              if(originalPoint == 4){
                if(highPointCount >= 3){
                  return -1.0;
                }else{
                  return 0.5;
                }
              }else{
                if(highPointCount >= 3){
                  return -10.0;
                }else{
                  return 0.2;
                }
              }
            }
            // 1ptも降っていない
          }else{
            // 高ポイント
            if(originalPoint >= 5){
              return -1.0;
              // 低ポイント
            }else{
              if(originalPoint == 4){
                return 0.0;
              }else{
                return 1.0;
              }
            }
          }
        }
        /*
         * 2位との差が1pt FSD1
         */
      }else if(diff == 1){
        // 2 turn FSD1T2
        if(turn == 2){
          // 2pt以上割り振る
          if(pointThisTurn[myId] >= 2){
            // 高ポイント
            if(originalPoint >= 5){
              // まだ場にポイントが無い
              if(sumPoint == 0){
                return 50.0;
              }else{
                return (sumPoint <= 1)? 4.0 : -1000.0;
              }
              // 低ポイント
            }else{
              if(sumPoint == 0){
                if(originalPoint == 4){
                  return 20.0;
                }else{
                  return 10.0;
                }
              }else{
                // 高スコアが少ない
                if(highPointCount <= 2){
                  if(originalPoint == 4){
                    return (sumPoint <= 2)? 0.0 : -100.0;
                  }else{
                    return (sumPoint <= 2)? 0.0 : -150.0;
                  }
                }else{
                  if(originalPoint == 4){
                    return (sumPoint <= 1)? -3.0 : -100.0;
                  }else{
                    return (sumPoint <= 1)? -4.0 : -150.0;
                  }
                }
              }
            }
            // 1ptも割り振らない
          }else{
            // 高ポイント
            if(originalPoint >= 5){
              if(highPointCount <= 2){
                return -20.0;
              }else{
                if(originalPoint == 6){
                  return -15.0;
                }else{
                  return -10.0;
                }
              }
              // 低ポイント
            }else{
              // 高スコアが少ない
              if(hiddenCount <= 2){
                return 0.0;
              }else{
                return 0.0;
              }
            }
          }
          // 4 turn FSD1T4
        }else if(turn == 4){
          // 4つのポイントを割り振った場合
          if(pointThisTurn[myId] >= 4){
            // 高ポイント
            if(originalPoint >= 5){
              if(sumPoint - point[myId] >= 8){
                if(highPointCount <= 3){
                  return (hiddenCount == 0)? -20.0 : BAN;
                }else{
                  return (hiddenCount == 0)? -10.0 : -1000.0;
                }
              }else{
                return (hiddenCount == 0)? -10.0 : -100.0;
              }
              // 低ポイント
            }else{
              if(highPointCount >= 3){
                return BAN;
              }else{
                if(highPointCount >= 3){
                  if(originalPoint == 4){
                    return (hiddenCount == 0)? -500.0 : BAN;
                  }else{
                    return BAN;
                  }
                }else{
                  return (hiddenCount == 0)? -200.0 : BAN;
                }
              }
            }
            // 2つのポイントを割り振った場合
          }else if(pointThisTurn[myId] >= 2){
            // high score
            if(originalPoint >= 5){
              if(hiddenCount == 0){
                return 10.0;
              }else{
                // 敵ポイントの合計値が15を超えている
                if(sumPoint >= 15){
                  return -10.0;
                }else{
                  return (hiddenCount == 1)? 1.0 : -10.0;
                }
              }
              // low score
            }else{
              if(highPointCount >= 3){
                return (hiddenCount == 0)? -10.0 : BAN;
              }else{
                return (hiddenCount == 0)? -1.0 : -100.0;
              }
            }
            // 何もポイントを振らない
          }else{
            // 高スコア
            if(originalPoint >= 5){
              return (hiddenCount <= 1)? -10.0 : -20.0;
              // 低スコア
            }else{
              return (hiddenCount == 0)? 0.5 : 0.2;
            }
          }
          // 3 turn FSD1T3
        }else if(turn == 3){
          // 4pt以上を割り振る
          if(pointThisTurn[myId] >= 4){
            // 高ポイント
            if(originalPoint >= 5){
              if(hiddenCount >= 3){
                return BAN;
              }else{
                if(highPointCount <= 2){
                  return (hiddenCount == 0)? -20.0 : -700.0;
                }else{
                  return (hiddenCount == 0)? -10.0 : -400.0;
                }
              }
              // 低ポイント
            }else{
              return (hiddenCount == 0)? -20.0 : -200.0;
            }
            // 3pt以上を割り振る
          }else if(pointThisTurn[myId] >= 3){
            // 高ポイント
            if(originalPoint >= 5){
              if(highPointCount <= 2){
                return (hiddenCount == 0)? -20.0 : -600.0;
              }else{
                return (hiddenCount == 0)? -10.0 : -300.0;
              }
              // 低ポイント
            }else{
              return (hiddenCount == 0)? -8.0 : -120.0;
            }
            // 2pt以上を割り振る
          }else if(pointThisTurn[myId] >= 2){
            // high point
            if(originalPoint >= 5){
              return (hiddenCount == 1)? -5.0 : -40.0;
              // low point
            }else{
              return (hiddenCount == 1)? -10.0 : -20.0;
            }
            // 1pt割り振る
          }else if(pointThisTurn[myId] >= 1){
            return (hiddenCount <= 1)? -1.0 : -2.0;
            // 0pt
          }else{
            if(originalPoint >= 5){
              return (hiddenCount == 0)? -5.0 : -1.0;
            }else{
              return (hiddenCount == 0)? -1.0 : -2.0;
            }
          }
          // 7T 1pt
        }else if(turn == 7){
          // 5ptを割り振る
          if(pointThisTurn[myId] == 5){
            if(originalPoint >= 5){
              return (hiddenCount == 0)? -200.0 : BAN;
            }else{
              return BAN;
            }
            // 4ptを割り振る
          }else if(pointThisTurn[myId] == 4){
            if(originalPoint >= 5){
              return (hiddenCount == 0)? -150.0 : BAN;
            }else{
              return BAN;
            }
            // 3pt以上を割り振る
          }else if(pointThisTurn[myId] >= 3){
            if(originalPoint >= 5){
              return (hiddenCount == 0)? -10.0 : -200.0;
            }else{
              return (hiddenCount == 0)? -50.0 : -400.0;
            }
            // 2pt割り振る
          }else if(pointThisTurn[myId] >= 2){
            // high point
            if(originalPoint >= 5){
              return (hiddenCount == 0)? -1.0 : -2.0;
              // low point
            }else{
              if(highPointCount >= 3){
                return (hiddenCount == 0)? -2.0 : -10.0;
              }else{
                return (hiddenCount == 0)? -1.0 : -5.0;
              }
            }
          }else{
            return (hiddenCount <= 1)? -1.0 : -2.0;
          }
          // 6 turn FSD1T6
        }else if(turn == 6){
          // 4pt振っている
          if(pointThisTurn[myId] >= 4){
            // 高ポイント
            if(originalPoint >= 5){
              if(highPointCount <= 2){
                return BAN;
              }else{
                return -1000.0;
              }
              // 低ポイント
            }else{
              if(highPointCount <= 2){
                return -400.0;
              }else{
                return -2000.0;
              }
            }
            // 2pt振っている
          }else if(pointThisTurn[myId] >= 2){
            // 高スコア
            if(originalPoint >= 5){
              if(highPointCount >= 4){
                return -100.0;
              }else{
                return -50.0;
              }
              // 低スコア
            }else{
              return -100.0;
            }
            // 何も振っていない
          }else{
            if(originalPoint >= 5){
              return -10.0;
            }else{
              return -1.0;
            }
          }
          // 8T 1pt FSD1T8
        }else{
          // 4pt振っている
          if(pointThisTurn[myId] >= 4){
            // high point
            if(originalPoint >= 5){
              if(hiddenCount >= 3){
                return BAN;
              }else{
                return (hiddenCount == 0)? 0.0 : -400.0;
              }
              // low point
            }else{
              if(originalPoint == 4){
                return (hiddenCount == 0)? -2.0 : BAN;
              }else{
                return (hiddenCount == 0)? -10.0 : BAN;
              }
            }
            // 2pt振っている
          }else if(pointThisTurn[myId] >= 2){
            // 高スコア
            if(originalPoint >= 5){
              return (hiddenCount == 0)? 1.0 : -20.0;
              // 低スコア
            }else{
              return (hiddenCount == 0)? 0.0 : -40.0;
            }
            // 何も振っていない
          }else{
            // high point
            if(originalPoint >= 5){
              return (hiddenCount <= 1)? -5.0 : -10.0;
              // low point
            }else{
              return (hiddenCount <= 1)? -10.0 : -20.0;
            }
          }
        }
        /*
         * 2位との差が0pt FSD0
         */
      }else{
        // 8T FSD0T8
        if(turn == 8){
          if(pointThisTurn[myId] == 4){
            // 高スコア
            if(originalPoint >= 5){
              return (hiddenCount == 0)? -400.0 : BAN;
              // 低スコア
            }else{
              if(originalPoint == 4){
                return (hiddenCount == 0)? -800.0 : BAN;
              }else{
                return BAN;
              }
            }
            // 2pt以上割り振る
          }else if(pointThisTurn[myId] >= 2){
            // 高スコア
            if(originalPoint >= 5){
              if(hiddenCount >= 3){
                return BAN;
              }else{
                return (hiddenCount == 0)? -1.0 : -40.0;
              }
              // 低スコア
            }else{
              if(hiddenCount >= 3){
                return BAN;
              }else{
                return (hiddenCount == 0)? -3.0 : -100.0;
              }
            }
            // 1ptも割り振らない
          }else{
            return (hiddenCount <= 1)? -1.0 : -2.0;
          }
          // 6T
        }else if(turn == 6){
          // 4pt割り振る
          if(pointThisTurn[myId] == 4){
            // high point
            if(originalPoint >= 5){
              if(sumPoint - point[myId] >= 10){
                return BAN;
              }else{
                return -1000.0;
              }
              // low point
            }else{
              return BAN;
            }
            // 2pt割り振る
          }else if(pointThisTurn[myId] == 2){
            // 高ポイント
            if(originalPoint >= 5){
              if(sumPoint - point[myId] >= 12){
                return -100.0;
              }else{
                return -10.0;
              }
              // 低ポイント
            }else{
              if(highPointCount <= 2){
                return -800.0;
              }else{
                return -1000.0;
              }
            }
            // 割り振らない
          }else{
            if(originalPoint >= 5){
              return -100.0;
            }else{
              return -10.0;
            }
          }
          // 4 turn 
        }else if(turn == 4){
          // 4pt割り振る
          if(pointThisTurn[myId] == 4){
            // high point
            if(originalPoint >= 5){
              if(highPointCount <= 3){
                return (hiddenCount == 0)? 0.0 : -200.0;
              }else{
                return (hiddenCount == 0)? 0.0 : -400.0;
              }
              // low point
            }else{
              return BAN;
            }
            // 2pt割り振る
          }else if(pointThisTurn[myId] == 2){
            // 高ポイント
            if(originalPoint >= 5){
              if(sumPoint - point[myId] >= 13){
                return (hiddenCount <= 1)? -1000.0 : BAN;
              }else if(sumPoint - point[myId] >= 10){
                if(highPointCount <= 2){
                  return (hiddenCount <= 1)? -40.0 : -80.0;
                }else{
                  return (hiddenCount <= 1)? -20.0 : -40.0;
                }
              }else{
                if(hiddenCount >= 3){
                  return BAN;
                }else{
                  return (hiddenCount == 0)? -2.0 : -200.0;
                }
              }
              // 低ポイント
            }else{
              return -200.0;
            }
          }else{
            return (hiddenCount <= 1)? -1.0 : -2.0;
          }
          // 2 turn FSD0T2
        }else if(turn == 2){
          // 4pt割り振る
          if(pointThisTurn[myId] == 4){
            // 高ポイント
            if(originalPoint >= 5){
              if(highPointCount <= 2){
                return -100.0;
              }else{
                return -50.0;
              }
              // 低ポイント
            }else{
              return -300.0;
            }
            // 2pt割り振る
          }else if(pointThisTurn[myId] == 2){
            // 高スコア
            if(originalPoint >= 5){
              if(sumPoint >= 4){
                return BAN;
              }else if(sumPoint - point[myId] >= 1){
                return -100.0;
              }else{
                return -1.0;
              }
              // 低スコア
            }else{
              return -300.0;
            }
            // 0pt
          }else{
            // high point
            if(originalPoint >= 5){
              return -4.0;
              // low point
            }else{
              return 0.0;
            }
          }
          // turn 3 FSD0T3
        }else if(turn == 3){
          // 3pt以上割り振る
          if(pointThisTurn[myId] >= 3){
            // 高ポイント
            if(originalPoint >= 5){
              if(sumPoint - point[myId] >= 4){
                return BAN;
              }else{
                return (hiddenCount == 0)? -1.0 : -80.0;
              }
              // 低ポイント
            }else{
              return (hiddenCount == 0)? -2.0 : -160.0;
            }
            // 2pt割り振る
          }else if(pointThisTurn[myId] == 2){
            // 高ポイント
            if(originalPoint >= 5){
              return (hiddenCount == 0)? -1.0 : -200.0;
              // 低ポイント
            }else{
              if(highPointCount <= 2){
                return (hiddenCount == 0)? -10.0 : -800.0;
              }else{
                return (hiddenCount == 0)? -50.0 : -800.0;
              }
            }
            // 1pt以上割り振る
          }else if(pointThisTurn[myId] >= 1){
            // 高ポイント
            if(originalPoint >= 5){
              return (hiddenCount == 0)? -1.0 : -20.0;
              // 低ポイント
            }else{
              return (hiddenCount == 0)? -2.0 : -80.0;
            }
            // 何も割り振らない
          }else{
            // 高ポイント
            if(originalPoint >= 5){
              return (hiddenCount <= 1)? -1.0 : -2.0;
              // 低ポイント
            }else{
              return (hiddenCount <= 1)? -1.0 : -2.0;
            }
          }
          // 7 turn FSD0T7
        }else if(turn == 7){
          if(pointThisTurn[myId] == 5){
            // 高ポイント
            if(originalPoint >= 5){
              if(highPointCount >= 4){
                return BAN;
              }else{
                return (hiddenCount == 0)? -500.0 : BAN;
              }
              // 低ポイント
            }else{
              return BAN;
            }
            // 4pt以上割り振る
          }else if(pointThisTurn[myId] >= 4){
            // 高ポイント
            if(originalPoint >= 5){
              return (hiddenCount == 0)? -500.0 : BAN;
              // 低ポイント
            }else{
              return (hiddenCount == 0)? -1000.0 : BAN;
            }
            // 3pt割り振る
          }else if(pointThisTurn[myId] >= 3){
            // 高ポイント
            if(originalPoint >= 5){
              return (hiddenCount == 0)? -10.0 : -200.0;
              // 低ポイント
            }else{
              return (hiddenCount == 0)? -20.0 : -400.0;
            }
            // 2pt以上割り振る
          }else if(pointThisTurn[myId] >= 2){
            // 高ポイント
            if(originalPoint >= 5){
              return (hiddenCount == 0)? -10.0 : -200.0;
              // 低ポイント
            }else{
              return (hiddenCount == 0)? -20.0 : -400.0;
            }
            // 1pt以上割り振る
          }else if(pointThisTurn[myId] >= 1){
            // 高ポイント
            if(originalPoint >= 5){
              return (hiddenCount == 0)? -1.0 : -20.0;
              // 低ポイント
            }else{
              return (hiddenCount == 0)? -2.0 : -40.0;
            }
            // 何も割り振らない
          }else{
            // 高ポイント
            if(originalPoint >= 5){
              return (hiddenCount <= 1)? -1.0 : -2.0;
              // 低ポイント
            }else{
              return (hiddenCount <= 1)? -1.0 : -2.0;
            }
          }
        }else{
          return (hiddenCount <= 1)? -1.0 : -2.0;
        }
      }
    }

    /*
     * スコア計算前の1位の評価
     * 低いほど悪い
     */
    double fuzzyScoreLast(int diff){
      /*
       * 2位との差が6pt以上 FSLD6
       */
      if(diff >= 6){
        // 4pt以上割り振る
        if(pointThisTurn[myId] >= 4){
          // 高スコア
          if(originalPoint >= 5){
            // 隠しポイントが多い
            if(hiddenCount >= 5){
              return BAN;
            }else{
              return (hiddenCount <= 3)? BAN : -1000.0;
            }
            // 低スコア
          }else{
            return BAN;
          }
          // 3pt以上割り振る
        }else if(pointThisTurn[myId] >= 3){
          // 高スコア
          if(originalPoint >= 5){
            if(highPointCount >= 3){
              if(diff >= 8){
                return (hiddenCount <= 2)? BAN : -100.0;
              }else{
                return (hiddenCount <= 2)? BAN : -10.0;
              }
            }else{
              return (hiddenCount <= 2)? BAN : -10.0;
            }
            // 低スコア
          }else{
            return BAN;
          }
          // 1pt以上割り振る
        }else if(pointThisTurn[myId] >= 1){
          // 高スコア
          if(originalPoint >= 5){
            if(highPointCount >= 3){
              return (hiddenCount <= 2)? BAN : 2.0;
            }else{
              return (hiddenCount <= 2)? BAN : 2.0;
            }
            // 低スコア
          }else{
            return BAN;
          }
          // 1ptも割り振らない
        }else{
          // 高スコア
          if(originalPoint >= 5){
            return (hiddenCount <= 1)? 10.0 : 1.0;
            // 低スコア
          }else{
            return (hiddenCount <= 1)? 20.0 : 1.0;
          }
        }
        /*
         * 2位との差が5pt FSLD5
         */
      }else if(diff == 5){
        // 5pt割り振っている
        if(pointThisTurn[myId] == 5){
          // 高スコア
          if(originalPoint >= 5){
            if(hiddenCount <= 1){
              return -50.0;
            }else{
              // 最初のスコア計算
              if(turn == 5){
                if(hiddenCount >= 4){
                  return BAN;
                }else{
                  return (hiddenCount <= 2)? -10.0 : -1000.0;
                }
                // 最後のスコア計算
              }else{
                return (hiddenCount <= 2)? -10.0 : -1000.0;
              }
            }
            // 低スコア
          }else{
            return BAN;
          }
        // 4pt割り振る
        }else if(pointThisTurn[myId] == 4){
          // 高スコア
          if(originalPoint >= 5){
            if(hiddenCount <= 1){
              return -50.0;
            }else{
              // 最初のスコア計算
              if(turn == 5){
                if(hiddenCount >= 3){
                  return -20.0;
                }else{
                  return (hiddenCount <= 1)? -100.0 : -1.0;
                }
                // 最後のスコア計算
              }else{
                return (hiddenCount <= 1)? -10.0 : 1.0;
              }
            }
            // low point
          }else{
            if(highPointCount >= 3){
              return (hiddenCount == 0)? BAN : -200.0;
            }else{
              return (hiddenCount == 0)? BAN : -200.0;
            }
          }
          // 3pt以上割り振っている
        }else if(pointThisTurn[myId] >= 3){
          // 高スコア
          if(originalPoint >= 5){
            if(hiddenCount <= 1){
              return -50.0;
            }else{
              // 最初のスコア計算
              if(turn == 5){
                if(hiddenCount >= 4){
                  return -10.0;
                }else{
                  return (hiddenCount <= 2)? -100.0 : -1.0;
                }
                // 最後のスコア計算
              }else{
                return (hiddenCount <= 2)? -10.0 : 1.0;
              }
            }
            // low point
          }else{
            if(highPointCount >= 3){
              return (hiddenCount == 0)? BAN : -200.0;
            }else{
              return (hiddenCount == 0)? BAN : -200.0;
            }
          }
          // 1pt以上割り振っている
        }else if(pointThisTurn[myId] >= 1){
          // 高スコア
          if(originalPoint >= 5){
            if(hiddenCount <= 1){
              return -50.0;
            }else{
              // 最初のスコア計算
              if(turn == 5){
                if(hiddenCount >= 4){
                  return -10.0;
                }else{
                  return (hiddenCount <= 2)? -100.0 : -1.0;
                }
                // 最後のスコア計算
              }else{
                return (hiddenCount <= 2)? -10.0 : 1.0;
              }
            }
            // 1O5D - 低スコア
          }else{
            return (hiddenCount == 0)? -200.0 : -50.0;
          }
          // 1ptも割り振っていない
        }else{
          if(originalPoint >= 5){
            return (hiddenCount <= 2)? 10.0 : -1.0;
          }else{
            return (hiddenCount <= 2)? 10.0 : 1.0;
          }
        }
        /*
         * 2位とのスコア差が4pt FSLD4
         */
      }else if(diff == 4){
        // 4pt以上割り振っている
        if(pointThisTurn[myId] >= 4){
          // 高スコア
          if(originalPoint >= 5){
            if(hiddenCount >= 3){
              return BAN;
            }else{
              return (hiddenCount <= 1)? 2.0 : -100.0;
            }
            // 低スコア
          }else{
            if(highPointCount >= 3){
              return (hiddenCount == 0)? -200.0 : BAN;
            }else{
              return (hiddenCount == 0)? -100.0 : BAN;
            }
          }
          // 3pt以上割り振っている
        }else if(pointThisTurn[myId] >= 3){
          // 高スコア
          if(originalPoint >= 5){
            return (hiddenCount <= 1)? 2.0 : -100.0;
            // 低スコア
          }else{
            if(highPointCount >= 3){
              return (hiddenCount == 0)? -40.0 : BAN;
            }else{
              return (hiddenCount == 0)? -20.0 : BAN;
            }
          }
          // 2pt割り振る
        }else if(pointThisTurn[myId] == 2){
          // high point
          if(originalPoint >= 5){
            if(hiddenCount >= 4){
              return -10.0;
            }else{
              return (hiddenCount <= 2)? 2.0 : 1.0;
            }
          // low point
          }else{
            if(highPointCount >= 3){
              return (hiddenCount == 0)? -5.0 : -400.0;
            }else{
              if(originalPoint == 4){
                return (hiddenCount == 0)? -4.0 : -200.0;
              }else{
                return (hiddenCount == 0)? -10.0 : -200.0;
              }
            }
          }
          // 1pt以上割り振っている
        }else if(pointThisTurn[myId] >= 1){
          // high point
          if(originalPoint >= 5){
            if(hiddenCount >= 4){
              return -10.0;
            }else{
              return (hiddenCount <= 2)? 2.0 : 1.0;
            }
            // low point
          }else{
            if(highPointCount >= 4){
              return (hiddenCount == 0)? -2.0 : -40.0;
            }else{
              if(originalPoint == 4){
                return (hiddenCount == 0)? -1.0 : -20.0;
              }else{
                return (hiddenCount == 0)? -2.0 : -40.0;
              }
            }
          }
          // 1ptも割り振っていない
        }else{
          // high point
          if(originalPoint >= 5){
            return (hiddenCount == 0)? 10.0 : -2.0;
            // low point
          }else{
            return (hiddenCount <= 1)? 10.0 : 0.0;
          }
        }
        /*
         * 2位とのスコア差が3pt FSLD3
         */
      }else if(diff == 3){
        // 5T
        if(turn == 5){
          // 5pt割り振っている
          if(pointThisTurn[myId] == 5){
            // 高ポイント
            if(originalPoint >= 5){
              return (hiddenCount == 0)? 1.0 : BAN;
              // 低ポイント
            }else{
              if(highPointCount >= 3){
                return BAN;
              }else{
                if(originalPoint == 4){
                  return (hiddenCount == 0)? -20.0 : BAN;
                }else{
                  return (hiddenCount == 0)? -100.0 : BAN;
                }
              }
            }
            // 4pt以上割り振っている
          }else if(pointThisTurn[myId] >= 4){
            // 高ポイント
            if(originalPoint >= 5){
              if(hiddenCount >= 3){
                return BAN;
              }else{
                return (hiddenCount <= 1)? 1.0 : -100.0;
              }
              // 低ポイント
            }else{
              if(highPointCount >= 3){
                return (hiddenCount == 0)? -20.0 : -400.0;
              }else{
                if(originalPoint == 4){
                  return (hiddenCount == 0)? -10.0 : BAN;
                }else{
                  return (hiddenCount == 0)? -100.0 : BAN;
                }
              }
            }
            // 3pt以上割り振っている
          }else if(pointThisTurn[myId] >= 3){
            // 高ポイント
            if(originalPoint >= 5){
              if(hiddenCount >= 4){
                return -1000.0;
              }else{
                return (hiddenCount <= 1)? 1.0 : -100.0;
              }
              // 低ポイント
            }else{
              if(highPointCount >= 3){
                return (hiddenCount == 0)? -1.0 : -400.0;
              }else{
                if(originalPoint == 4){
                  return (hiddenCount == 0)? 0.0 : -300.0;
                }else{
                  return (hiddenCount == 0)? -0.5 : -600.0;
                }
              }
            }
            // 2pt割り振る
          }else if(pointThisTurn[myId] == 2){
            // 高ポイント
            if(originalPoint >= 5){
              return (hiddenCount <= 1)? 3.0 : -100.0;
              // 低ポイント
            }else{
              if(highPointCount <= 2){
                if(originalPoint == 4){
                  return (hiddenCount == 0)? 0.0 : -200.0;
                }else{
                  return (hiddenCount == 0)? -1.0 : -200.0;
                }
              }else{
                if(originalPoint == 4){
                  return (hiddenCount == 0)? 0.5 : BAN;
                }else{
                  return (hiddenCount == 0)? 0.0 : BAN;
                }
              }
            }
            // 1pt以上割り振っている
          }else if(pointThisTurn[myId] >= 1){
            // 高ポイント
            if(originalPoint >= 5){
              return (hiddenCount <= 1)? 3.0 : -100.0;
              // 低ポイント
            }else{
              if(highPointCount <= 2){
                return (hiddenCount <= 1)? 0.0 : -10.0;
              }else{
                return (hiddenCount <= 1)? 0.5 : -1.0;
              }
            }
            // 1ptも割り振っていない
          }else{
            // 高ポイント
            if(originalPoint >= 5){
              if(turn == 5){
                return (hiddenCount <= 0)? -1.0 : -2.0;
              }else{
                return (hiddenCount <= 0)? 1.0 : -1.0;
              }
              // 低ポイント
            }else{
              if(originalPoint == 4){
                return (hiddenCount == 0)? 2.0 : 0.0;
              }else{
                return (hiddenCount == 0)? 20.0 : 2.0;
              }
            }
          }
          // 9T FSLD3T9
        }else{
          // 5pt割り振る
          if(pointThisTurn[myId] == 5){
            if(originalPoint >= 5){
              return (hiddenCount <= 1)? 1.0 : -100.0;
            }else{
              return BAN;
            }
            // 3pt以上割り振っている
          }else if(pointThisTurn[myId] >= 3){
            if(originalPoint >= 5){
              return (hiddenCount == 0)? 1.0 : -100.0;
            }else{
              return (hiddenCount == 0)? -1.0 : -200.0;
            }
            // 2pt割り振る
          }else if(pointThisTurn[myId] == 2){
            // 高ポイント
            if(originalPoint >= 5){
              return (hiddenCount == 0)? 3.0 : -100.0;
              // 低ポイント
            }else{
              if(highPointCount <= 2){
                return (hiddenCount == 0)? 0.0 : -10.0;
              }else{
                return (hiddenCount == 0)? -10.0 : -100.0;
              }
            }
            // 1pt以上割り振っている
          }else if(pointThisTurn[myId] >= 1){
            // 高ポイント
            if(originalPoint >= 5){
              return (hiddenCount == 0)? 3.0 : -100.0;
              // 低ポイント
            }else{
              if(highPointCount <= 2){
                return (hiddenCount <= 1)? 0.0 : -1.0;
              }else{
                return (hiddenCount <= 1)? 0.0 : 1.0;
              }
            }
            // 1ptも割り振っていない
          }else{
            // 高ポイント
            if(originalPoint >= 5){
              if(turn == 5){
                return (hiddenCount <= 0)? -1.0 : -2.0;
              }else{
                return (hiddenCount <= 0)? 1.0 : -1.0;
              }
              // 低ポイント
            }else{
              if(originalPoint == 4){
                return (hiddenCount == 0)? 2.0 : 1.0;
              }else{
                return (hiddenCount == 0)? 15.0 : 1.0;
              }
            }
          }
        }
        /*
         * 2位とのスコア差が2pt FSLD2
         */
      }else if(diff == 2){
        // 5pt以上割り振っている
        if(pointThisTurn[myId] >= 5){
          // 高ポイント
          if(originalPoint >= 5){
            if(playerRank[myId] != 1){
              if(highPointCount <= 2){
                return (hiddenCount == 0)? -10.0 : BAN;
              }else{
                return (hiddenCount == 0)? -10.0 : BAN;
              }
            }else{
              return (hiddenCount == 0)? -20.0 : BAN;
            }
            // 低ポイント
          }else{
            if(originalPoint == 4){
              return (hiddenCount == 0)? -100.0 : BAN;
            }else{
              return (hiddenCount == 0)? -200.0 : BAN;
            }
          }
          // 4pt以上割り振っている
        }else if(pointThisTurn[myId] >= 4){
          // 高ポイント
          if(originalPoint >= 5){
            if(playerRank[myId] != 1){
              return (hiddenCount == 0)? -1.0 : -1600.0;
            }else{
              return (hiddenCount == 0)? -1.0 : -800.0;
            }
            // 低ポイント
          }else{
            if(highPointCount <= 2){
              return (hiddenCount == 0)? -50.0 : BAN;
            }else{
              return (hiddenCount == 0)? -300.0 : BAN;
            }
          }
          // 3pt以上割り振っている
        }else if(pointThisTurn[myId] >= 3){
          // 高ポイント
          if(originalPoint >= 5){
            if(playerRank[myId] != 1){
              if(hiddenCount >= 3){
                return BAN;
              }else{
                return (hiddenCount == 0)? 0.0 : -200.0;
              }
            }else{
              if(hiddenCount >= 3){
                return BAN;
              }else{
                return (hiddenCount == 0)? -2.0 : BAN;
              }
            }
            // 低ポイント
          }else{
            if(playerRank[myId] != 1){
              return (hiddenCount == 0)? -20.0 : -320.0;
            }else{
              return (hiddenCount == 0)? -10.0 : -160.0;
            }
          }
          // 2pt以上割り振っている
        }else if(pointThisTurn[myId] >= 2){
          // 高ポイント
          if(originalPoint >= 5){
            // 最初のスコア計算
            if(turn == 5){
              if(hiddenCount >= 3){
                return BAN;
              }else{
                return (hiddenCount <= 1)? 1.0 : -100.0;
              }
            }else{
              if(hiddenCount >= 3){
                return BAN;
              }else{
                return (hiddenCount <= 1)? 1.0 : -200.0;
              }
            }
            // 低ポイント
          }else{
            if(hiddenCount >= 2){
              return -200.0;
            }else{
              if(highPointCount >= 3){
                return (hiddenCount == 0)? -1.0 : -200.0;
              }else{
                return (hiddenCount <= 1)? -1.0 : -200.0;
              }
            }
          }
        // 1pt以上割り振っている
        }else if(pointThisTurn[myId] >= 1){
          // 高ポイント
          if(originalPoint >= 5){
            // 最初のスコア計算
            if(turn == 5){
              if(hiddenCount >= 3){
                return BAN;
              }else{
                return (hiddenCount == 0)? 1.0 : -10.0;
              }
            }else{
              if(hiddenCount >= 3){
                return BAN;
              }else{
                return (hiddenCount <= 1)? 1.0 : -20.0;
              }
            }
            // 低ポイント
          }else{
            return (hiddenCount <= 1)? 1.0 : -1.0;
          }
          // 1ptも割り振っていない
        }else{
          // 高ポイント
          if(originalPoint >= 5){
            if(turn == 5){
              return (hiddenCount == 0)? -1.0 : -10.0;
            }else{
              return (hiddenCount <= 1)? 1.0 : 0.5;
            }
            // 低ポイント
          }else{
            return (hiddenCount <= 1)? 1.0 : 0.5;
          }
        }
        /*
         * 2位とのスコア差が1pt FSLD1
         */
      }else if(diff == 1){
        // 5pt割り振る
        if(pointThisTurn[myId] >= 5){
          // 高スコア
          if(originalPoint >= 5){
            if(playerRank[myId] != 1){
              return (hiddenCount == 0)? -10.0 : BAN;
            }else{
              return (hiddenCount == 0)? -1.0 : -100.0;
            }
            // 低スコア
          }else{
            return BAN;
          }
          // 4pt割り振っている
        }else if(pointThisTurn[myId] == 4){
          // 高スコア
          if(originalPoint >= 5){
            if(playerRank[myId] != 1){
              if(hiddenCount >= 3){
                return BAN;
              }else{
                return (hiddenCount == 0)? -50.0 : BAN;
              }
            }else{
              return (hiddenCount == 0)? -1.0 : -10.0;
            }
            // 低スコア
          }else{
            if(hiddenCount == 0){
              if(highPointCount <= 2){
                if(originalPoint == 4){
                  return -500.0;
                }else{
                  return BAN;
                }
              }else{
                return BAN;
              }
            }else{
              return BAN;
            }
          }
          // 3pt以上割り振っている
        }else if(pointThisTurn[myId] >= 3){
          // 高スコア
          if(originalPoint >= 5){
            if(playerRank[myId] != 1){
              if(hiddenCount >= 3){
                return BAN;
              }else{
                return (hiddenCount <= 1)? -10.0 : -1000.0;
              }
            }else{
              return (hiddenCount <= 1)? -1.0 : -10.0;
            }
            // 低スコア
          }else{
            if(hiddenCount <= 1){
              if(highPointCount <= 2){
                return -50.0;
              }else{
                return -100.0;
              }
            }else{
              return -1000.0;
            }
          }
          // 2pt以上割り振っている
        }else if(pointThisTurn[myId] >= 2){
          // 高ポイント
          if(originalPoint >= 5){
            return (hiddenCount == 0)? 0.0 : BAN;
            // 低ポイント
          }else{
            if(highPointCount <= 2){
              return (hiddenCount == 0)? 0.0 : -2000.0;
            }else{
              return (hiddenCount == 0)? -10.0 : BAN;
            }
          }
          // 1pt以上割り振っている
        }else if(pointThisTurn[myId] >= 1){
          // 高ポイント
          if(originalPoint >= 5){
            if(hiddenCount >= 3){
              return BAN;
            }else{
              return (hiddenCount == 0)? 1.0 : -100.0;
            }
            // 低ポイント
          }else{
            if(hiddenCount >= 3){
              return BAN;
            }else{
              return (hiddenCount == 0)? 2.0 : -200.0;
            }
          }
          // 0pt
        }else{
          if(pointThisTurn[myId] >= 1){
            return (hiddenCount <= 2)? 0.7 : 0.3;
          }else{
            return (hiddenCount <= 2)? 0.5 : 0.1;
          }
        }
        /*
         * 2位とのスコア差が0pt FSLD0
         */
      }else{
        // 4pt以上割り振る
        if(pointThisTurn[myId] >= 4){
          // 高ポイント
          if(originalPoint >= 5){
            return (hiddenCount == 0)? -1000.0 : BAN;
            // 低ポイント
          }else{
            return BAN;
          }
          // 3pt割り振る
        }else if(pointThisTurn[myId] == 3){
          // 高ポイント
          if(originalPoint >= 5){
            return (hiddenCount == 0)? -1000.0 : BAN;
            // 低ポイント
          }else{
            return (hiddenCount == 0)? -2000.0 : BAN;
          }
          // 2pt割り振る
        }else if(pointThisTurn[myId] >= 2){
          // 高スコア
          if(originalPoint >= 5){
            if(playerRank[myId] != 1){
              if(hiddenCount >= 3){
                return BAN;
              }else{
                return (hiddenCount == 0)? -300.0 : -1000.0;
              }
            }else{
              return (hiddenCount == 0)? 0.0 : -200.0;
            }
            // 低スコア
          }else{
            // 高スコアが少ない
            if(highPointCount <= 2){
              return (hiddenCount == 2)? -100.0 : -200.0;
            }else{
              return (hiddenCount == 0)? -200.0 : -400.0;
            }
          }
          // 1pt割り振る
        }else if(pointThisTurn[myId] >= 1){
          // 高スコア
          if(originalPoint >= 5){
            if(hiddenCount >= 3){
              return BAN;
            }else{
              return (hiddenCount == 0)? 0.0 : -200.0;
            }
            // 低スコア
          }else{
            // 高スコアが少ない
            if(highPointCount <= 2){
              return (hiddenCount == 0)? -10.0 : -200.0;
            }else{
              return (hiddenCount == 0)? 0.0 : -100.0;
            }
          }
          // 1ptも割り振らない
        }else{
          if(originalPoint == 3){
            return (hiddenCount >= 1)? 0.0 : -1.0;
          }else{
            return (hiddenCount >= 1)? -10.0 : -1.0;
          }
        }
      }
    }

    /*
     * 自分が最下位の場合に使用する評価関数
     * 高いほど悪い
     */
    double fuzzyWorst(int diff){
      int sumPoint = enemyTotalPoint();
      /*
       * 3位とのpt差が4以上 FWD4
       */
      if(diff >= 4){
        if(pointThisTurn[myId] >= 1){
          return -BAN;
        }else{
          return 0.0;
        }
        // そこまで無理しないでも良い
        /*
         * 3位とのpt差が3 FWD3
         */
      }else if(diff == 3){
        // 3 or 7 turn
        if(turn % 4 == 3){
          // 2pt以上割り振る
          if(pointThisTurn[myId] >= 2){
            // high point
            if(originalPoint >= 5){
              return (hiddenCount == 0)? 0.2 : 200.0;
              // low point
            }else{
              return (hiddenCount == 0)? 0.4 : 400.0;
            }
          }else{
            // high point
            if(originalPoint >= 5){
              return (hiddenCount == 0)? 20.0 : 200.0;
              // low point
            }else{
              return (hiddenCount == 0)? 10.0 : 100.0;
            }
          }
        }else if(turn == 8){
          // 2pt以上を割り振る
          if(pointThisTurn[myId] >= 2){
            return 100.0;
          }else{
            return (hiddenCount <= 2)? 0.2 : 0.0;
          }
        }else if(turn == 6){
          if(pointThisTurn[myId] == 4){
            return -BAN;
            // 2pt以上を割り振る
          }else if(pointThisTurn[myId] >= 2){
            if(originalPoint >= 5){
              return 1000.0;
            }else{
              return -BAN;
            }
            // 何も割り振らない
          }else{
            return 0.2;
          }
          // 2 or 4 or 8
        }else{
          // 2pt以上を割り振る
          if(pointThisTurn[myId] >= 2){
            return (hiddenCount == 0)? 1.0 : 10.0;
          }else if(pointThisTurn[myId] >= 1){
            return (hiddenCount == 0)? 1.0 : 10.0;
            // 何も割り振らない
          }else{
            return (hiddenCount <= 2)? 0.2 : 0.0;
          }
        }
        // 隠しパラメータ1発同点範囲
        /*
         * 3位とのpt差が2pt FWD2
         */
      }else if(diff >= 2){
        // 7 turn
        if(turn == 7){
          // 1pt以上割り振る
          if(pointThisTurn[myId] >= 1){
            // high point
            if(originalPoint >= 5){
              return (hiddenCount == 0)? 40.0 : 160.0;
              // low point
            }else{
              return (hiddenCount == 0)? 80.0 : 320.0;
            }
          }else{
            // high point
            if(originalPoint >= 5){
              return (hiddenCount <= 1)? 2.5 : 10.0;
              // low point
            }else{
              return (hiddenCount <= 1)? 5.0 : 20.0;
            }
          }
          // 3 turn FWD2T3
        }else if(turn == 3){
          // high point
          if(originalPoint >= 5){
            return (hiddenCount <= 1)? 1.5 : 10.0;
            // low point
          }else{
            return (hiddenCount <= 1)? 3.0 : 20.0;
          }
        // 6 turn FWD2T6
        }else if(turn == 6){
          // 1pt以上割り振る
          if(pointThisTurn[myId] >= 1){
            if(originalPoint >= 5){
              return (hiddenCount == 0)? 50.0 : -BAN;
            }else{
              return (hiddenCount == 0)? 100.0 : -BAN;
            }
          // 0pt
          }else{
            if(originalPoint >= 5){
              return (sumPoint < 15)? 10.0 : 100.0;
            }else{
              return (sumPoint <= 10)? 20.0 : 200.0;
            }
          }
        // 8 turn
        }else if(turn == 8){
          // 2pt以上割り振る
          if(pointThisTurn[myId] >= 2){
            return -BAN;
          }else{
            if(originalPoint >= 5){
              return (hiddenCount <= 1)? 0.5 : 10.0;
            }else{
              return (hiddenCount <= 1)? 0.5 : 20.0;
            }
          }
          // 2 or 4
        }else{
          // 2pt以上割り振る
          if(pointThisTurn[myId] >= 2){
            return -BAN;
          }else{
            if(originalPoint >= 5){
              return (hiddenCount <= 1)? 0.5 : 10.0;
            }else{
              return (hiddenCount <= 1)? 0.5 : 20.0;
            }
          }
        }
        // 隠しパラメータ1発逆転
        /*
         * 3位とのpt差が1pt FWD1
         */
      }else if(diff == 1){
        if(turn == 8){
          // 2pt以上割り振る
          if(pointThisTurn[myId] >= 2){
            if(originalPoint >= 5){
              return (sumPoint >= 15)? 1000.0 : 10;
            }else{
              return (sumPoint >= 10)? -BAN : 100;
            }
            // 何も割り振らない
          }else{
            if(originalPoint >= 5){
              return (sumPoint >= 15)? 100.0 : 10;
            }else{
              return (sumPoint >= 10)? 200.0 : 100;
            }
          }
          // 6 turn
        }else if(turn == 6){
          if(pointThisTurn[myId] >= 2){
            if(originalPoint >= 5){
              return (sumPoint >= 15)? 1000.0 : 10;
            }else{
              return (sumPoint >= 10)? 2000.0 : 100;
            }
            // 何も割り振らない
          }else{
            if(originalPoint >= 5){
              return 10;
            }else{
              return 100;
            }
          }
          // 7 turn
        }else if(turn == 7){
          // 1pt以上割り振る
          if(pointThisTurn[myId] >= 1){
            if(originalPoint >= 5){
              return 200.0;
            }else{
              return 400.0;
            }
            // 何も割り振らない
          }else{
            if(originalPoint >= 5){
              return 10.0;
            }else{
              return 20.0;
            }
          }
        }else{
          if(pointThisTurn[myId] >= 2){
            // high point
            if(originalPoint >= 5){
              return 20.0;
              // low point
            }else{
              return 40.0;
            }
          }else{
            // high point
            if(originalPoint >= 5){
              return 20.0;
              // low point
            }else{
              return 40.0;
            }
          }
        }
        /*
         * 3位とのpt差が0 FWD0
         */
      }else{
        // 6 turn
        if(turn == 6){
          // 2pt以上割り振っている
          if(pointThisTurn[myId] >= 2){
            // 高スコア
            if(originalPoint >= 5){
              if(highPointCount >= 3){
                return 500.0;
              }else{
                return 400.0;
              }
              // 低スコア
            }else{
              if(highPointCount >= 3){
                return 1000.0;
              }else{
                if(originalPoint == 4){
                  return 1000.0;
                }else{
                  return 500.0;
                }
              }
            }
            // 1ptも割り振っていない
          }else{
            return (hiddenCount >= 2)? 5.0 : 1.0;
          }
          // 4 turn
        }else if(turn == 4){
          // 1pt以上割り振っている
          if(pointThisTurn[myId] >= 2){
            // high point
            if(originalPoint >= 5){
              return (hiddenCount == 0)? 10.0 : 100.0;
              // low point
            }else{
              if(highPointCount <= 3){
                return (hiddenCount == 0)? 5.0 : 10.0;
              }else{
                return (hiddenCount == 0)? 5.0 : 10.0;
              }
            }
            // 1ptも割り振らない
          }else{
            return 0.5;
          }
          // 7 turn
        }else if(turn == 7){
          // 3pt割り振っている
          if(pointThisTurn[myId] >= 3){
            // high point
            if(originalPoint >= 5){
              return (hiddenCount == 0)? 100.0 : -BAN;
              // low point
            }else{
              return (hiddenCount == 0)? 400.0 : -BAN;
            }
            // 1pt以上割り振っている
          }else if(pointThisTurn[myId] >= 1){
            // high point
            if(originalPoint >= 5){
              return (hiddenCount == 0)? 10.0 : 100.0;
              // low point
            }else{
              return (hiddenCount == 0)? 20.0 : 200.0;
            }
            // 1ptも割り振らない
          }else{
            return 0.5;
          }
          // その他のターン
        }else{
          // 1pt以上割り振っている
          if(pointThisTurn[myId] >= 1){
            // high point
            if(originalPoint >= 5){
              return (hiddenCount == 0)? 10.0 : 100.0;
              // low point
            }else{
              return (hiddenCount == 0)? 20.0 : 200.0;
            }
            // 1ptも割り振らない
          }else{
            return 0.5;
          }
        }
      }
    }

    /*
     * 点数が高いほど悪い評価
     */
    double fuzzyNormal(int diff, int topDiff){
      int sumPoint = enemyTotalPoint();

      /*
       * 最下位との差が5pt FND5
       */
      if(diff >= 5){
        // 7 turn FND5T7
        if(turn == 7){
          // 1pt以上割り振る
          if(pointThisTurn[myId] >= 1){
            // 高ポイント
            if(originalPoint >= 5){
              return (hiddenCount <= 2)? 10.0 : 50.0;
              // 低ポイント
            }else{
              return (hiddenCount <= 2)? 20.0 : 100.0;
            }
            // 0pt
          }else{
            // 高ポイント
            if(originalPoint >= 5){
              return (hiddenCount <= 1)? 0.0 : 50.0;
              // 低ポイント
            }else{
              return (hiddenCount <= 1)? 0.0 : 100.0;
            }
          }
          // 3 turn FND5T3
        }else if(turn == 3){
          // high point
          if(originalPoint >= 5){
            return (hiddenCount <= 2)? 0.0 : 0.15;
            // low point
          }else{
            return (hiddenCount <= 2)? 0.0 : 0.15;
          }
          // 8 turn FND5T8
        }else if(turn == 8){
          // 4pt割り振る
          if(pointThisTurn[myId] == 4){
            // 高スコア
            if(originalPoint >= 5){
              if(diff >= 7){
                return -BAN;
              }else if(diff >= 6 && hiddenCount <= 1){
                return -BAN;
              }else if(highPointCount >= 3){
                return (hiddenCount <= 1)? 1.0 : 20.0;
              }else{
                return (hiddenCount <= 1)? 1.0 : 20.0;
              }
              // 低スコア
            }else{
              return (hiddenCount <= 2)? 10.0 : 20.0;
            }
            // 2pt割り振る
          }else if(pointThisTurn[myId] == 2){
            // 高スコア
            if(originalPoint >= 5){
              if(topDiff >= 5){
                return (hiddenCount <= 2)? 100.0 : 10.0;
              }else{
                return (hiddenCount == 0)? 0.0 : 100.0;
              }
              // 低スコア
            }else{
              return (hiddenCount <= 2)? 10.0 : 5.0;
            }
            // 0pt
          }else{
            // 高スコア
            if(originalPoint >= 5){
              return (hiddenCount == 0)? -5.0 : 0.15;
              // 低スコア
            }else{
              return (hiddenCount == 0)? -10.0 : 0.15;
            }
          }
          // 6 turn
        }else if(turn == 6){
          // 4pt割り振る
          if(pointThisTurn[myId] == 4){
            return -BAN;
            // 2pt割り振る
          }else if(pointThisTurn[myId] == 2){
            // 高スコア
            if(originalPoint >= 5){
              return 100.0;
              // 低スコア
            }else{
              if(originalPoint >= 5){
                return 0.0;
              }else{
                return -BAN;
              }
            }
          }else{
            // 高スコア
            if(originalPoint >= 5){
              return 0.0;
              // 低スコア
            }else{
              return 0.0;
            }
          }
          // 2 or 4
        }else{
          // 4pt割り振る
          if(pointThisTurn[myId] == 4){
            // 高スコア
            if(originalPoint >= 5){
              return (hiddenCount <= 2)? 0.0 : 2.0;
              // 低スコア
            }else{
              return (hiddenCount <= 2)? 10.0 : 20.0;
            }
            // 2pt割り振る
          }else if(pointThisTurn[myId] == 2){
            // 高スコア
            if(originalPoint >= 5){
              if(topDiff >= 5){
                return (hiddenCount <= 2)? 100.0 : 10;
              }else{
                return (hiddenCount <= 2)? 0.0 : 0.15;
              }
              // 低スコア
            }else{
              return (hiddenCount <= 2)? 0.0 : 0.15;
            }
          }else{
            // 高スコア
            if(originalPoint >= 5){
              return (hiddenCount <= 2)? 0.0 : 0.15;
              // 低スコア
            }else{
              return (hiddenCount <= 2)? 0.0 : 0.15;
            }
          }
        }
        /*
         * 最下位との差が4pt FND4
         */
      }else if(diff >= 4){
        // 7T
        if(turn == 7){
          // 1pt以上割り振る
          if(pointThisTurn[myId] >= 1){
            // 高スコア
            if(originalPoint >= 5){
              if(topPoint - point[myId] >= 1){
                return (hiddenCount <= 1)? 5.0 : 100;
              }else{
                return (hiddenCount <= 1)? 0.0 : 0.25;
              }
              // 低スコア
            }else{
              return (hiddenCount <= 2)? 10.0 : 0.25;
            }
            // 1ptも割り振らない
          }else{
            if(originalPoint >= 5){
              return (hiddenCount == 0)? -1.0 : 0.5;
            }else{
              return (hiddenCount == 0)? -2.0 : 0.25;
            }
          }
          // 6 turn FND4T6
        }else if(turn == 6){
          // 4pt割り振る
          if(pointThisTurn[myId] == 4){
            // 高ポイント
            if(originalPoint >= 5){
              if(highPointCount >= 3){
                return 300.0;
              }else{
                return 200.0;
              }
              // 低ポイント
            }else{
              return 400.0;
            }
            // 2pt割り振る
          }else if(pointThisTurn[myId] == 2){
            // 高ポイント
            if(originalPoint >= 5){
              if(highPointCount <= 3){
                return (hiddenCount <= 1)? 0.0 : 1.0;
              }else{
                return (hiddenCount <= 1)? 1.0 : 10.0;
              }
              // 低ポイント
            }else{
              if(highPointCount <= 2){
                if(originalPoint == 4){
                  return 10.0;
                }else{
                  return 20.0;
                }
              }else{
                if(originalPoint == 4){
                  return 20.0;
                }else{
                  return 40.0;
                }
              }
            }
            // 1ptも割り振らない
          }else{
            if(originalPoint >= 5){
              return 0.0;
            }else{
              if(highPointCount >= 3){
                return -2.0;
              }else{
                return -1.0;
              }
            }
          }
          // 8 turn
        }else if(turn == 8){
          // 4pt割り振る
          if(pointThisTurn[myId] == 4){
            // 高ポイント
            if(originalPoint >= 5){
              if(topPoint - point[myId] >= 1){
                return 500.0;
              }else{
                return 100.0;
              }
              // 低ポイント
            }else{
              if(topPoint - point[myId] >= 1){
                return 1000.0;
              }else{
                return 400.0;
              }
            }
            // 2pt割り振る
          }else if(pointThisTurn[myId] == 2){
            // high point
            if(originalPoint >= 5){
              if(highPointCount <= 2){
                return (hiddenCount <= 1)? 0.0 : 1.25;
              }else{
                return (hiddenCount <= 1)? 0.0 : 1.25;
              }
              // low point
            }else{
              if(highPointCount <= 2){
                return (hiddenCount == 0)? 5.0 : 50.0;
              }else{
                return (hiddenCount == 0)? 10.0 : 100.0;
              }
            }
            // 0pt
          }else{
            // high point
            if(originalPoint >= 5){
              return (hiddenCount <= 1)? -1.0 : 0.25;
              // low point
            }else{
              return (hiddenCount <= 1)? -2.0 : 0.25;
            }
          }
        }else if(turn == 4){
          if(pointThisTurn[myId] == 4){
            // high point
            if(originalPoint >= 5){
              if(sumPoint - point[myId] >= 10){
                return -BAN;
              }else{
                return (hiddenCount <= 1)? 0.0 : 1.25;
              }
              // low point
            }else{
              return (hiddenCount == 0)? 10.0 : 100.0;
            }
          }else{
            return (hiddenCount == 0)? 0.0 : 1.25;
          }
        }else{
          return (hiddenCount <= 2)? 0.0 : 0.25;
        }
        /*
         * 最下位との差が3pt FND3
         */
      }else if(diff >= 3){
        // 3 turn
        if(turn == 3){
          // 3pt以上割り振る
          if(pointThisTurn[myId] >= 3){
            if(originalPoint >= 5){
              return (hiddenCount == 0)? 1.0 : 10.0;
            }else{
              return (hiddenCount == 0)? 5.0 : 50.0;
            }
            // 1pt以上割り振る
          }else if(pointThisTurn[myId] >= 1){
            if(originalPoint >= 5){
              return (hiddenCount == 0)? 0.0 : 1.0;
            }else{
              return (hiddenCount == 0)? 0.0 : 2.0;
            }
            // 何も割り振らない
          }else{
            return (hiddenCount <= 2)? 0.0 : 0.35;
          }
          // 7 turn
        }else if(turn == 7){
          if(originalPoint >= 5){
            if(sumPoint - point[myId] >= 15){
              return (hiddenCount == 0)? 10.0 : 100.0;
            }else{
              return (hiddenCount <= 2)? 5.0 : 50.0;
            }
          }else{
            if(sumPoint - point[myId] >= 15){
              return (hiddenCount == 0)? 20.0 : 200.0;
            }else{
              return (hiddenCount == 0)? 1.0 : 20.0;
            }
          }
          // 6T FND3T6
        }else if(turn == 6){
          // 4pt割り振る
          if(pointThisTurn[myId] == 4){
            // 高ポイント
            if(originalPoint >= 5){
              if(topPoint - point[myId] >= 1){
                return -BAN;
              }else{
                return 1.0;
              }
              // 低ポイント
            }else{
              if(highPointCount >= 3){
                if(originalPoint == 4){
                  return 200.0;
                }else{
                  return 250.0;
                }
              }else{
                if(originalPoint == 4){
                  return 100.0;
                }else{
                  return 150.0;
                }
              }
            }
            // 2pt割り振る
          }else if(pointThisTurn[myId] == 2){
            // 高スコア
            if(originalPoint >= 5){
              return 1.0;
              // 低スコア
            }else{
              if(highPointCount >= 3){
                return 10.0;
              }else{
                return 2.0;
              }
            }
            // 何もptを振らない
          }else{
            // 高ポイント
            if(originalPoint >= 5){
              return (hiddenCount <= 1)? 0.0 : 1.0;
              // 低ポイント
            }else{
              return (hiddenCount == 0)? 0.5 : 1.0;
            }
          }
          // 8T FND3T8
        }else if(turn == 8){
          // 4pt割り振る
          if(pointThisTurn[myId] == 4){
            // 高ポイント
            if(originalPoint >= 5){
              return (hiddenCount == 0)? 10.0 : -BAN;
              // 低ポイント
            }else{
              return (hiddenCount == 0)? 100.0 : -BAN;
            }
            // 2pt割り振る
          }else if(pointThisTurn[myId] == 2){
            // 高スコア
            if(originalPoint >= 5){
              return (hiddenCount == 0)? 1.0 : 10.0;
              // 低スコア
            }else{
              return (hiddenCount == 0)? 2.0 : 20.0;
            }
            // 何もptを振らない
          }else{
            // 高ポイント
            if(originalPoint >= 5){
              return (hiddenCount <= 1)? 0.0 : 1.0;
              // 低ポイント
            }else{
              return (hiddenCount == 0)? 0.5 : 1.0;
            }
          }
          // 4 turn
        }else if(turn == 4){
          // 4pt割り振る
          if(pointThisTurn[myId] == 4){
            // high point
            if(originalPoint >= 5){
              return (hiddenCount == 0)? 1.0 : 30.0;
              // low point
            }else{
              return (hiddenCount == 0)? 0.0 : 50.0;
            }
            // 2pt割り振る
          }else if(pointThisTurn[myId] == 2){
            // 高スコア
            if(originalPoint >= 5){
              return (hiddenCount == 0)? 1.0 : 2.5;
              // 低スコア
            }else{
              return (hiddenCount == 0)? 2.0 : 5.0;
            }
            // 何もptを振らない
          }else{
            // 高ポイント
            if(originalPoint >= 5){
              return (hiddenCount <= 1)? 0.0 : 1.0;
              // 低ポイント
            }else{
              return (hiddenCount == 0)? 0.5 : 1.0;
            }
          }
          // 2 
        }else{
          // 4pt割り振る
          if(pointThisTurn[myId] == 4){
            if(originalPoint >= 5){
              if(highPointCount >= 3){
                return 200.0;
              }else{
                return 50.0;
              }
            }else{
              return 100.0;
            }
            // 2pt割り振る
          }else if(pointThisTurn[myId] == 2){
            // 高スコア
            if(originalPoint >= 5){
              return 1.0;
              // 低スコア
            }else{
              return 2.0;
            }
            // 何もptを振らない
          }else{
            // 高ポイント
            if(originalPoint >= 5){
              return (hiddenCount <= 1)? 0.0 : 1.0;
              // 低ポイント
            }else{
              return (hiddenCount == 0)? 0.5 : 1.0;
            }
          }
        }
        /*
         * 最下位との差が2pt FND2
         */
      }else if(diff >= 2){
        // 7 turn FND2T7
        if(turn == 7){
          // 5pt割り振る
          if(pointThisTurn[myId] == 5){
            // 高ポイント
            if(originalPoint >= 5){
              return (hiddenCount == 0)? 50.0 : -BAN;
              // 低ポイント
            }else{
              return (hiddenCount == 0)? 100.0 : -BAN;
            }
            // 3pt以上割り振っている
          }else if(pointThisTurn[myId] >= 3){
            // 高ポイント
            if(originalPoint >= 5){
              return (hiddenCount == 0)? 50.0 : 300.0;
              // 低ポイント
            }else{
              return (hiddenCount == 0)? 100.0 : 800.0;
            }
            // 2pt以上割り振っている
          }else if(pointThisTurn[myId] >= 2){
            if(sumPoint - point[myId] >= 20){
              return (hiddenCount <= 1)? 20.0 : 50.0;
            }else{
              return (hiddenCount <= 1)? 0.0 : 0.9;
            }
          }else if(pointThisTurn[myId] >= 1){
            if(sumPoint - point[myId] >= 15){
              // 高ポイント
              if(originalPoint >= 5){
                return (hiddenCount <= 1)? 20.0 : 50.0;
                // 低ポイント
              }else{
                return (hiddenCount <= 1)? 20.0 : 50.0;
              }
            }else{
              return (hiddenCount <= 1)? 0.0 : 0.9;
            }
          }else{
            if(sumPoint - point[myId] >= 20){
              return (hiddenCount <= 1)? 20.0 : 50.0;
            }else{
              return (hiddenCount <= 1)? 0.0 : 0.9;
            }
          }
          // 3 turn FND3T2
        }else if(turn == 3){
          // high point
          if(originalPoint >= 5){
            if(sumPoint >= 10){
              return (hiddenCount == 0)? 10.0 : 200.0;
            }else{
              if(highPointCount <= 2){
                return (hiddenCount == 0)? 5.0 : 100.0;
              }else{
                return (hiddenCount == 0)? 5.0 : 200.0;
              }
            }
          // low point
          }else{
            if(sumPoint >= 10){
              return (hiddenCount == 0)? 20.0 : -BAN;
            }else{
              return (hiddenCount == 0)? 8.0 : 150.0;
            }
          }
          // 4 turn
        }else if(turn == 4){
          if(pointThisTurn[myId] == 4){
            if(originalPoint >= 5){
              if(sumPoint - point[myId] >= 10){
                return (hiddenCount == 0)? 20.0 : 100.0;
              }else{
                return (hiddenCount == 0)? 10.0 : 50.0;
              }
            }else{
              if(sumPoint - point[myId] >= 10){
                return 1000.0;
              }else{
                return (hiddenCount <= 1)? 2.0 : 10.0;
              }
            }
            // 2pt以上割り振る
          }else if(pointThisTurn[myId] >= 2){
            if(originalPoint >= 5){
              if(sumPoint - point[myId] >= 10){
                return (hiddenCount <= 1)? 2.0 : 100.0;
              }else{
                return (hiddenCount <= 1)? 0.0 : 1.0;
              }
            }else{
              if(sumPoint - point[myId] >= 10){
                return 1000.0;
              }else{
                return (hiddenCount <= 1)? 2.0 : 10.0;
              }
            }
            // 1ptも割り振らない
          }else{
            return (hiddenCount <= 1)? 0.0 : 0.9;
          }
          // 6 turn FND2T6
        }else if(turn == 6){
          // 4pt割り振る
          if(pointThisTurn[myId] == 4){
            // 高ポイント
            if(originalPoint >= 5){
              if(highPointCount <= 2){
                return 200.0;
              }else{
                if(playerRank[myId] == 4){
                  return 200.0;
                }else{
                  return 100.0;
                }
              }
              // 低ポイント
            }else{
              if(highPointCount <= 2){
                return 400.0;
              }else{
                return 800.0;
              }
            }
            // 2pt以上割り振る
          }else if(pointThisTurn[myId] >= 2){
            // high point
            if(originalPoint >= 5){
              if(sumPoint >= 15){
                return 2.0;
              }else{
                return 0.0;
              }
              // low point
            }else{
              if(originalPoint == 4){
                return 2.0;
              }else{
                if(highPointCount >= 3){
                  return 8.0;
                }else{
                  return 4.0;
                }
              }
            }
            // 1ptも割り振らない
          }else{
            return (hiddenCount <= 1)? 0.0 : 0.9;
          }
          // 8 turn FND2T8
        }else if(turn == 8){
          // 4pt割り振る
          if(pointThisTurn[myId] == 4){
            // high point
            if(originalPoint >= 5){
              if(turn == 4 && sumPoint >= 15){
                return (hiddenCount <= 1)? 20.0 : 100.0;
              }else{
                return (hiddenCount <= 1)? 10.0 : 100.0;
              }
              // low piont
            }else{
              return (hiddenCount <= 1)? 20.0 : 100.0;
            }
            // 2pt以上割り振る
          }else if(pointThisTurn[myId] >= 2){
            if(originalPoint >= 5){
              if(turn == 4 && sumPoint >= 15){
                return (hiddenCount <= 1)? 2.0 : 10.0;
              }else{
                return (hiddenCount <= 1)? 0.0 : 1.0;
              }
            }else{
              return (hiddenCount <= 1)? 2.0 : 10.0;
            }
            // 1ptも割り振らない
          }else{
            return (hiddenCount <= 1)? 0.0 : 0.9;
          }
          // 2 or 8
        }else{
          // 2pt以上割り振る
          if(pointThisTurn[myId] >= 2){
            if(originalPoint >= 5){
              if(turn == 4 && sumPoint >= 15){
                return (hiddenCount <= 1)? 2.0 : 10.0;
              }else{
                return (hiddenCount <= 1)? 0.0 : 30.0;
              }
            }else{
              return (hiddenCount <= 1)? 2.0 : 10.0;
            }
            // 1ptも割り振らない
          }else{
            return (hiddenCount <= 1)? 0.0 : 0.9;
          }
        }
        /*
         * 最下位との差が1pt FND1
         */
      }else if(diff >= 1){
        // 3 turn
        if(turn == 3){
          if(pointThisTurn[myId] >= 2){
            if(originalPoint >= 5){
              return (hiddenCount == 0)? 1.0 : 50.0;
            }else{
              return (hiddenCount == 0)? 0.0 : 20.0;
            }
            // 1pt以上割り振る
          }else if(pointThisTurn[myId] >= 1){
            if(sumPoint - point[myId] >= 10){
              return (hiddenCount == 0)? 1.0 : 50.0;
            }else{
              return (hiddenCount == 0)? 0.0 : 20.0;
            }
            // 1ptも割り振らない
          }else{
            if(sumPoint - point[myId] >= 10){
              return (hiddenCount == 0)? 0.0 : 1.0;
            }else{
              return (hiddenCount == 0)? 0.0 : 1.0;
            }
          }
          // 4 turn FND1T4
        }else if(turn == 4){
          // 2pt以上振っている
          if(pointThisTurn[myId] >= 2){
            // 高ポイント
            if(originalPoint >= 5){
              if(hiddenCount >= 3){
                return -BAN;
              }else{
                return (hiddenCount == 0)? 2.0 : 100.0;
              }
              // 低ポイント
            }else{
              return (hiddenCount == 0)? 10.0 : 400.0;
            }
            // 何も振っていない
          }else{
            return (hiddenCount == 0)? 0.0 : 1.0;
          }
          // 2 turn
        }else if(turn == 2){
          // 2pt以上割り振っている
          if(pointThisTurn[myId] >= 2){
            // 高ポイント
            if(originalPoint >= 5){
              if(sumPoint - point[myId] >= 5){
                return 100.0;
              }else if(sumPoint - point[myId] >= 4){
                return 30.0;
              }else{
                return 0.0;
              }
              // 低ポイント
            }else{
              if(sumPoint - point[myId] >= 2){
                return 20.0;
              }else{
                return 1.0;
              }
            }
            // 1ptも割り振っていない
          }else{
            return (hiddenCount == 0)? 0.0 : 1.0;
          }
          // 6 turn FND1T6
        }else if(turn == 6){
          // 4pt割り振る
          if(pointThisTurn[myId] == 4){
            // 高ポイント
            if(originalPoint >= 5){
              if(sumPoint - point[myId] >= 10){
                return 300.0;
              }else{
                if(topPoint - point[myId] >= 1){
                  return 200.0;
                }else{
                  return 0.0;
                }
              }
              // 低ポイント
            }else{
              if(highPointCount >= 3){
                return 200.0;
              }else{
                return 100.0;
              }
            }
            // 2pt以上割り振っている
          }else if(pointThisTurn[myId] >= 2){
            // 高ポイント
            if(originalPoint >= 5){
              if(sumPoint - point[myId] >= 12){
                return 400.0;
              }else{
                if(topPoint - point[myId] >= 1){
                  return 200.0;
                }else{
                  return 0.0;
                }
              }
              // 低ポイント
            }else{
              if(highPointCount >= 3){
                return 200.0;
              }else{
                return 10.0;
              }
            }
            // 1ptも割り振っていない
          }else{
            return (hiddenCount == 0)? 0.0 : 1.0;
          }
          // 7 turn
        }else if(turn == 7){
          if(pointThisTurn[myId] >= 4){
            // 高ポイント
            if(originalPoint >= 5){
              return (hiddenCount == 0)? 50.0 : -BAN;
              // 低ポイント
            }else{
              return (hiddenCount == 0)? 100.0 : -BAN;
            }
            // 2pt以上割り振っている
          }else if(pointThisTurn[myId] >= 2){
            // 高ポイント
            if(originalPoint >= 5){
              return (hiddenCount == 0)? 10.0 : -BAN;
              // 低ポイント
            }else{
              return (hiddenCount == 0)? 20.0 : -BAN;
            }
            // 1ptも割り振っていない
          }else{
            return (hiddenCount == 0)? 0.0 : 1.0;
          }
          // 8 turn
        }else{
          // 2pt以上割り振っている
          if(pointThisTurn[myId] >= 2){
            // 高ポイント
            if(originalPoint >= 5){
              return (hiddenCount == 0)? 5.0 : -BAN;
              // 低ポイント
            }else{
              return (hiddenCount == 0)? 10.0 : -BAN;
            }
            // 1ptも割り振っていない
          }else{
            return (hiddenCount == 0)? 0.0 : 1.0;
          }
        }
      }else{
        return 1.5;
      }
    }

    /*
     * 最下位との点差を考えてペナルティを考える
     * 高いほど悪い評価
     */
    double fuzzyNormalLast(int diff, int topDiff){
      int sumPoint = enemyTotalPoint();
      /*
       * 最下位との点差が5pt FNLD5
       */
      if(diff >= 5){
        // 1pt以上割り振る
        if(pointThisTurn[myId] >= 1){
          // high point
          if(originalPoint >= 5){
            return (hiddenCount <= 1)? 10.0 : 100.0;
            // low point
          }else{
            return (hiddenCount <= 1)? 100.0 : 50.0;
          }
          // 0pt
        }else{
          return (hiddenCount <= 1)? 1.0 : 10.0;
        }
        /*
         * 最下位との点差が4pt FNLD4
         */
      }else if(diff == 4){
        // 4pt以上割り当て
        if(pointThisTurn[myId] >= 4){
          // 高ポイント
          if(originalPoint >= 5){
            if(hiddenCount >= 3){
              return -BAN;
            }else{
              return (hiddenCount <= 1)? 2.0 : 200.0;
            }
            // 低ポイント
          }else{
            if(topDiff >= 5){
              return -BAN;
            }else{
              return (hiddenCount <= 1)? 40.0 : -BAN;
            }
          }
          // 1pt以上割り当て
        }else if(pointThisTurn[myId] >= 1){
          return (hiddenCount <= 4)? 10.0 : 100.0;
        }else{
          return (hiddenCount <= 4)? 0.0 : 10.0;
        }
        /*
         * 最下位との点差が3pt FNLD3
         */
      }else if(diff == 3){
        // 5 turn
        if(turn == 5){
          if(pointThisTurn[myId] == 5){
            // 高ポイント
            if(originalPoint >= 5){
              if(highPointCount >= 4){
                return (hiddenCount == 0)? 100.0 : 800.0;
              }else{
                return (hiddenCount == 0)? 50.0 : 400.0;
              }
              // 低ポイント
            }else{
              return (hiddenCount == 0)? 300.0 : -BAN;
            }
            // 3pt以上割り振る
          }else if(pointThisTurn[myId] >= 3){
            // 高ポイント
            if(originalPoint >= 5){
              if(highPointCount >= 4){
                return (hiddenCount == 0)? 2.0 : -BAN;
              }else{
                return (hiddenCount == 0)? 10.0 : -BAN;
              }
              // 低ポイント
            }else{
              return (hiddenCount <= 2)? 10.0 : 40.0;
            }
            // 2pt以上割り振る
          }else if(pointThisTurn[myId] >= 2){
            if(originalPoint >= 5){
              return (hiddenCount <= 2)? 2.0 : 200.0;
            }else{
              return (hiddenCount <= 2)? 5.0 : 400.0;
            }
            // 1pt以上割り振る
          }else if(pointThisTurn[myId] >= 1){
            return (hiddenCount <= 3)? 5.0 : 10.0;
          }else{
            return (hiddenCount <= 3)? 0.0 : 0.1;
          }
          // turn 9
        }else{
          if(pointThisTurn[myId] == 5){
            return -BAN;
            // 3pt以上割り振る
          }else if(pointThisTurn[myId] >= 3){
            // 高ポイント
            if(originalPoint >= 5){
              if(highPointCount >= 4){
                return (hiddenCount <= 1)? 10.0 : 800.0;
              }else{
                return (hiddenCount <= 1)? 2.0 : 400.0;
              }
              // 低ポイント
            }else{
              return (hiddenCount <= 2)? 20.0 : -BAN;
            }
            // 2pt以上割り振る
          }else if(pointThisTurn[myId] >= 2){
            if(originalPoint >= 5){
              return (hiddenCount <= 2)? 2.0 : 200.0;
            }else{
              return (hiddenCount <= 2)? 5.0 : 400.0;
            }
            // 1pt以上割り振る
          }else if(pointThisTurn[myId] >= 1){
            // high point
            if(originalPoint >= 5){
              return (hiddenCount <= 1)? 5.0 : 10.0;
              // low point
            }else{
              return (hiddenCount == 0)? 20.0 : 10.0;
            }
          }else{
            return (hiddenCount <= 3)? 0.0 : 0.1;
          }
        }
        /*
         * 最下位とのpt差が2pt FNLD2
         * 高いほど悪い評価
         */
      }else if(diff == 2){
        if(pointThisTurn[myId] >= 4){
          // 高スコア
          if(originalPoint >= 5){
            if(hiddenCount >= 3){
              return -BAN;
            }else{
              // 最初の得点計算
              if(turn == 5){
                if(sumPoint >= 15){
                  if(highPointCount <= 3){
                    return (hiddenCount == 0)? 100.0 : 500.0;
                  }else{
                    return (hiddenCount == 0)? 200.0 : 1000.0;
                  }
                }else{
                  if(highPointCount <= 3){
                    return (hiddenCount == 0)? 0.0 : 400.0;
                  }else{
                    return (hiddenCount == 0)? 0.0 : 300.0;
                  }
                }
              }else{
                if(sumPoint >= 30){
                  return (hiddenCount == 0)? 10.0 : 200.0;
                }else{
                  if(highPointCount <= 2){
                    return (hiddenCount == 0)? 0.0 : 500.0;
                  }else{
                    return (hiddenCount == 0)? 0.0 : 1000.0;
                  }
                }
              }
            }
            // 低スコア
          }else{
            if(originalPoint == 4){
              return (hiddenCount == 0)? 100.0 : -BAN;
            }else{
              return (hiddenCount == 0)? 200.0 : -BAN;
            }
          }
          // 3pt割り振る
        }else if(pointThisTurn[myId] == 3){
          // 高スコア
          if(originalPoint >= 5){
            if(hiddenCount >= 3){
              return -BAN;
            }else if(highPointCount <= 3){
              return (hiddenCount == 0)? 1.0 : 300.0;
            }else{
              return (hiddenCount == 0)? 0.0 : 200.0;
            }
            // 低スコア
          }else{
            return (hiddenCount == 0)? 0.0 : -BAN;
          }
          // 2pt以上割り振る
        }else if(pointThisTurn[myId] >= 2){
          // 高スコア
          if(originalPoint >= 5){
            if(highPointCount <= 3){
              return (hiddenCount == 0)? 1.0 : 300.0;
            }else{
              return (hiddenCount == 0)? 0.0 : 200.0;
            }
            // 低スコア
          }else{
            if(originalPoint == 4){
              return (hiddenCount == 0)? 0.0 : 200.0;
            }else{
              if(highPointCount >= 3){
                return (hiddenCount == 0)? 5.0 : 200.0;
              }else{
                return (hiddenCount == 0)? 0.0 : 200.0;
              }
            }
          }
          // 1pt以上割り振る
        }else if(pointThisTurn[myId] >= 1){
          // 高スコア
          if(originalPoint >= 5){
            return (hiddenCount <= 2)? 0.0 : 200.0;
            // 低スコア
          }else{
            return (hiddenCount <= 1)? 0.0 : 400.0;
          }
          // 2pt以上割り振る
        }else if(pointThisTurn[myId] >= 2){
          // 高スコア
          if(originalPoint >= 5){
            if(hiddenCount >= 3){
              return -BAN;
            }else{
              // 最初の得点計算
              if(turn == 5){
                if(sumPoint >= 15){
                  return (hiddenCount <= 1)? 100.0 : 500.0;
                }else{
                  return (hiddenCount <= 1)? 0.0 : 100.0;
                }
              }else{
                if(sumPoint >= 30){
                  return (hiddenCount <= 1)? 10.0 : 200.0;
                }else{
                  return (hiddenCount <= 1)? 0.0 : 100.0;
                }
              }
            }
            // 低スコア
          }else{
            return (hiddenCount <= 2)? 10.0 : 20.0;
          }
          // 1pt以上割り振る
        }else if(pointThisTurn[myId] >= 1){
          // 高スコア
          if(originalPoint >= 5){
            return (hiddenCount <= 2)? 0.0 : 20.0;
            // 低スコア
          }else{
            return (hiddenCount <= 2)? 0.0 : 1.0;
          }
          // 1ptも割り振らない
        }else{
          if(hiddenCount <= 2){
            return (hiddenCount <= 1)? 5.0 : 10.0;
          }else{
            return (hiddenCount <= 1)? 0.0 : 0.5;
          }
        }
        /*
         * 最下位とのpt差が1pt FNLD1
         */
      }else if(diff >= 1){
        // 4pt以上割り振る
        if(pointThisTurn[myId] >= 4){
          // 高スコア
          if(originalPoint >= 5){
            return (hiddenCount == 0)? 300.0 : -BAN;
            // 低スコア
          }else{
            // 高スコアが少ない
            if(highPointCount <= 2){
              if(turn == 5){
                return (hiddenCount == 0)? 1000.0 : -BAN;
              }else{
                return -BAN;
              }
            }else{
              return -BAN;
            }
          }
          // 3pt以上割り振る
        }else if(pointThisTurn[myId] >= 3){
          // 高スコア
          if(originalPoint >= 5){
            return (hiddenCount == 0)? 10.0 : 1000.0;
            // 低スコア
          }else{
            // 高スコアが少ない
            if(highPointCount <= 2){
              if(originalPoint == 4){
                return (hiddenCount == 0)? 20.0 : 80.0;
              }else{
                return (hiddenCount == 0)? 30.0 : 160.0;
              }
            }else{
              return (hiddenCount == 0)? 100.0 : 500.0;
            }
          }
          // 2pt以上割り振る
        }else if(pointThisTurn[myId] >= 2){
          // 高スコア
          if(originalPoint >= 5){
            if(hiddenCount >= 3){
              return -BAN;
            }else{
              return (hiddenCount == 0)? 2.0 : 200.0;
            }
            // 低スコア
          }else{
            // 高スコアが少ない
            if(highPointCount <= 2){
              return (hiddenCount == 0)? 1.0 : 200.0;
            }else{
              return (hiddenCount == 0)? 2.0 : 400.0;
            }
          }
          // 1pt割り振る
        }else if(pointThisTurn[myId] >= 1){
          // 高スコア
          if(originalPoint >= 5){
            if(hiddenCount >= 3){
              return -BAN;
            }else{
              if(highPointCount >= 3){
                return (hiddenCount == 0)? 10.0 : 1000.0;
              }else{
                return (hiddenCount == 0)? 10.0 : 600.0;
              }
            }
            // 低スコア
          }else{
            // 高スコアが少ない
            if(highPointCount <= 2){
              return (hiddenCount == 0)? 1.0 : 20.0;
            }else{
              return (hiddenCount == 0)? 20.0 : 100.0;
            }
          }
          // 何も割り振らない
        }else{
          return (hiddenCount <= 1)? 5.0 : 20.0;
        }
      }else{
        return 1.0;
      }
    }

    double playerPoint(int id){
      int bestScore      = -1;
      int secondBest     = -1;
      int bestPlayer     = -1;
      int bestSameCount   = 1;
      int worstScore     = 100;
      int worstPlayer    = -1;
      int secondWorst    = -1;
      int worstSameCount  = 1;

      for(int i = 0; i < PLAYER_COUNT; i++){
        if(bestScore <= point[i]){
          if(bestScore == point[i]){
            bestSameCount++;
          }else{
            bestPlayer = i;
            bestSameCount = 1;
          }
          secondBest = bestScore;
          bestScore = point[i];
        }else if(secondBest < point[i]){
          secondBest = point[i];
        }

        if(worstScore >= point[i]){
          if(worstScore == point[i]){
            worstSameCount++;
          }else{
            worstPlayer = i;
            worstSameCount = 1;
          }
          secondWorst = worstScore;
          worstScore = point[i];
        }else if(secondWorst > point[i]){
          secondWorst = point[i];
        }
      }

      int bestDiff     = bestScore - secondBest;
      int normalDiff   = point[myId] - worstScore;
      int worstDiff    = secondWorst - point[myId];
      int topDiff      = bestScore - point[myId];

      if(worstPlayer == id){
        if(isLastDay()){
          // 最後のターン
          if(turn == 9){
            if(pointThisTurn[myId] >= 1){
              return BIGBAN;
            }else{
              return 0.0;
            }
            // 5 turn
            // 2pt以上割り振る
          }else if(pointThisTurn[myId] >= 2){
            return -(attention / worstSameCount) * 1000.0;
            // 1pt割り振る
          }else if(pointThisTurn[myId] == 1){
            if(originalPoint >= 5){
              return (hiddenCount == 0)? -100 : BAN;
            }else{
              return (hiddenCount == 0)? -500 : BAN;
            }
            // 0pt割り振る
          }else{
            if(originalPoint >= 5){
              return -1000.0;
            }else{
              return -10.0;
            }
          }
        }else{
          return -(attention / worstSameCount) * fuzzyWorst(worstDiff);
        }
      }else if(bestPlayer == id){
        if(isLastDay()){
          return fuzzyScoreLast(bestDiff);
        }else{
          return fuzzyScore(bestDiff);
        }
      }else{
        if(isLastDay()){
          return -attention * fuzzyNormalLast(normalDiff, topDiff);
        }else{
          return -attention * fuzzyNormal(normalDiff, topDiff);
        }
      }
    }
};

struct PickUpList {
  vector<int> list;
  double score;

  bool operator >(const PickUpList &e) const{
    return score < e.score;
  }    
};

/*
 * split関数
 */
vector<int> split(string s, char del = ' '){
  vector<int> res;
  replace(s.begin(),s.end(),del,' ');
  stringstream ss(s);
  string elem;
  for(int i=0; ss >> elem; i++){
    res.push_back(atoi(elem.c_str()));
  }
  return res;
}

Player player_list[PLAYER_COUNT];
Lang langList[LANG_COUNT];

int langTotalPoint = 0;

class Tutorial{
  public:

    void allBefore(){
      string str;
      ifstream ifs("first_selection.txt");

      if(ifs.fail()){
        fprintf(stderr, "Failed\n");
        cout << "Failed" << endl;
      }   

      int i = 0;
      int j = 0;

      while(getline(ifs, str)){
        string sl = str.c_str();
        vector<int> s = split(sl);

        if(i % 2 == 0){
          j = 1;
          for(int i = 0; i < s.size(); i++){
            j *= s[i];
          }
        }else{
          firstSelection[j] = s;
        }

        i++;
      }   
    }   

    /*
     * 初期化処理
     */
    void init(){
      vector<cardScore> scoreList;

      cin >> T >> P >> N;

      memset(expectPoint, 0.0, sizeof(expectPoint));
      memset(scoreTypeCount, 0, sizeof(scoreTypeCount));

      /*
       * 言語情報の初期化
       */
      for(int i = 0; i < N; i++){
        Lang l;
        l.id = i;
        int attention;
        // 言語のポイントを取得
        cin >> attention;
        l.attention = attention;
        l.originalPoint = l.attention;
        langTotalPoint += l.attention;
        scoreTypeCount[attention]++;

        if(attention >= 5) highPointCount++;

        scoreList.push_back(cardScore(attention, i));
        fsId *= attention;
        langList[i] = l;
      }

      sort(scoreList.begin(), scoreList.end());
      reverse(scoreList.begin(), scoreList.end());

      /*
       * 選択ソートでポイントが高い順にランク付けを行う
       */
      for(int i = 1; i <= LANG_COUNT; i++){
        int id = scoreList[i-1].second;
        langList[id].ranking = i;
      }

      /*
       * プレイヤー情報の初期化
       */
      for(int i = 0; i < P; i++){
        Player p;
        p.id = i;
        p.score = 0.0;
        player_list[i] = p;
      }
    }

    // 渡されたidのプレイヤーのスコアを計算
    double calcScore(int id){
      double score = 0.0;

      for(int i = 0; i < LANG_COUNT; i++){
        score += langList[i].playerPoint(id);
      }

      return score;
    }

    /*
     * 予測値を計算、
     */
    double calcExpectScore(int hiddenCount){
      //fprintf(stderr, "calcExpectScore\n");
      if(hiddenCount == 6){
        return 0.0;
      }else if(hiddenCount == 5){
        return 0.0;
      }else if(hiddenCount == 4){
        return 0.0;
      }else if(hiddenCount == 3){
        return 0.0;
      }else if(hiddenCount == 2){
        return 0.0;
      }else{
        return 2.0;
      }
    }

    void expectHiddenPoint2(){
    }

    /*
     * ゲームデータの更新を行う
     */
    void updateGameData(){
      //fprintf(stderr, "update =>\n");
      for(int i = 1; i < LANG_COUNT; i++){
        langList[i].popularity = (double)langList[i].enemyTotalPoint() / gameTotalPoint;
      }
    }

    void showHiddenCount(){
      for(int i = 0; i < LANG_COUNT; i++){
        fprintf(stderr, "%d ", langList[i].hiddenCount);
      }
      fprintf(stderr, "\n");
    }

    void showPointThisTurn(){
      for(int i = 0; i < LANG_COUNT; i++){
        fprintf(stderr, "%d ", langList[i].pointThisTurn[myId]);
      }
      fprintf(stderr, "\n");
    }

    /*
     * 公開ポイントの追加を行う
     */
    void addPoint(int id, vector<int> &list, int p = 1){
      int size = list.size();

      for(int i = 0; i < size; i++){
        langList[list[i]].point[id] += p;
        langList[list[i]].pointThisTurn[id] += p;
      }
    }

    /*
     * 公開ポイントの削除を行う
     */
    void subPoint(int id, vector<int> &list, int p = 1){
      int size = list.size();

      for(int i = 0; i < size; i++){
        langList[list[i]].point[id] -= p;
        langList[list[i]].pointThisTurn[id] -= p;
      }
    }

    /*
     * 平日の選択を考える
     */
    vector<int> weekSelect(){
      vector<int> bestPattern;
      priority_queue< PickUpList, vector<PickUpList>, greater<PickUpList> > que;

      RepeatedCombinationGenerator<int> g(&card_list[0], &card_list[6], 5);
      do {
        vector<int> data = g.data();

        addPoint(myId, data);

        double score = calcScore(myId);

        PickUpList pick;
        pick.score = score;
        pick.list = data;
        que.push(pick);

        subPoint(myId, data);

      } while(g.next());

      vector<int> ans = que.top().list;
      //fprintf(stderr, "turn %d: BestScore = %f\n", turn, que.top().score);

      return ans;
    }

    vector<int> holidaySelect(){
      priority_queue< PickUpList, vector<PickUpList>, greater<PickUpList> > que;

      RepeatedCombinationGenerator<int> g(&card_list[0], &card_list[6], 2);
      do {
        vector<int> data = g.data();

        addPoint(myId, data, 2);

        double score = calcScore(myId); 

        PickUpList pick;
        pick.score = score;
        pick.list = data;
        que.push(pick);

        subPoint(myId, data, 2);

      } while(g.next());
      vector<int> ans = que.top().list;
      //fprintf(stderr, "turn %d: BestScore = %f\n", turn, que.top().score);

      return ans;
    }

    /*
     * 最初のターンの選択
     *   - ポイントが高い順に1ptずつ振っていく
     */
    vector<int> firstSelect(){
      vector<int> list;

      /*
         int cnt = 0;
         for(int i = 6; i > 0 && cnt < 6; i--){
         for(int j = 0; j < LANG_COUNT && cnt < 6; j++){
         if(langList[j].originalPoint >= i){
         list.push_back(j);
         cnt++;
         }
         }
         }

         return list;
         */

      vector<int> sl = firstSelection[fsId];

      for(int i = 0; i < LANG_COUNT; i++){
        for(int j = 0; j < sl[langList[i].ranking-1]; j++){
          list.push_back(i);
        }
      }

      return list;

      for(int i = 0; i < LANG_COUNT; i++){
        if(langList[i].ranking == 1){
          list.push_back(i);
          if(langList[i].attention >= 0.21){
            //list.push_back(i);
          }
          if(langList[i].attention >= 0.26){
            //list.push_back(i);
          }
        }
        if(langList[i].ranking == 2){
          list.push_back(i);
          if(langList[i].attention < 0.21){
            //list.push_back(i);
          }
          if(langList[i].attention < 0.26){
            //list.push_back(i);
          }
        }
        if(langList[i].ranking == 3){
          list.push_back(i);
        }
        if(langList[i].ranking == 4){
          list.push_back(i);
        }
        if(langList[i].ranking == 5){
          list.push_back(i);
        }
        if(langList[i].ranking == 6){
        }
      }

      return list;
    }

    /*
     * ターン毎に必要な初期化処理を行う
     */
    void eachTurnProc(){
      if(turn == 6){
        memset(expectPoint, 0.0, sizeof(expectPoint));

        for(int i = 0; i < LANG_COUNT; i++){
          langList[i].hiddenCount = 0;
        }
      }

      for(int i = 0; i < LANG_COUNT; i++){
        langList[i].update();
      }
    }

    void updateAttention(){
      for(int i = 0; i < LANG_COUNT; i++){
        langList[i].attention = langList[i].attention / langTotalPoint;
      }
    }

    void rollbackAttention(){
      for(int i = 0; i < LANG_COUNT; i++){
        langList[i].attention = langList[i].originalPoint;
      }
    }

    void updateOpenPoint(){
      /*
       * 公開されているポイントの更新
       */
      //fprintf(stderr, "\noriginal\n");
      for(int n = 0; n < LANG_COUNT; n++){
        for(int m = 0; m < PLAYER_COUNT; m++){
          int pt;
          cin >> pt;
          //fprintf(stderr, "%d ", pt);
          langList[n].point[m] = pt;
        }
        //fprintf(stderr, "\n");
      }

      /*
       * 自分のリアルポイントの更新
       */
      //fprintf(stderr, "\n");
      for(int n = 0; n < N; n++){
        cin >> langList[n].point[myId];
        langList[n].showRanking();
      }
    }

    /*
     * 隠しポイントの更新を行う
     */
    void updateHiddenPoint(){
      int hidden = 0;
      vector<int> hiddenList;

      for(int n = 0; n < N; n++){
        int c;
        cin >> c;
        if(c != 0){
          hidden++;
        }
        for(int h = 0; h < c-myHiddenSelect[n]; h++){
          hiddenList.push_back(n);
        }
        langList[n].hiddenCount += c;
        totalHiddenCount += c;
      }

      gameTotalPoint += 15;
    }

    void run(){
      cout << "READY" << endl;

      allBefore();
      init();

      /*
       * 9ターン処理を回す
       */
      for(turn = 1; turn <= TURN_LIMIT; turn++){
        cin >> T >> D;

        eachTurnProc();

        if(isFirstDay()){
          updateAttention();
        }else{
          rollbackAttention();
        }

        updateOpenPoint();
        //showPointList();

        if(!isHoliday()){
          updateHiddenPoint();
        }else{
          gameTotalPoint += 12;
        }

        updateGameData();
        vector<int> res;

        /*
         * 日付によって処理を変える
         *   - 初日
         *   - 休日
         *   - 平日
         */
        if(isFirstDay()){
          res = firstSelect();
        }else if(isHoliday()){
          res = holidaySelect();
        }else{
          res = weekSelect();
        }

        cout << submit2string(res) << endl;
        //fprintf(stderr, "turn = %d, size = %lu\n", turn, res.size());
        //showHiddenCount();
      }
    }

    // 休日かどうかをチェック
    bool isHoliday(){
      return turn % 2 == 0;
    }

    // 初日かどうかをチェック
    bool isFirstDay(){
      return turn == 1;
    }

    void showMyHiddenSelect(){
      for(int i = 0; i < LANG_COUNT; i++){
        fprintf(stderr, "%d ", myHiddenSelect[i]);
      }
      fprintf(stderr, "\n");
    }

    void showPointList(){
      fprintf(stderr, "\n");

      for(int i = 0; i < LANG_COUNT; i++){
        for(int j = 0; j < PLAYER_COUNT; j++){
          fprintf(stderr, "%d ", langList[i].point[j]);
        }
        fprintf(stderr, "\n");
      }
    }

    /*
     * vector型で渡した投票のリストを文字列に変換する
     */
    string submit2string(vector<int> &list){
      memset(myHiddenSelect, 0, sizeof(myHiddenSelect));
      int size = list.size();
      string res = "";

      for(int i = 0; i < size; i++){
        res += '0' + list[i];
        if(i != size-1) res += " ";

        /*
         * 自分の隠しポイントは差し引く
         */
        if(isHoliday()){
          langList[list[i]].hiddenCount--;
          myHiddenSelect[list[i]]++;
          totalHiddenCount--;
        }
      }

      return res;
    }
};

int main(){

  Tutorial t;

  t.run();

  return 0;
}
