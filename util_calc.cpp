#include <iostream>
#include <fstream>
#include <string>
#include <math.h>
#include <vector>
#include <stdlib.h>
#include <stdio.h>
using namespace std;

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
