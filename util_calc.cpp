#include <iostream>
#include <fstream>
#include <string>
#include <math.h>
#include <vector>
#include <stdlib.h>
#include <stdio.h>
using namespace std;
//the function split, splits the string wrt ',' and outputs a vector of strings
vector<string> split(string str) {
    vector<string> arr;
    int n = str.size();
    string temp = "";
    for (int i = 0; i < n; i++) {
        if (str[i] != ',') {
            temp += str[i];
        }
        else if (temp != "") {
            arr.push_back(temp);
            temp = "";
        }
    }
    if (temp != "") { arr.push_back(temp); }
    return arr;
}
int main(int argc, char* argv[]){
   string name = argv[1];
   ifstream original("outo.txt");
   ifstream optimised(name);
   
   string ol, nl;
   vector<string> ov, nv;
   double qdo, ddo, qdn, ddn;

   double qderror, dderror;

   int line_no=0;

   while(getline(original,ol) && getline(optimised,nl)){
      line_no++;
      if(line_no == 1){continue;}
      ov = split(ol);   //ol - line from the subtask 2 ouput
      nv = split(nl);   //nl - corresponding line in the new output
      qdo = stod ((ov.at(1)));  //queue density of subask 2
      ddo = stod ((ov.at(2)));  //dynamic density of subtask 2
      qdn = stod ((nv.at(1)));  //new queue density
      ddn = stod ((nv.at(2)));  //new dynamic density
      if(qdo==0.0){
        qderror+=qdn;
      }else{
        qderror += pow( (qdo-qdn)/qdo, 2.0);
      }
      if (ddo==0.0){dderror+= ddn;}
      else{
        dderror += pow( (ddo-ddn)/ddo, 2.0);
      }
      /*if(qderror!=0){
      	cout<<line_no<<','<<qderror<<'\n';
      }*/

   }
   //cout<<qderror<<','<<dderror<<','<<line_no-1<<'\n';
   qderror = pow ( (qderror /(line_no-1)) , 0.5);
   dderror = pow ( (dderror /(line_no-1)) , 0.5);
   
   double qdutil = 1 - qderror;
   double ddutil = 1 - dderror;

   cout<< "queue density utililty = "<<qdutil*100<<",  "<<"dynamic desity utility = "<<ddutil*100<<'\n';
}
