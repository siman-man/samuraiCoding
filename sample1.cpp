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
int T, P, N;
string D;
int turn = 0;
int attention[9] = { 0, 0, 0, 0, 0, 0, 0, 0, 0 };

class Player{
  public:
    int id;
    int points[9];
    int real_point[9];
};

Player player_list[9];
int hidden_count[9][9];

class Tutorial{
  public:
    void init(){
      cin >> T >> P >> N;

      memset(hidden_count, 0, sizeof(hidden_count));

      for(int i = 0; i < N; i++){
        cin >> attention[i];
      }
      for(int i = 0; i < P; i++){
        Player p;
        p.id = i;
        memset(p.points, 0, sizeof(p.points));
        memset(p.real_point, 0, sizeof(p.real_point));
        player_list[i] = p;
      }
    }

    bool isHoliday(int t){
      return t%2 == 0;
    }

    void run(){
      init();

      cout << "READY" << endl;

      for(turn = 0; turn < 9; turn++){
        cin >> T >> D;

        for(int n = 0; n < N; n++){
          for(int m = 0; m < P; m++){
            cin >> player_list[m].points[n];
          }
        }

        for(int n = 0; n < N; n++){
          cin >> player_list[0].real_point[n];
        }

        if(isHoliday(turn)){
          for(int n = 0; n < N; n++){
            cin >> hidden_count[turn/2][n];
          }
        }

        if(isHoliday(turn)){
          cout << "0 0 0 0 0" << endl;
        }else{
          cout << "0 0" << endl;
        }
      }
    }
};

int main(){

  Tutorial t;

  t.run();

  return 0;
}