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
template < class BidirectionalIterator >
bool next_combination ( BidirectionalIterator first1 ,
  BidirectionalIterator last1 ,
  BidirectionalIterator first2 ,
  BidirectionalIterator last2 )
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

template < class BidirectionalIterator >
bool next_combination ( BidirectionalIterator first ,
  BidirectionalIterator middle ,
  BidirectionalIterator last )
{
  return next_combination (first , middle , middle , last );
}

template < class BidirectionalIterator >
inline
bool prev_combination ( BidirectionalIterator first ,
  BidirectionalIterator middle ,
  BidirectionalIterator last )
{
  return next_combination (middle , last , first , middle );
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

class Lang{
  public:
    int id;
    int ranking;
    int point[PLAYER_COUNT];
    int hidden_count;
    int attention;

    double player_point(int id){
      int best_score = -1;
      int best_player = -1;
      int best_same_cnt = 0;
      int worst_score = 100;
      int worst_player = -1;
      int worst_same_cnt = 0;

      for(int i = 0; i < PLAYER_COUNT; i++){
        if(best_score <= point[i]){
          if(best_score == point[i]){
            best_same_cnt++;
          }else{
            best_player = i;
            best_same_cnt = 1;
          }
          best_score = point[i];
        }else if(worst_score >= point[i]){
          if(worst_score == point[i]){
            worst_same_cnt++;
          }else{
            worst_score = point[i];
            worst_player = i;
            worst_same_cnt = 1;
          }
        }
      }

      if(worst_player == id){
        return -(double)attention / worst_same_cnt;
      }else if(best_player == id){
        return (double)attention / best_same_cnt;
      }else{
        return 0.0;
      }
    }
};

Player player_list[PLAYER_COUNT];
Lang lang_list[LANG_COUNT];
int hidden_count[4][LANG_COUNT];
int my_point[LANG_COUNT];

class Tutorial{
  public:
    void init(){
      cin >> T >> P >> N;

      memset(hidden_count, 0, sizeof(hidden_count));

      for(int i = 0; i < N; i++){
        Lang l;
        l.id = i;
        l.ranking = -1;
        l.hidden_count = 0;
        cin >> l.attention;

        lang_list[i] = l;
      }

      for(int i = 1; i <= LANG_COUNT; i++){
        int best_attention = 0;
        int best_number = -1;

        for(int j = 0; j < LANG_COUNT; j++){
          if(lang_list[j].ranking != -1) continue;

          if(best_attention < lang_list[j].attention){
            best_attention = lang_list[j].attention;
            best_number = j;
          }
        }

        lang_list[best_number].ranking = i;
      }

      for(int i = 0; i < P; i++){
        Player p;
        p.id = i;
        p.score = 0.0;
        memset(my_point, 0, sizeof(my_point));
        player_list[i] = p;
      }
    }

    double calc_score(int id){
      double score = 0.0;

      for(int i = 0; i < LANG_COUNT; i++){
        score += lang_list[i].player_point(id);
      }

      return score;
    }

    void update(int turn){
    }

    void add_point(int id, vector<int> &list, int p = 1){
      int size = list.size();

      for(int i = 0; i < size; i++){
        lang_list[list[i]].point[id] += p;
      }
    }

    void sub_point(int id, vector<int> &list, int p = 1){
      int size = list.size();

      for(int i = 0; i < size; i++){
        lang_list[list[i]].point[id] -= p;
      }
    }

    vector<int> week_select(int turn){
      double best_score = -100.0;
      double score;
      vector<int> best_pattern;

      RepeatedCombinationGenerator<int> g(&card_list[0], &card_list[6], 5);
      do {
        vector<int> data = g.data();

        add_point(MY_ID, data);

        score = calc_score(MY_ID);

        if(best_score < score){
          best_score = score;
          best_pattern = data;
        }

        sub_point(MY_ID, data);

      } while(g.next());

      return best_pattern;
    }

    vector<int> holiday_select(int turn){
      double best_score = -100.0;
      double score;
      vector<int> best_pattern;

      RepeatedCombinationGenerator<int> g(&card_list[0], &card_list[6], 2);
      do {
        vector<int> data = g.data();

        add_point(MY_ID, data, 2);

        score = calc_score(MY_ID);

        if(best_score < score){
          best_score = score;
          best_pattern = data;
        }

        sub_point(MY_ID, data, 2);

      } while(g.next());

      return best_pattern;
    }

    vector<int> first_select(){
      vector<int> list;

      for(int i = 0; i < 6; i++){
        if(lang_list[i].ranking < 6){
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
            cin >> lang_list[n].point[m];
          }
        }

        for(int n = 0; n < N; n++){
          cin >> my_point[n];
        }

        if(isHoliday(turn)){
          for(int n = 0; n < N; n++){
            cin >> hidden_count[turn/2][n];
          }
        }

        update(turn);
        vector<int> res;

        if(isFirstDay(turn)){
          res = first_select();
        }else if(isHoliday(turn)){
          res = week_select(turn);
        }else{
          res = holiday_select(turn);
        }
        cout << submit2string(res) << endl;
      }
    }

    bool isHoliday(int t){
      return t%2 == 0;
    }

    bool isFirstDay(int t){
      return t == 0;
    }

    string submit2string(vector<int> list){
      int size = list.size();
      string res = "";

      for(int i = 0; i < size; i++){
        res += '0' + list[i];
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