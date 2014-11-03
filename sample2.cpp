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
int T, P, N;
string D;
int turn = 0;

class Player{
  public:
    int id;
};

class Lang{
  public:
    int id;
    int ranking;
    int point[PLAYER_COUNT];
    int hidden_count;
    int attention;
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
        memset(my_point, 0, sizeof(my_point));
        player_list[i] = p;
      }
    }

    void update(int turn){
    }

    vector<int> week_select(int turn){
      vector<int> list;

      list.push_back(0);
      list.push_back(1);
      list.push_back(1);
      list.push_back(2);
      list.push_back(2);

      return list;
    }

    vector<int> holiday_select(int turn){
      vector<int> list;

      list.push_back(1);
      list.push_back(2);

      return list;
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
};

int main(){

  Tutorial t;

  t.run();

  return 0;
}