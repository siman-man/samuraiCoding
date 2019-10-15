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
const int myId          = 0;

// params
const int TOP           = 2;
const int TDIFF         = 7;
const int LDIFF         = 7;
const int DIVISION      = 7;
const int POINT         = 6;
const int TURN          = 9;
const int HIDE          = 6;
const int SCORE         = 4;

const double BAN = -100000000.0;
int T, P, N;
string D;
int turn = 0;
int division;
int card_list[6] = { 0, 1, 2, 3, 4, 5 };
int pointValue[4] = { 3, 5, 7, 9 };
int leaning[TOP * TDIFF * LDIFF * DIVISION * POINT * TURN * HIDE * SCORE];
vector<int> idList(6,0);
vector<int> bestIdList(6,0);

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

    int createId(int tdiff, int ldiff, int point, int top = 0){
      int s       = SCORE;
      int hs      = HIDE * SCORE;
      int lhs     = LDIFF * hs;
      int tlhs    = TDIFF * lhs;
      int ptlhs   = POINT * tlhs;
      int dptlhs  = DIVISION * ptlhs;
      int tdptlhs = TURN * dptlhs;

      int pid = (top * (tdptlhs)) + ((turn-1) * (dptlhs)) + (division * (ptlhs)) + (point * tlhs) + (min(TDIFF-1, tdiff) * lhs) + (min(LDIFF-1, ldiff) * hs) + (min(HIDE-1, hiddenCount) * s) + (originalPoint-3);

      idList[id] = pid;
      return pid;
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

      int top          = (bestPlayer == myId)? 1 : 0;
      int topDiff      = (top == 1)? point[myId] - secondBest : bestScore - point[myId];
      int lastDiff     = point[myId] - worstScore;

      return leaning[createId(topDiff, lastDiff, pointThisTurn[myId], top)];
    }
};

struct PickUpList {
  vector<int> list;
  double score;
  vector<int> idList;

  bool operator >(const PickUpList &e) const{
    return score < e.score;
  }    
};

Player player_list[PLAYER_COUNT];
Lang langList[LANG_COUNT];

int langTotalPoint = 0;

class Tutorial{
  public:
    void allBefore(){
      string str;
      ifstream ifs("leaning.txt");

      if(ifs.fail()){
        cout << "Failed" << endl;
      }
  
      int i = 0;
      int score;
      while(getline(ifs, str)){
        score = atoi(str.c_str());
        leaning[i] = score;
        i++;
      }

      fprintf(stderr, "leaning size = %lu\n", sizeof(leaning)/sizeof(int));
    }

    void init(){
      cin >> T >> P >> N;

      memset(expectPoint, 0.0, sizeof(expectPoint));
      memset(scoreTypeCount, 0, sizeof(scoreTypeCount));

      int sumPt = 0;
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
        sumPt += attention;

        if(attention >= 5) highPointCount++;

        langList[i] = l;
      }

      // 区間を決める
      division = min(36, sumPt) / 3 - 5;
      fprintf(stderr, "division = %d, sumPt = %d\n", division, sumPt);

      /*
       * 選択ソートでポイントが高い順にランク付けを行う
       */
      for(int i = 1; i <= LANG_COUNT; i++){
        double bestAttention = 0;
        int bestNumber = -1;

        for(int j = 0; j < LANG_COUNT; j++){
          if(langList[j].ranking != -1) continue;

          if(bestAttention < langList[j].attention){
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
        pick.idList = idList;
        que.push(pick);

        subPoint(myId, data);

      } while(g.next());

      vector<int> ans = que.top().list;
      bestIdList = que.top().idList;
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

        if(score > BAN * 0.1){
          PickUpList pick;
          pick.score = score;
          pick.list = data;
          pick.idList = idList;
          que.push(pick);
        }

        subPoint(myId, data, 2);

      } while(g.next());
      vector<int> ans = que.top().list;
      //fprintf(stderr, "turn %d: BestScore = %f\n", turn, que.top().score);
      bestIdList = que.top().idList;

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
        if(isHoliday()){
          res = holidaySelect();
        }else if(isFirstDay()){
          //res = firstSelect();
          res = weekSelect();
        }else{
          res = weekSelect();
        }

        cout << submit2string(res) << " " << leaning2string() << endl;
        fprintf(stderr, "turn %d: %s\n", turn, leaning2string().c_str());
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

    string leaning2string(){
      int size = bestIdList.size();
      string res = "";

      for(int i = 0; i < size; i++){
        //fprintf(stderr, "id: %d, value: %d\n", bestIdList[i], leaning[bestIdList[i]]);
        stringstream ss;
        ss << bestIdList[i];
        res += ss.str();
        if(i != size-1) res += " ";
      }


      return res;
    }
};

int main(){

  Tutorial t;

  t.run();

  return 0;
}
