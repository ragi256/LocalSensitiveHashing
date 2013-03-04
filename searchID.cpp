// searchID.cpp
// ver 3.10
// 3/4 update

// ビットストリーム列を複数回シャッフル→ソートを繰り返し
// 全IDとの組み合わせを検証し、近似IDを抜き出す

#include <iostream>
#include <sstream>
#include <iomanip>
#include <fstream>
#include <string>
#include <vector>
#include <set>
#include <utility>
#include <boost/random.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/unordered_map.hpp>

#include "time.h"

using namespace std;
using namespace boost;

typedef pair<string,string> Pair;
typedef vector<string> v_str;
typedef unordered_map<string,set<string> > near_map;

inline void stop(void);
inline void shuffle(vector<Pair> &setlist, mt19937 &r_engine, uniform_int<> &dst);
inline void extractID(vector<Pair> &setlist, const int &ti, set<string> &nearlist);

struct str_pair_less{
  bool operator()(const Pair &x, const Pair &y) const {
    return x.second < y.second;
  };
};

int list_size,limit,nearness;

int main(int argc, char **argv)
{
  ifstream input(argv[1]); //check.txt
  ofstream output(argv[2], ios::out); //ID.txt
  string line, ID, target;
  v_str ID_and_stream, target_list;
  vector<Pair> setlist;
  near_map result;
  unsigned long int K,TIME, counter=0;
  time_t start_time,end_time;

  cout << "評価回数と検索対象との近似幅を入力してください" << endl;
  cin >> TIME; cin >> nearness;

  time(&start_time);
  
  cout << "ID,ストリーム読み込み処理開始" << endl;
  while( input && getline(input, line)){ 
    algorithm::split(ID_and_stream,line,is_any_of("\t"));
    string bit_stream = ID_and_stream[1];
    string bits="";
    K = bit_stream.size();
    for(int i=0; i< K/4; ++i){
      string four_bits = bit_stream.substr(4*i,4);
      short int cash=0x00;
      for(int j=0; j<4; ++j){
	if(four_bits[j]=='1') cash = cash<<1 | 0x01;
	else if(four_bits[j]=='0') cash = cash <<1;
      }
      ostringstream oss;
      oss << hex << cash;
      string hex_str = oss.str();
      bits += hex_str;
      four_bits.clear();
      cash=0x00;
    }
    Pair temp = make_pair(ID_and_stream[0],bits);
    target_list.push_back(ID_and_stream[0]);
    setlist.push_back(temp);
  }
  cout << "ID,ストリーム読み込み処理終了" << endl;

  limit =setlist[0].second.size(); // 一回のシャッフルでどことどこをスワップするかを
  mt19937 r_engine;
  uniform_int<> dst(1, limit); // 16進ハッシュの桁数に依存する上限のランダムで決定する
  list_size = setlist.size();
  output << list_size << endl;
  cerr << "list_size is " << list_size << endl;

  cout << "近似ツイート抽出処理開始" << endl;
  for(int i=0; i<TIME; ++i){ // 読み込んだstreamに対して処理
    shuffle(setlist, r_engine, dst);
    sort(setlist.begin(),setlist.end(), str_pair_less());
    //    if((i+1)%10 == 0)
    cout << "\t" << i+1 << "/" << TIME << "times finish" << endl;
        
    for(int i=0; i<list_size; ++i){
      string target = setlist[i].first;
      extractID(setlist, i, result[target]);
    }
  }

  cout << "近似ツイート抽出処理終了" << endl;
  int percent;
  if(list_size%100!=0) percent = (list_size - list_size%100)/100;
  else percent = list_size/100;    
	
  for(v_str::iterator target=target_list.begin(); target!=target_list.end(); ++target){
    output << *target << "\n\n";
    for(set<string>::iterator ID=result[*target].begin(), p_end=result[*target].end(); ID!=p_end; ++ID)
      output << *ID << endl;
    result[*target].clear();
    //    copy(result[*target].begin(), result[*target].end(), ostream_iterator<string>(output,"\n"));
    output << "\n";
    ++counter;
    if(counter%percent == 0) cerr << counter/percent << "% tweet finish" << endl;
  }

  ID_and_stream.clear();
  target_list.clear();
  setlist.clear();
  result.clear();
  
  time(&end_time);
  cerr << "所要時間:" << difftime(end_time, start_time) << "秒" << endl;  
}

inline void stop(void){
  cout << "入力IDが見つかりません。" << endl;
  cout << "プログラムを終了します。" << endl;
  exit(1);
}

inline void shuffle(vector<Pair> &setlist, mt19937 &r_engine, uniform_int<> &dst){
  int shuffle_times = dst(r_engine)/2; 
  int key1,key2;
  for(int j=0; j<shuffle_times; ++j){
    key1 = dst(r_engine);
    key2 = dst(r_engine);
    for(int k=0; k < list_size; ++k){
      string &temp=setlist[k].second;
      swap(temp[key1],temp[key2]);
    }
  }
}

inline void extractID(vector<Pair> &setlist, const int &ti,set<string> &nearlist){
  int index;
  for(int point=0; point<nearness; ++point){
    index = ti - (nearness - point);
    if( index < 0 ); // pass
    else nearlist.insert(setlist[index].first);
    index = ti + (nearness - point);
    if( (list_size-1) < index ); //pass
    else nearlist.insert(setlist[index].first);
  }
}

