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
const int MY_ID         = 0;
int T, P, N;
string D;
int turn = 0;
int card_list[6] = { 0, 1, 2, 3, 4, 5 };

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

int total_hidden_count = 0;
int game_total_point = 0;
double expect_point[4][6];
int myHiddenSelect[LANG_COUNT];

bool isLastDay(){
  return turn == 8;
}

class Lang{
  public:
    int id;
    int ranking;
    int point[PLAYER_COUNT];
    int hidden_count;
    int attention;
    double popularity;

    Lang(){
      ranking = -1;
      hidden_count = 0;
      popularity = 0.0;
    }

    double interest(int id){
      return (double)point[id] / total_point();
    }

    int total_point(){
      int total_point = 0;

      for(int i = 1; i < PLAYER_COUNT; i++){
        total_point += point[i];
      }

      return total_point + 2 * hidden_count;
    }

    double get_ratio(){
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

    double fuzzy_score(int diff){
      if(diff >= 5){
        return 1.0;
      }else if(diff >= 4){
        return 1.0;
      }else if(diff >= 3){
        return (isLastDay())? 1.0 : 0.9;
      }else if(diff >= 2){
        return (isLastDay())? 1.0 : 0.8;
      }else if(diff >= 1){
        return (isLastDay())? 1.0 : 0.6;
      }else{
        return 0.3;
      }
    }

    double fuzzy_worst(int diff){
      if(diff >= 5){
        return 0.0;
      }else if(diff >= 4){
        return (isLastDay())? 1.0 : 0.1;
      }else if(diff >= 3){
        return (isLastDay())? 1.0 : 0.6;
      }else if(diff >= 2){
        return (isLastDay())? 1.0 : 0.8;
      }else if(diff >= 1){
        return (isLastDay())? 1.0 : 1.5;
      }else{
        return (isLastDay())? 0.5 : 3.0;
      }
    }

    double fuzzy_normal(int diff){
      if(diff >= 5){
        return 0.0;
      }else if(diff >= 4){
        return (isLastDay())? 0.0 : 0.05;
      }else if(diff >= 3){
        return (isLastDay())? 0.0 : 0.15;
      }else if(diff >= 2){
        return (isLastDay())? 0.0 : 0.2;
      }else if(diff >= 1){
        return (isLastDay())? 0.0 : 0.4;
      }else{
        return 0.7;
      }
    }

    double player_point(int id){
      int best_score      = -1;
      int second_best     = -1;
      int best_player     = -1;
      int best_same_cnt   = 0;
      int worst_score     = 100;
      int worst_player    = -1;
      int second_worst    = -1;
      int worst_same_cnt  = 0;

      for(int i = 0; i < PLAYER_COUNT; i++){
        if(best_score <= point[i]){
          if(best_score == point[i]){
            best_same_cnt++;
          }else{
            best_player = i;
            best_same_cnt = 1;
          }
          second_best = best_score;
          best_score = point[i];
        }

        if(worst_score >= point[i]){
          if(worst_score == point[i]){
            worst_same_cnt++;
          }else{
            worst_player = i;
            worst_same_cnt = 1;
          }
          second_worst = worst_score;
          worst_score = point[i];
        }
      }

      int best_diff     = best_score - second_best;
      int normal_diff   = point[0] - worst_score;
      int worst_diff    = second_worst - point[0];

      if(worst_player == id){
        return -((double)attention / worst_same_cnt) * fuzzy_worst(worst_diff);
      }else if(best_player == id){
        return ((double)attention / best_same_cnt) * fuzzy_score(best_diff);
      }else{
        return -attention * fuzzy_normal(normal_diff) * get_ratio();
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

class Tutorial{
  public:
    void init(){
      cin >> T >> P >> N;

      memset(expect_point, 0.0, sizeof(expect_point));

      for(int i = 0; i < N; i++){
        Lang l;
        l.id = i;
        cin >> l.attention;

        langList[i] = l;
      }

      for(int i = 1; i <= LANG_COUNT; i++){
        int best_attention = 0;
        int best_number = -1;

        for(int j = 0; j < LANG_COUNT; j++){
          if(langList[j].ranking != -1) continue;

          if(best_attention < langList[j].attention){
            best_attention = langList[j].attention;
            best_number = j;
          }
        }

        langList[best_number].ranking = i;
      }

      for(int i = 0; i < P; i++){
        Player p;
        p.id = i;
        p.score = 0.0;
        player_list[i] = p;
      }
    }

    // 渡されたidのプレイヤーのスコアを計算
    double calc_score(int id){
      double score = 0.0;

      for(int i = 0; i < LANG_COUNT; i++){
        score += langList[i].player_point(id);
      }

      return score;
    }

    double calc_expect_score(int hidden_cnt){
      //fprintf(stderr, "calc_expect_score\n");
      if(hidden_cnt == 6){
        return 0.1;
      }else if(hidden_cnt == 5){
        return 0.3;
      }else if(hidden_cnt == 4){
        return 0.5;
      }else if(hidden_cnt == 3){
        return 1.0;
      }else if(hidden_cnt == 2){
        return 2.0;
      }else{
        return 2.0;
      }
    }

    void calc_expected_score(int hidden_cnt, vector<int> list){
      //fprintf(stderr, "calc_expected_score =>\n");
      if(list.size() == 0) return;
      if(isFirstDay(turn)) return;

      if(turn == 5){
        for(int i = 0; i < 6; i++){
          expect_point[1][i] = 0;
          expect_point[2][i] = 0;
          expect_point[3][i] = 0;
        }
      }

      vector<PickUpList> p1list = selectHiddenPick(1, 2, 2, list);
      vector<PickUpList> p2list = selectHiddenPick(2, 2, 2, list);
      vector<PickUpList> p3list = selectHiddenPick(3, 2, 2, list);

      vector<int> p1 = p1list[0].list;
      vector<int> p2 = p2list[0].list;
      vector<int> p3 = p3list[0].list;

      double expect_score = calc_expect_score(hidden_cnt);

      for(int i = 0; i < 2; i++){
        expect_point[1][p1[i]] += expect_score;
        expect_point[2][p2[i]] += expect_score;
        expect_point[3][p3[i]] += expect_score;
      }
    }

    void update(){
      //fprintf(stderr, "update =>\n");
      for(int i = 1; i < LANG_COUNT; i++){
        langList[i].popularity = (double)langList[i].total_point() / game_total_point;
      }

      for(int i = 0; i < LANG_COUNT; i++){
        langList[i].point[1] += expect_point[1][i];
        langList[i].point[2] += expect_point[2][i];
        langList[i].point[3] += expect_point[3][i];
      }
    }

    void add_point(int id, vector<int> &list, int p = 1){
      int size = list.size();

      for(int i = 0; i < size; i++){
        langList[list[i]].point[id] += p;
      }
    }

    void sub_point(int id, vector<int> &list, int p = 1){
      int size = list.size();

      for(int i = 0; i < size; i++){
        langList[list[i]].point[id] -= p;
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
        add_point(id, data, point);

        score = calc_score(id);
        pl.score = score;
        que.push(pl);

        sub_point(id, data, point);
      }while(next_combination(data.begin(), data.begin()+num, data.end()));

      for(int i = 0; i < 5 && !que.empty(); i++){
        PickUpList pl = que.top(); que.pop();
        list.push_back(pl);
      }

      return list;
    }

    vector<PickUpList> selectTopPick(int id, int num, int point = 1){
      vector<PickUpList> list;
      priority_queue< PickUpList, vector<PickUpList>, greater<PickUpList>  > que;
      double score;

      RepeatedCombinationGenerator<int> g(&card_list[0], &card_list[6], num);
      do {
        vector<int> data = g.data();
        PickUpList pl;
        pl.list = data;

        add_point(id, data, point);

        score = calc_score(id);
        pl.score = score;
        que.push(pl);

        sub_point(id, data, point);

      } while(g.next());

      for(int i = 0; i < 20; i++){
        PickUpList pl = que.top(); que.pop();
        list.push_back(pl);
      }

      return list;
    }

    vector<int> week_select(){
      double best_score = -1000000000.0;
      double score;
      vector<int> best_pattern;

      vector<PickUpList> p1list = selectTopPick(1, 5);
      vector<PickUpList> p2list = selectTopPick(2, 5);
      vector<PickUpList> p3list = selectTopPick(3, 5);

      RepeatedCombinationGenerator<int> g(&card_list[0], &card_list[6], 5);
      do {
        vector<int> data = g.data();

        add_point(MY_ID, data);

        int p1size = p1list.size();
        double total_score = 0.0;

        for(int i = 0; i < p1size; i++){
          int p2size = p2list.size();

          add_point(1, p1list[i].list);

          for(int j = 0; j < p2size; j++){
            int p3size = p3list.size();

            add_point(2, p2list[j].list);

            for(int k = 0; k < p3size; k++){

              add_point(3, p3list[k].list);

              score = calc_score(MY_ID);
              total_score += score;

              sub_point(3, p3list[k].list);
            }

            sub_point(2, p2list[j].list);
          }

          sub_point(1, p1list[i].list);
        }


        if(best_score < total_score){
          best_score = total_score;
          best_pattern = data;
        }

        sub_point(MY_ID, data);

      } while(g.next());

      return best_pattern;
    }

    vector<int> holiday_select(){
      double best_score = -1000000000.0;
      double total_score;
      vector<int> best_pattern;

      vector<PickUpList> p1list = selectTopPick(1, 2, 2);
      vector<PickUpList> p2list = selectTopPick(2, 2, 2);
      vector<PickUpList> p3list = selectTopPick(3, 2, 2);

      RepeatedCombinationGenerator<int> g(&card_list[0], &card_list[6], 2);
      do {
        vector<int> data = g.data();

        add_point(MY_ID, data, 2);
        total_score = 0.0;

        int p1size = p1list.size();

        for(int i = 0; i < p1size; i++){
          int p2size = p2list.size();

          add_point(1, p1list[i].list, 2);

          for(int j = 0; j < p2size; j++){
            int p3size = p3list.size();

            add_point(2, p2list[j].list, 2);

            for(int k = 0; k < p3size; k++){
              add_point(3, p3list[k].list, 2);

              double score = calc_score(MY_ID);
              total_score += score;

              sub_point(3, p3list[k].list, 2);
            }

            sub_point(2, p2list[j].list, 2);
          }

          sub_point(1, p1list[i].list, 2);
        }

        if(best_score < total_score){
          best_score = total_score;
          best_pattern = data;
        }

        sub_point(MY_ID, data, 2);

      } while(g.next());

      return best_pattern;
    }

    vector<int> first_select(){
      vector<int> list;

      for(int i = 0; i < 6; i++){
        if(langList[i].ranking < 6){
          list.push_back(i);
        }
      }

      return list;
    }

    void run(){
      cout << "READY" << endl;

      init();

      for(turn = 0; turn < TURN_LIMIT; turn++){
        cin >> T >> D;

        for(int n = 0; n < N; n++){
          for(int m = 0; m < P; m++){
            cin >> langList[n].point[m];
          }
        }

        for(int n = 0; n < N; n++){
          cin >> langList[0].point[n];
        }


        if(!isHoliday(turn)){
          int hidden = 0;
          vector<int> hidden_list;

          for(int n = 0; n < N; n++){
            int c;
            cin >> c;
            if(c != 0){
              hidden++;
            }
            for(int h = 0; h < c-myHiddenSelect[n]; h++){
              hidden_list.push_back(n);
            }
            langList[n].hidden_count += c;
            total_hidden_count += c;
          }
          calc_expected_score(hidden, hidden_list);
          game_total_point += 15;
        }else{
          game_total_point += 12;
        }

        update();
        vector<int> res;

        if(isFirstDay(turn)){
          res = first_select();
        }else if(isHoliday(turn)){
          res = holiday_select();
        }else{
          res = week_select();
        }
        cout << submit2string(res) << endl;
      }
    }

    bool isHoliday(int t){
      return t%2 == 1;
    }

    bool isFirstDay(int t){
      return t == 0;
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
        if(!isHoliday(turn)){
          langList[list[i]].hidden_count--;
          myHiddenSelect[list[i]] = 1;
          total_hidden_count--;
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
