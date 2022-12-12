#pragma once
#include<string>
#include<vector>

using namespace std;

class node
{
    public:
    string gate;
    string gate_name; 
    pair<int, string> A1, A2, ZN;
    vector<node*> fan_out;
    double c_out, c_in1, c_in2, trans_out, trans_in1, trans_in2;
    int sen_stat;
    double cell_del, delay_a1, delay_a2, delay_out;
    node* prev_1, *prev_2;
};

void parseNode(fstream&, vector<string>&);
void separate(vector<string>&, vector<string>&,vector<string>&,vector<string>&,vector<string>&,string&);
int truthTable(string, int, int);
void parseInput(fstream&,vector<string>&, vector<vector<int>>&, int);
void parseLiberty(fstream&, vector<string>&);
void parseData(vector<string>&);
int findFromLib(node*);

void makeGraph(vector<string>&);
void makeConnection(vector<node*>&);
void display(node*, fstream&);
void displaySection2(node*, fstream&);
void assignAddr(vector<string>&, node*);
void overallCalc(vector<pair<string,vector<node*>>>&,vector<vector<int>>&, node*, fstream&, fstream&, fstream&);
void detOutput(node*);
void reInitialze(node*);
void detTransitionDelayTime(node*);
void valuesFromLib(double,double,node*);
void longShortPath(vector<vector<string>>&,vector<node*>&,fstream&);
void detPaths(vector<node*>&);

extern vector<vector<double>> nand_trans_rise;
extern vector<vector<double>> nand_trans_fall;
extern vector<vector<double>> nand_delay_rise;
extern vector<vector<double>> nand_delay_fall;

extern vector<vector<double>> nor_trans_rise;
extern vector<vector<double>> nor_trans_fall;
extern vector<vector<double>> nor_delay_rise;
extern vector<vector<double>> nor_delay_fall;
 
extern vector<vector<double>> inv_trans_rise;
extern vector<vector<double>> inv_trans_fall;
extern vector<vector<double>> inv_delay_rise;
extern vector<vector<double>> inv_delay_fall;

extern vector<double> cap_index;
extern vector<double> time_index;

extern double inv_i;
extern double nand_a1, nand_a2;
extern double nor_a1, nor_a2;