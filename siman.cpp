#include <iostream>
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
int T, P, N;
string D;
int turn = 0;
int card_list[6] = { 0, 1, 2, 3, 4, 5 };
int pointValue[4] = { 3, 5, 7, 9 };

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

bool isLastDay(){
  return turn == 9 || turn == 5;
}

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

    double interest(int id){
      return (double)point[id] / totalPoint();
    }

    int totalPoint(){
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

    double fuzzyScore(int diff){
      // 2位との差が5pt以上ある
      if(diff >= 5){
        // 4ターン目の隠しポイント
        if(turn == 4){
          // 2pt以上割り振った場合
          if(pointThisTurn[myId] >= 2){
            if(originalPoint >= 5){
              return (hiddenCount <= 1)? -1.0 : 1.5;
            }else{
              return (hiddenCount <= 2)? -1.0 : 0.5;
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
        // 3ターン目と7ターン目 - 5pt
        }else if(turn % 4 == 3){
          // 1pt以上振っている
          if(pointThisTurn[myId] >= 1){
            // 高ポイント
            if(originalPoint >= 5){
              return (hiddenCount <= 2)? -1.0 : 2.0;
            // 低ポイント
            }else{
              return (hiddenCount <= 2)? -1.0 : 2.0;
            }
          // 1ptも降っていない - 3T-7T-5PT
          }else{
            // 高ポイント
            if(originalPoint >= 5){
              return (hiddenCount <= 2)? -1.0 : -2.0;
            // 低ポイント
            }else{
              return (hiddenCount <= 2)? -0.5 : -1.0;
            }
          }
        // 6T-8T 5pt
        }else{
          // 高ポイント
          if(originalPoint >= 5){
            // 1pt以上割り振っている
            if(pointThisTurn[myId] >= 1){
              return (hiddenCount <= 2)? 1.0 : 1.5;
            // 1ptも割り振っていない
            }else{
              return (hiddenCount <= 2)? 1.0 : 0.5;
            }
          // 低ポイント
          }else{
            // 1pt以上割り振っている
            if(pointThisTurn[myId] >= 1){
              return (hiddenCount <= 2)? -1.0 : 1.0;
            // 1ptも割り振っていない
            }else{
              return (hiddenCount <= 2)? 0.0 : 1.0;
            }
          }
        }
      // diff 4
      }else if(diff == 4){
        if(turn % 4 == 3){
          // 1pt以上割り振る
          if(pointThisTurn[myId] >= 1){
            // 高スコア
            if(originalPoint >= 5){
              return (hiddenCount <= 2)? -1.0 : 1.0;
            // 低スコア
            }else{
              return (hiddenCount <= 2)? -1.0 : 1.0;
            }
          // 1ptも割り振らない
          }else{
            // 高スコア
            if(originalPoint >= 5){
              return (hiddenCount <= 2)? 0.5 : 0.1;
            // 低スコア
            }else{
              return (hiddenCount <= 2)? 1.0 : 0.5;
            }
          }
        }else{
          // 1pt割り振っている
          if(pointThisTurn[myId] >= 1){
            return (hiddenCount <= 2)? -1.0 : 1.0;
          // 1ptも割り振らない
          }else{
            return (hiddenCount <= 2)? 0.5 : 1.0;
          }
        }
      // 2位との差が3ptある
      }else if(diff == 3){
        if(turn % 4 == 3){
          // 1pt以上割り振る
          if(pointThisTurn[myId] >= 2){
            // 高ポイント
            if(originalPoint >= 5){
              return (hiddenCount <= 1)? 2.0 : 0.9;
            // 低ポイント
            }else{
              return (hiddenCount <= 1)? 1.0 : 0.5;
            }
          // 1ptも増やさない
          }else{
            // 高スコア
            if(originalPoint >= 5){
              return (hiddenCount <= 1)? -1.0 : -2.0;
            }else{
              return (hiddenCount <= 1)? 1.0 : 0.5;
            }
          }
        // 4ターン目(2回目の隠しポイント)
        }else if(turn == 4){
          // 2pt以上振っている
          if(pointThisTurn[myId] >= 2){
            return (hiddenCount <= 1)? 10.0 : 0.9;
          // 何も降っていない
          }else{
            if(originalPoint >= 5){
              return (hiddenCount <= 1)? -1.0 : -2.0;
            }else{
              return (hiddenCount <= 1)? 1.0 : 0.9;
            }
          }
        }else{
          return (hiddenCount <= 1)? 1.0 : 0.9;
        }
      // 2位との差が2pt
      }else if(diff == 2){
        if(turn % 4 == 3){
          // 1pt以上割り振っている
          if(pointThisTurn[myId] >= 1){
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
        // 意味ないかも
        }else{
          return (hiddenCount <= 1)? 1.0 : 0.3;
        }
      // 2位との差が1pt
      }else if(diff == 1){
        // 2ターン目の隠しポイントの割り当て
        if(turn == 2){
          // 1ターン目に競合が少なければ積極的に狙う
          // 高ポイント
          if(originalPoint >= 5){
            return (totalPoint() <= 1)? 20.0 : -1.0;
          // 低ポイント
          }else{
            return (totalPoint() <= 1)? 10.0 : -1.0;
          }
        // 4ターン目の隠しポイントの割り当て
        }else if(turn == 4){
          // 2つのポイントを割り振った場合
          if(pointThisTurn[myId] >= 4){
            // 高ポイント
            if(originalPoint >= 5){
              return (hiddenCount == 0)? 0.7 : 0.2;
            // 低ポイント
            }else{
              return (hiddenCount == 0)? 0.1 : -1.0;
            }
          // 1つのポイントを割り振った場合
          }else if(pointThisTurn[myId] >= 2){
            return (hiddenCount == 0)? 0.3 : 0.2;
          // 何もポイントを振らない
          }else{
            // 高スコア
            if(originalPoint >= 5){
              return (hiddenCount <= 1)? -1.0 : -2.0;
            // 低スコア
            }else{
              return (hiddenCount == 0)? 0.5 : 0.2;
            }
          }
        // それ以外のターン 3 or 7
        }else{
          return (hiddenCount <= 1)? -1.0 : -2.0;
        }
      // 2位との差が0pt
      }else{
        return (hiddenCount <= 1)? -1.0 : -2.0;
      }
    }

    double fuzzyScoreLast(int diff, int normalDiff){
      if(diff >= 5){
        if(pointThisTurn[myId] >= 1){
          return (hiddenCount <= 2)? -100.0 : 1.0;
        }else{
          return (hiddenCount <= 2)? 10.0 : 1.0;
        }
      // diff 4
      }else if(diff == 4){
        if(pointThisTurn[myId] >= 2){
          return (hiddenCount <= 2)? -100.0 : 1.0;
        }else{
          return (hiddenCount <= 2)? 10.0 : 1.0;
        }
      // diff 3
      }else if(diff == 3){
        if(pointThisTurn[myId] >= 1){
          return (hiddenCount <= 1)? 10.0 : 1.0;
        }else{
          return (hiddenCount <= 0)? 5.0 : 1.0;
        }
      // diff 2
      }else if(diff == 2){
        if(pointThisTurn[myId] >= 1){
          return (hiddenCount <= 1)? 1.0 : 0.5;
        }else{
          return (hiddenCount <= 1)? 1.0 : 0.5;
        }
      // diff 1
      }else if(diff == 1){
        if(pointThisTurn[myId] >= 3){
          return (originalPoint == 3)? -1.0 : 0.3;
        }else if(pointThisTurn[myId] >= 2){
          return (originalPoint == 3)? 0.5 : 0.3;
        }else if(pointThisTurn[myId] >= 1){
          return (hiddenCount == 0)? 1.0 : 0.3;
        }else{
          if(pointThisTurn[myId] >= 1){
            return (hiddenCount <= 2)? 0.7 : 0.3;
          }else{
            return (hiddenCount <= 2)? 0.5 : 0.1;
          }
        }
      // diff 0
      }else{
        if(pointThisTurn[myId] >= 1){
          if(originalPoint == 3){
            return (hiddenCount >= 3)? 0.0 : 0.0;
          }else{
            return (hiddenCount >= 3)? -10.0 : 0.0;
          }
        }else{
          if(originalPoint == 3){
            return (hiddenCount >= 1)? 0.0 : -1.0;
          }else{
            return (hiddenCount >= 1)? -10.0 : -1.0;
          }
        }
      }
    }

    double fuzzyWorst(int diff){
      // 無理しないで諦めたほうが良い
      if(diff >= 4){
        return 0.0;
      // そこまで無理しないでも良い
      }else if(diff >= 3){
        return (hiddenCount <= 2)? 0.2 : 0.0;
      // 隠しパラメータ1発同点範囲
      }else if(diff >= 2){
        return (hiddenCount <= 3)? 0.5 : 0.2;
      // 隠しパラメータ1発逆転
      }else if(diff >= 1){
        return (hiddenCount <= 3)? 0.5 : 0.2;
      }else{
        return 0.5;
      }
    }

    double fuzzyNormal(int diff){
      if(diff >= 5){
        return (hiddenCount <= 2)? 0.0 : 0.15;
      }else if(diff >= 4){
        return (hiddenCount <= 2)? 0.0 : 0.25;
      }else if(diff >= 3){
        return (hiddenCount <= 2)? 0.0 : 0.35;
      }else if(diff >= 2){
        return (hiddenCount <= 1)? 0.0 : 0.9;
      }else if(diff >= 1){
        return (hiddenCount == 0)? 0.0 : 1.0;
      }else{
        return 1.5;
      }
    }

    /*
     * 最下位との点差を考えてペナルティを考える
     */
    double fuzzyNormalLast(int diff){
      if(diff >= 5){
        if(pointThisTurn[myId] >= 1){
          return (hiddenCount <= 4)? 10.0 : 0.0;
        }else{
          return (hiddenCount <= 4)? 0.0 : 0.0;
        }
      }else if(diff >= 4){
        if(pointThisTurn[myId] >= 1){
          return (hiddenCount <= 4)? 10.0 : 0.0;
        }else{
          return (hiddenCount <= 4)? 0.0 : 0.0;
        }
      }else if(diff >= 3){
        if(pointThisTurn[myId] >= 1){
          return (hiddenCount <= 3)? 5.0 : 0.1;
        }else{
          return (hiddenCount <= 3)? 0.0 : 0.1;
        }
      }else if(diff >= 2){
        if(pointThisTurn[myId] >= 2){
          return (hiddenCount <= 2 || pointThisTurn[myId] <= 1)? 0.0 : 0.5;
        }else{
          return (hiddenCount <= 2 || pointThisTurn[myId] <= 1)? 0.0 : 0.5;
        }
      }else if(diff >= 1){
        if(pointThisTurn[myId] >= 2){
          return (hiddenCount <= 2 && originalPoint >= 4)? 10.0 : 10;
        }else{
          return (hiddenCount <= 1)? -2.0 : 10.0;
        }
      }else{
        return 1.0;
      }
    }

    double playerPoint(int id){
      int bestScore      = -1;
      int secondBest     = -1;
      int bestPlayer     = -1;
      int bestSameCount   = 0;
      int worstScore     = 100;
      int worstPlayer    = -1;
      int secondWorst    = -1;
      int worstSameCount  = 0;

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
      int normalDiff   = point[0] - worstScore;
      int worstDiff    = secondWorst - point[0];

      if(worstPlayer == id){
        if(isLastDay()){
          // ラス確なのでそのままの値を返す
          return -(attention / worstSameCount) * 10.0;
        }else{
          return -(attention / worstSameCount) * fuzzyWorst(worstDiff);
        }
      }else if(bestPlayer == id){
        if(isLastDay()){
          return (attention / bestSameCount) * fuzzyScoreLast(bestDiff, normalDiff);
        }else{
          return (attention / bestSameCount) * fuzzyScore(bestDiff);
        }
      }else{
        if(isLastDay()){
          return -attention * fuzzyNormalLast(normalDiff);
        }else{
          return -attention * fuzzyNormal(normalDiff);
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

Player player_list[PLAYER_COUNT];
Lang langList[LANG_COUNT];

int langTotalPoint = 0;

class Tutorial{
  public:
    void init(){
      cin >> T >> P >> N;

      memset(expectPoint, 0.0, sizeof(expectPoint));

      /*
       * 言語情報の初期化
       */
      for(int i = 0; i < N; i++){
        Lang l;
        l.id = i;
        cin >> l.attention;
        l.originalPoint = l.attention;
        langTotalPoint += l.attention;

        langList[i] = l;
      }

      /*
       * 選択ソートでポイントが高い順にランク付けを行う
       */
      for(int i = 1; i <= LANG_COUNT; i++){
        double bestAttention = 0;
        int bestNumber = -1;

        for(int j = 0; j < LANG_COUNT; j++){
          if(langList[j].ranking != -1) continue;

          if(bestAttention <= langList[j].attention){
            bestAttention = langList[j].attention;
            bestNumber = j;
          }
        }

        langList[bestNumber].ranking = i;
      }

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
     * 敵の隠しポイントを予測する。
     */
    void calcExpectedScore(int hiddenCount, vector<int> &list){
      //fprintf(stderr, "calcExpectedScore =>\n");
      if(list.size() == 0) return;
      if(isFirstDay()) return;

      /*
       * 5ターン目終了後に、隠しポイントが公開されるので、隠しポイントは
       * 初期化しておく。
       */
      if(turn == 6){
        for(int i = 0; i < LANG_COUNT; i++){
          langList[i].hiddenCount = 0;
        }
      }

      vector<PickUpList> p1list = selectHiddenPick(1, 2, 2, list);
      vector<PickUpList> p2list = selectHiddenPick(2, 2, 2, list);
      vector<PickUpList> p3list = selectHiddenPick(3, 2, 2, list);

      vector<int> p1 = p1list[0].list;
      vector<int> p2 = p2list[0].list;
      vector<int> p3 = p3list[0].list;

      double expectScore = calcExpectScore(hiddenCount);

      if(expectScore < 2.0) return;

      for(int i = 0; i < 2; i++){
        expectPoint[1][p1[i]] += expectScore;
        expectPoint[2][p2[i]] += expectScore;
        expectPoint[3][p3[i]] += expectScore;

        langList[p1[i]].hiddenCount--;
        langList[p2[i]].hiddenCount--;
        langList[p3[i]].hiddenCount--;
      }
    }

    /*
     * ゲームデータの更新を行う
     */
    void updateGameData(){
      //fprintf(stderr, "update =>\n");
      for(int i = 1; i < LANG_COUNT; i++){
        langList[i].popularity = (double)langList[i].totalPoint() / gameTotalPoint;
      }

      /*
       * 予測ポイントを公開ポイントに追加する
       */
      for(int i = 0; i < LANG_COUNT; i++){
        langList[i].point[1] += expectPoint[1][i];
        langList[i].point[2] += expectPoint[2][i];
        langList[i].point[3] += expectPoint[3][i];
      }
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

    vector<PickUpList> selectHiddenPick(int id, int num, int point, vector<int> data){
      //fprintf(stderr, "selectHiddenPick =>\n");
      vector<PickUpList> list;
      priority_queue< PickUpList, vector<PickUpList>, greater<PickUpList>  > que;
      double score;

      do{
        PickUpList pl;
        pl.list = data;
        addPoint(id, data, point);

        score = calcScore(id);
        pl.score = score;
        que.push(pl);

        subPoint(id, data, point);
      }while(next_combination(data.begin(), data.begin()+num, data.end()));

      for(int i = 0; i < 10 && !que.empty(); i++){
        PickUpList pl = que.top(); que.pop();
        list.push_back(pl);
      }

      return list;
    }

    /*
     * 平日の選択を考える
     */
    vector<int> weekSelect(){
      double bestScore = -10000000000.0;
      double score;
      vector<int> bestPattern;

      RepeatedCombinationGenerator<int> g(&card_list[0], &card_list[6], 5);
      do {
        vector<int> data = g.data();

        addPoint(myId, data);

        score = calcScore(myId);

        if(bestScore < score){
          bestScore = score;
          bestPattern = data;
        }

        subPoint(myId, data);

      } while(g.next());

      return bestPattern;
    }

    vector<int> holidaySelect(){
      double bestScore = -1000000000.0;
      double totalScore;
      vector<int> bestPattern;

      RepeatedCombinationGenerator<int> g(&card_list[0], &card_list[6], 2);
      do {
        vector<int> data = g.data();

        addPoint(myId, data, 2);

        double score = calcScore(myId);

        if(bestScore < score){
          bestScore = score;
          bestPattern = data;
        }

        subPoint(myId, data, 2);

      } while(g.next());

      return bestPattern;
    }

    /*
     * 最初のターンの選択
     *   - ポイントが高い順に1ptずつ振っていく
     */
    vector<int> firstSelect(){
      vector<int> list;

      for(int i = 0; i < LANG_COUNT; i++){
        if(langList[i].ranking == 1){
          list.push_back(i);
          list.push_back(i);
          if(langList[i].attention >= 0.21){
            list.push_back(i);
          }
          if(langList[i].attention >= 0.26){
            list.push_back(i);
          }
        }
        if(langList[i].ranking == 2){
          list.push_back(i);
          if(langList[i].attention < 0.21){
            list.push_back(i);
          }
          if(langList[i].attention < 0.26){
            list.push_back(i);
          }
        }
        if(langList[i].ranking == 3){
        }
        if(langList[i].ranking == 4){
        }
        if(langList[i].ranking == 5){
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

        if(turn == 3 && langList[i].originalPoint == 3) langList[i].attention = 0.1;
        if(turn == 2) langList[i].attention -= max(2, langList[i].totalPoint());
      }
    }

    void updateOpenPoint(){
      /*
       * 公開されているポイントの更新
       */
      for(int n = 0; n < N; n++){
        for(int m = 0; m < P; m++){
          cin >> langList[n].point[m];
        }
      }

      /*
       * 自分のリアルポイントの更新
       */
      for(int n = 0; n < N; n++){
        cin >> langList[0].point[n];
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

      calcExpectedScore(hidden, hiddenList);
      gameTotalPoint += 15;
    }

    void run(){
      cout << "READY" << endl;

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
          //updateAttention();
          rollbackAttention();
        }

        updateOpenPoint();

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
