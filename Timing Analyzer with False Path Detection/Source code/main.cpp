#include<iostream>
#include<fstream>
#include<map>
#include<unordered_map>
#include<algorithm>
#include"node.h"

using namespace std;

vector<string> prim_input, prim_out, wire, circuit, full_data;
vector<node*> prim_out_pointer;
string case_name;
vector<node*> node_ptr;
vector<pair<string,vector<node*>>> pattern1;
vector<string> input_pattern;
vector<vector<int>> input_data;
vector<string> libfile;
vector<vector<string>> full_path;
vector<string> long_path, short_path;


node* top=NULL;

bool compare_p(pair<pair<string, int>,pair<double,double>> p1, pair<pair<string, int>,pair<double,double>> p2)
{
    if(p1.second.first==p2.second.first)
    {
        return stod(p1.first.first.erase(0,1))<stod(p2.first.first.erase(0,1));
    }
    else
        return p1.second.first>p2.second.first;
}

bool mycompare(pair<string,double> p1, pair<string,double> p2)
{
    if(p1.second==p2.second)
    {
        return stod(p1.first.erase(0,1))<stod(p2.first.erase(0,1));
    }
    else
        return p1.second>p2.second;
}
bool compare_delay(node* ptr1, node* ptr2)
{
    return ptr1->delay_out>ptr2->delay_out;
}

int main(int argc, char* argv[])
{
    fstream netlist,input_pat, liberty;
    fstream out_load, node_delay, delay_path;
    netlist.open(argv[1],ios::in);
    input_pat.open(argv[3],ios::in);
    liberty.open(argv[5],ios::in);
    if(argc!=6)
    {
        cout<<"Error in arguments"<<endl;
        exit(0);
    }
    if(!netlist.is_open())
    {
        cout<<"Error opening the verilog file for taking input"<<endl;
        exit(0);
    }  
    if(!input_pat.is_open())
    {
        cout<<"Error opening the .pat file for taking input"<<endl;
        exit(0);
    }  
    if(!liberty.is_open())
    {
        cout<<"Error opening the liberty file for taking input"<<endl;
        exit(0);
    }  
    parseNode(netlist,full_data);
    separate(full_data,prim_input, prim_out, wire, circuit,case_name);
    parseLiberty(liberty, libfile);
    parseData(libfile);
    out_load.open("310540011_"+case_name+"_load.txt", ios:: out);
    node_delay.open("310540011_"+case_name+"_delay.txt", ios:: out);
    delay_path.open("310540011_"+case_name+"_path.txt", ios:: out);

    top = new node;
    top->gate_name="top";
    top->A1.first=10;
    top->A2.first=10;
    top->ZN.first=-1;
    top->trans_in1=10;
    top->trans_in2=10;
    top->trans_out=10;
    top->delay_a1=10;
    top->delay_a2=10;
    parseInput(input_pat,input_pattern, input_data,prim_input.size());

    makeGraph(circuit);
    makeConnection(node_ptr);
    assignAddr(input_pattern,top);

    overallCalc(pattern1,input_data,top,out_load,node_delay,delay_path);
    netlist.close();
    input_pat.close();
    liberty.close();
    out_load.close();
    node_delay.close();
    delay_path.close();
    return 0;
}

void makeGraph(vector<string>& circuit)
{
    string str;
    for(int i=0;i<circuit.size();)
    {
        node* ptr = new node;
        ptr->gate=circuit[i];
        
        ptr->gate_name=circuit[i+1];
        if(ptr->gate=="NANDX1" || ptr->gate=="NOR2X1")
        {
            map<string,string> temp;
            temp.insert({circuit[i+2],circuit[i+3]});
            temp.insert({circuit[i+4],circuit[i+5]});
            temp.insert({circuit[i+6],circuit[i+7]});
            map<string, string>::iterator it = temp.begin();
            ptr->A1.second=it->second;
            it++;
            ptr->A2.second=it->second;
            it++;
            ptr->ZN.second=it->second;
            
            if(ptr->gate=="NANDX1")
            {
                ptr->c_in1=nand_a1;
                ptr->c_in2=nand_a2;
            }
            else
            {
                ptr->c_in1=nor_a1;
                ptr->c_in2=nor_a2;
            }
            i=i+8;
        }
        else
        {
            if(circuit[i+2]=="I")
            {
                ptr->A1.second=circuit[i+3];
                ptr->A2.second="xx";
                ptr->ZN.second=circuit[i+5];
            }
            else
            {
                ptr->ZN.second=circuit[i+3];
                ptr->A1.second=circuit[i+5];
                ptr->A2.second="xx";
            }
                
            ptr->c_in1=inv_i;
            ptr->c_in2=-1;
            i=i+6;
        }
        ptr->A1.first=-1;
        ptr->A2.first=-1;
        ptr->ZN.first=-1;
        ptr->trans_out=-1; 
        ptr->trans_in1=-1; 
        ptr->trans_in2=-1;
        ptr->cell_del=-1;
        ptr->delay_a1=-1; 
        ptr->delay_a2=-1;
        ptr->delay_out=-1;
        ptr->prev_1=NULL;
        ptr->prev_2=NULL;

        if(find(prim_out.begin(),prim_out.end(),ptr->ZN.second)!=prim_out.end())
        {
            ptr->c_out=0.03;
            prim_out_pointer.push_back(ptr);
        }
        else
            ptr->c_out=0;

        node_ptr.push_back(ptr);

        // finding whether the node is a primary 
        if(find(prim_input.begin(), prim_input.end(), ptr->A1.second)!=prim_input.end())
        {
            //fanout.push_back(ptr);
            top->fan_out.push_back(ptr);
            ptr->trans_in1=0;
            ptr->delay_a1=0;
        }
        if(ptr->gate!="INVX1" && (find(prim_input.begin(), prim_input.end(), ptr->A2.second)!=prim_input.end()))
        {
            //fanout.push_back(ptr);
            if(find(top->fan_out.begin(),top->fan_out.end(),ptr)==top->fan_out.end())
            {
                top->fan_out.push_back(ptr);
            }
            ptr->trans_in2=0;
            ptr->delay_a2=0;
        }
    }
}

void makeConnection(vector<node*>& node_ptr)
{
    for(int i=0;i<node_ptr.size();i++)
    {
        node* ptr= node_ptr[i];
        for(int j=0;j<node_ptr.size();j++)
        {
            if(i==j)
                continue;
            if(ptr->ZN.second==node_ptr[j]->A1.second)
            {
                ptr->fan_out.push_back(node_ptr[j]);
                ptr->c_out=ptr->c_out+node_ptr[j]->c_in1;
                node_ptr[j]->prev_1=ptr;        // It is connected through path of A1
            }
            else if(node_ptr[j]->gate!="INVX1" && ptr->ZN.second==node_ptr[j]->A2.second)
            {
                ptr->fan_out.push_back(node_ptr[j]);
                ptr->c_out=ptr->c_out+node_ptr[j]->c_in2;
                node_ptr[j]->prev_2=ptr;
            }
        }
    }
}
void display(node* top, fstream& out_load)
{
    int count=1;
    vector<node*> visit;
    visit.push_back(top);
    for(int i=0;i<visit.size();i++)
    {
        for(int j=0;j<visit[i]->fan_out.size();j++)
        {
            if(find(visit.begin(),visit.end(),(visit[i]->fan_out[j]))==visit.end())
            {
                visit.push_back(visit[i]->fan_out[j]);
            }
        }
        count++;
    }

    vector<pair<string,double>> ckt_gph;
    for(int i=1;i<visit.size();i++)
    {
        ckt_gph.push_back({visit[i]->gate_name,visit[i]->c_out}); 
    }
    sort(ckt_gph.begin(),ckt_gph.end(),mycompare);
    for(int i=0;i<ckt_gph.size();i++)
    {
        if(i==ckt_gph.size()-1) 
        {
            out_load<<ckt_gph[i].first<<" "<<ckt_gph[i].second;
            continue;
        }
        out_load<<ckt_gph[i].first<<" "<<ckt_gph[i].second<<endl;
    }

}

void displaySection2(node* top, fstream& node_delay)
{
    int count=1;
    vector<node*> visit;
    visit.push_back(top);
    for(int i=0;i<visit.size();i++)
    {
        for(int j=0;j<visit[i]->fan_out.size();j++)
        {
            if(find(visit.begin(),visit.end(),(visit[i]->fan_out[j]))==visit.end())
            {
                visit.push_back(visit[i]->fan_out[j]);
            }
        }
        count++;
    }
    visit.erase(visit.begin());
    vector<pair<pair<string,int>,pair<double,double>>> delay_data;
    for(int i=0;i<visit.size();i++)
    {
        delay_data.push_back({{visit[i]->gate_name,visit[i]->ZN.first},{visit[i]->cell_del,visit[i]->trans_out}});
    }
    sort(delay_data.begin(),delay_data.end(),compare_p);
    for(int i=0;i<delay_data.size();i++)
    {
         if(i==delay_data.size()-1)
        {
            node_delay<<delay_data[i].first.first<<" "<<delay_data[i].first.second<<" "<<delay_data[i].second.first<<" "<<delay_data[i].second.second;
        }
        else
            node_delay<<delay_data[i].first.first<<" "<<delay_data[i].first.second<<" "<<delay_data[i].second.first<<" "<<delay_data[i].second.second<<endl;
    }

}

void assignAddr(vector<string>& input_pattern, node* top)
{
    for(int i=0;i<input_pattern.size();i++)
    {
        vector<node*> v;
        for(int j=0;j<top->fan_out.size();j++)
        {
            if(input_pattern[i]==top->fan_out[j]->A1.second)
            {
                v.push_back(top->fan_out[j]);
            }
            else if(top->fan_out[j]->gate!="INVX1" && input_pattern[i]==top->fan_out[j]->A2.second)
            {
                v.push_back(top->fan_out[j]);
            }
        }
        pattern1.push_back({input_pattern[i],v});
    }
}
void reInitialize(node* ptr)
{
    int count=1;
    vector<node*> visit;
    visit.push_back(top);
    for(int i=0;i<visit.size();i++)
    {
        for(int j=0;j<visit[i]->fan_out.size();j++)
        {
            if(find(visit.begin(),visit.end(),(visit[i]->fan_out[j]))==visit.end())
            {
                visit.push_back(visit[i]->fan_out[j]);
            }
        }
        count++;
    }
    visit[0]->A1.first=10;
    visit[0]->A2.first=10;
    visit[0]->ZN.first=-1;
    visit[0]->trans_in1=10;
    visit[0]->trans_in2=10;
    visit[0]->trans_out=10;
    visit[0]->delay_a1=10;
    visit[0]->delay_a2=10;

    for(int i=1;i<visit.size();i++)
    {
        visit[i]->A1.first=-1;
        visit[i]->A2.first=-1;
        visit[i]->ZN.first=-1;

        visit[i]->trans_in1=-1;
        visit[i]->trans_in2=-1;
        visit[i]->trans_out=-1;
        visit[i]->delay_a1=-1;
        visit[i]->delay_a2=-1;
        visit[i]->cell_del=-1;
    }
}
void overallCalc(vector<pair<string,vector<node*>>>& pattern1, vector<vector<int>>& input_data, node* top,fstream& out_load, fstream& node_delay, fstream& delay_path)
{
    display(top,out_load);
    for(int i=0;i<input_data.size();i++)        //Primary input initialization
    {
        reInitialize(top);
        for(int j=0;j<input_data[i].size();j++)
        { 

            for(int k=0;k<pattern1[j].second.size();k++)
            {
                if(pattern1[j].first==pattern1[j].second[k]->A1.second)
                {
                    pattern1[j].second[k]->A1.first=input_data[i][j];
                    pattern1[j].second[k]->trans_in1=0;
                    pattern1[j].second[k]->delay_a1=0;

                }
                else if(pattern1[j].first==pattern1[j].second[k]->A2.second)
                {
                    pattern1[j].second[k]->A2.first=input_data[i][j];
                    pattern1[j].second[k]->trans_in2=0;
                    pattern1[j].second[k]->delay_a2=0;
                }
            }
        }
        detOutput(top);         // Output calculation
        detTransitionDelayTime(top);        // Delay and Transition calculation
        displaySection2(top, node_delay);
        if(i!=input_data.size()-1)
        {
            node_delay<<endl<<endl;
        }

        full_path.clear();
        long_path.clear();
        short_path.clear();
        detPaths(prim_out_pointer);
        longShortPath(full_path, prim_out_pointer, delay_path);

        if(i!=input_data.size()-1)
        {
            delay_path<<endl<<endl;
        }
    }
}
void detOutput(node* ptr)   //ptr initially pointing to top
{
    if(ptr->A1.first==-1)
        return;
    else if(ptr->gate!="INVX1" && ptr->A2.first==-1)
        return;

    if(ptr->ZN.first!=-1)
        return;

    if(ptr!=top && ptr->A1.first!=-1)
    {
        if(ptr->gate!="INVX1" && ptr->A2.first!=-1)
        {
            ptr->ZN.first =  truthTable(ptr->gate,ptr->A1.first, ptr->A2.first);
        }
        else if(ptr->gate=="INVX1")
        {
            ptr->ZN.first =  truthTable(ptr->gate,ptr->A1.first, -1);
        }
        for(int k=0;k<ptr->fan_out.size();k++)
        {
            if(ptr->ZN.second==ptr->fan_out[k]->A1.second)
            {
                ptr->fan_out[k]->A1.first= ptr->ZN.first;
            }
            else if(ptr->fan_out[k]->gate!="INVX1" && ptr->ZN.second==ptr->fan_out[k]->A2.second)
            {
                ptr->fan_out[k]->A2.first= ptr->ZN.first;
            }
            
        }
    }
    for(int i=0;i<ptr->fan_out.size();i++)
    {
        detOutput(ptr->fan_out[i]);
    }
}
void detTransitionDelayTime(node* ptr)
{
    if(ptr->trans_in1==-1 && ptr->delay_a1==-1)
    {
        return;
    }
    else if(ptr->gate!="INVX1" && ptr->trans_in2==-1 && ptr->delay_a2==-1)
    {
        return;
    }
    if(ptr!=top)
    {
        // Both trans_in1 and trans_in2 are valid.
        int a = findFromLib(ptr);       // Now calculating delay
        //transition_time
        // At this stage the delay and transition time is updated
        double d;
        if(a==0)
        {
            cout<<"error"<<endl;
        }
        else if(a==1)
        {
            d=ptr->cell_del+ptr->delay_a1;
            ptr->delay_out=d;
        }
        else if(a==2)
        {
            d=ptr->cell_del+ptr->delay_a2;
            ptr->delay_out=d;
        }
        //delay_input of next node =cell_del+delay_input
        for(int k=0;k<ptr->fan_out.size();k++)
        {
            if(ptr->ZN.second==ptr->fan_out[k]->A1.second)
            {
                ptr->fan_out[k]->delay_a1=d;
            }
            else if(ptr->fan_out[k]->gate!="INVX1" && ptr->ZN.second==ptr->fan_out[k]->A2.second)
            {
                ptr->fan_out[k]->delay_a2=d;
            }

        }
        for(int i=0;i<ptr->fan_out.size();i++)          // trans_in will be updated in the suceeding nodes
        {
            if(ptr->ZN.second == ptr->fan_out[i]->A1.second)
            {
                ptr->fan_out[i]->trans_in1=ptr->trans_out;
            }
            else if(ptr->fan_out[i]->gate!="INVX1" && ptr->ZN.second == ptr->fan_out[i]->A2.second)
            {
                ptr->fan_out[i]->trans_in2=ptr->trans_out;
            }
        }
    }
    for(int i=0;i<ptr->fan_out.size();i++)
    {
        detTransitionDelayTime(ptr->fan_out[i]);
    }
}
int findFromLib(node* ptr)
{
    //ptr has valid trans_in1 and trans_in2
    // calculate delay and transition time of node pointed by ptr
    if(ptr->gate=="NOR2X1")     //NC=0, C=1
    {
        if(ptr->A1.first==0 && ptr->A2.first==0)    //Non-controlling input
        {
            if(ptr->delay_a1>=ptr->delay_a2)
            {
                valuesFromLib(ptr->trans_in1,ptr->c_out,ptr);
                return 1;
            }
            else if(ptr->delay_a1<ptr->delay_a2)
            {
                valuesFromLib(ptr->trans_in2,ptr->c_out,ptr);
                return 2;
            }
        }
        else if(ptr->A1.first==1 && ptr->A2.first==1)   //Controlling-input
        {
            if(ptr->delay_a1<=ptr->delay_a2)
            {
                valuesFromLib(ptr->trans_in1,ptr->c_out,ptr);
                return 1;
            }
            else if(ptr->delay_a1>ptr->delay_a2)
            {
                valuesFromLib(ptr->trans_in2,ptr->c_out,ptr);
                return 2;
            }                
        }
        else                                           // One is Controlling and other is Non-Controlling
        {
            if(ptr->A1.first==1)
            {
                valuesFromLib(ptr->trans_in1,ptr->c_out,ptr);
                return 1;
            }
            else
            {
                valuesFromLib(ptr->trans_in2,ptr->c_out,ptr);
                return 2;
            }
        }
    }
    else if(ptr->gate=="NANDX1")
    {
        if(ptr->A1.first==1 && ptr->A2.first==1)    //Non-controlling input
        {
            if(ptr->delay_a1>=ptr->delay_a2)
            {
                valuesFromLib(ptr->trans_in1,ptr->c_out,ptr);
                return 1;
            }
            else if(ptr->delay_a1<ptr->delay_a2)
            {
                valuesFromLib(ptr->trans_in2,ptr->c_out,ptr);
                return 2;
            }
        }
        else if(ptr->A1.first==0 && ptr->A2.first==0)    //Controlling-input
        {
            if(ptr->delay_a1<=ptr->delay_a2)
            {
                valuesFromLib(ptr->trans_in1,ptr->c_out,ptr);
                return 1;
            }
            else if(ptr->delay_a1>ptr->delay_a2)
            {
                valuesFromLib(ptr->trans_in2,ptr->c_out,ptr);
                return 2;
            }
        }
        else                                            // One is Controlling and other is Non-Controlling
        {
            if(ptr->A1.first==0)
            {
                valuesFromLib(ptr->trans_in1,ptr->c_out,ptr);
                return 1;
            }
            else
            {
                valuesFromLib(ptr->trans_in2,ptr->c_out,ptr);
                return 2;
            }                
        }
    }
    else                                               // Inverter Gate
    {
        valuesFromLib(ptr->trans_in1,ptr->c_out,ptr);
        return 1;
    }
    
    return 0;
}
void valuesFromLib(double trans_time,double cap_out,node* ptr)
{
    //Calculate cell delay and transition time

    int c1=-1, c2=-1, t1=-1, t2=-1;
  
    if(find(cap_index.begin(), cap_index.end(),cap_out)!=cap_index.end())
    {
        c1=find(cap_index.begin(), cap_index.end(),cap_out)-cap_index.begin();
    }
    else if(cap_out>cap_index[0] && cap_out<cap_index[cap_index.size()-1])
    {
        for(int i=1;i<cap_index.size();i++)
        {
            if(cap_out>cap_index[i-1] && cap_out<cap_index[i])
            {
                c1=i-1; //first index
                c2=i;
                break;
            }
        }
    }
    else if(cap_out>cap_index[cap_index.size()-1])
    {
        c1=cap_index.size()-2;
        c2=cap_index.size()-1;
    }
    else if(cap_out<cap_index[0])
    {
        c1=0;
        c2=1;
    }
    if(find(time_index.begin(), time_index.end(),trans_time)!=time_index.end())
    {
        t1=find(time_index.begin(), time_index.end(),trans_time)-time_index.begin();
    }
    else if(trans_time>time_index[0] && trans_time<time_index[time_index.size()-1])
    {
        for(int i=1;i<time_index.size();i++)
        {
            if(trans_time>time_index[i-1] && trans_time<time_index[i])
            {
                t1=i-1;     // first index
                t2=i;
                break;
            }
        }
    }
    else if(trans_time<time_index[0])
    {
        t1=0;
        t2=1;
    }
    else if(trans_time>time_index[time_index.size()-1])
    {
        t1=time_index.size()-2;
        t2=time_index.size()-1;
    }

    double temp, temp1, temp2;
    if(ptr->gate=="NOR2X1")
    {
        if(ptr->ZN.first==1)        //Output is rising
        {
            if(t2==-1 && c2==-1)
            {
                ptr->cell_del=nor_delay_rise[t1][c1];
                ptr->trans_out=nor_trans_rise[t1][c1];
            }
            else if(t2==-1 && c2!=-1)
            {
                temp = ((nor_delay_rise[t1][c2]-nor_delay_rise[t1][c1])/(cap_index[c2]-cap_index[c1]));
                temp=temp*(cap_out-cap_index[c1]);
                temp=temp+nor_delay_rise[t1][c1];
                ptr->cell_del=temp;
                temp = ((nor_trans_rise[t1][c2]-nor_trans_rise[t1][c1])/(cap_index[c2]-cap_index[c1]));
                temp=temp*(cap_out-cap_index[c1]);
                temp=temp+nor_trans_rise[t1][c1];
                ptr->trans_out=temp;
            }
            else if(t2!=-1 && c2==-1)
            {
                temp=(nor_delay_rise[t2][c1]-nor_delay_rise[t1][c1])/(time_index[t2]-time_index[t1]);
                temp=temp*(trans_time-time_index[t1]);
                temp=temp+nor_delay_rise[t1][c1];
                ptr->cell_del=temp;
                temp=(nor_trans_rise[t2][c1]-nor_trans_rise[t1][c1])/(time_index[t2]-time_index[t1]);
                temp=temp*(trans_time-time_index[t1]);
                temp=temp+nor_trans_rise[t1][c1];
                ptr->trans_out=temp;                
            }
            else if(t2!=-1 && c2!=-1)
            {
                temp1=(nor_delay_rise[t1][c2]-nor_delay_rise[t1][c1])/(cap_index[c2]-cap_index[c1]);
                temp1=temp1*(cap_out-cap_index[c1]);
                temp1=temp1+nor_delay_rise[t1][c1];

                temp2=(nor_delay_rise[t2][c2]-nor_delay_rise[t2][c1])/(cap_index[c2]-cap_index[c1]);
                temp2=temp2*(cap_out-cap_index[c1]);
                temp2=temp2+nor_delay_rise[t2][c1];

                temp=(temp2-temp1)/(time_index[t2]-time_index[t1]);
                temp=temp*(trans_time-time_index[t1]);
                temp=temp+temp1;
                ptr->cell_del=temp;

                temp1=(nor_trans_rise[t1][c2]-nor_trans_rise[t1][c1])/(cap_index[c2]-cap_index[c1]);
                temp1=temp1*(cap_out-cap_index[c1]);
                temp1=temp1+nor_trans_rise[t1][c1];

                temp2=(nor_trans_rise[t2][c2]-nor_trans_rise[t2][c1])/(cap_index[c2]-cap_index[c1]);
                temp2=temp2*(cap_out-cap_index[c1]);
                temp2=temp2+nor_trans_rise[t2][c1];

                temp=(temp2-temp1)/(time_index[t2]-time_index[t1]);
                temp=temp*(trans_time-time_index[t1]);
                temp=temp+temp1;
                ptr->trans_out=temp;                 
            }

        }
        else if(ptr->ZN.first==0)   //Output is falling
        {
            if(t2==-1 && c2==-1)
            {
                ptr->cell_del=nor_delay_fall[t1][c1];
                ptr->trans_out=nor_trans_fall[t1][c1];
            }
            else if(t2==-1 && c2!=-1)
            {
                temp = ((nor_delay_fall[t1][c2]-nor_delay_fall[t1][c1])/(cap_index[c2]-cap_index[c1]));
                temp=temp*(cap_out-cap_index[c1]);
                temp=temp+nor_delay_fall[t1][c1];
                ptr->cell_del=temp;
                temp = ((nor_trans_fall[t1][c2]-nor_trans_fall[t1][c1])/(cap_index[c2]-cap_index[c1]));
                temp=temp*(cap_out-cap_index[c1]);
                temp=temp+nor_trans_fall[t1][c1];
                ptr->trans_out=temp;
            }
            else if(t2!=-1 && c2==-1)
            {
                temp=(nor_delay_fall[t2][c1]-nor_delay_fall[t1][c1])/(time_index[t2]-time_index[t1]);
                temp=temp*(trans_time-time_index[t1]);
                temp=temp+nor_delay_fall[t1][c1];
                ptr->cell_del=temp;
                temp=(nor_trans_fall[t2][c1]-nor_trans_fall[t1][c1])/(time_index[t2]-time_index[t1]);
                temp=temp*(trans_time-time_index[t1]);
                temp=temp+nor_trans_fall[t1][c1];
                ptr->trans_out=temp;                
            }
            else if(t2!=-1 && c2!=-1)
            {
                temp1=(nor_delay_fall[t1][c2]-nor_delay_fall[t1][c1])/(cap_index[c2]-cap_index[c1]);
                temp1=temp1*(cap_out-cap_index[c1]);
                temp1=temp1+nor_delay_fall[t1][c1];

                temp2=(nor_delay_fall[t2][c2]-nor_delay_fall[t2][c1])/(cap_index[c2]-cap_index[c1]);
                temp2=temp2*(cap_out-cap_index[c1]);
                temp2=temp2+nor_delay_fall[t2][c1];

                temp=(temp2-temp1)/(time_index[t2]-time_index[t1]);
                temp=temp*(trans_time-time_index[t1]);
                temp=temp+temp1;
                ptr->cell_del=temp;

                temp1=(nor_trans_fall[t1][c2]-nor_trans_fall[t1][c1])/(cap_index[c2]-cap_index[c1]);
                temp1=temp1*(cap_out-cap_index[c1]);
                temp1=temp1+nor_trans_fall[t1][c1];

                temp2=(nor_trans_fall[t2][c2]-nor_trans_fall[t2][c1])/(cap_index[c2]-cap_index[c1]);
                temp2=temp2*(cap_out-cap_index[c1]);
                temp2=temp2+nor_trans_fall[t2][c1];

                temp=(temp2-temp1)/(time_index[t2]-time_index[t1]);
                temp=temp*(trans_time-time_index[t1]);
                temp=temp+temp1;
                ptr->trans_out=temp;                 
            }
        }
        else
        {
            cout<<"error"<<endl;
        }
    }
    else if(ptr->gate=="NANDX1")
    {
        if(ptr->ZN.first==1)        //Output is rising
        {
            if(t2==-1 && c2==-1)
            {
                ptr->cell_del=nand_delay_rise[t1][c1];
                ptr->trans_out=nand_trans_rise[t1][c1];
            }
            else if(t2==-1 && c2!=-1)
            {
                temp = ((nand_delay_rise[t1][c2]-nand_delay_rise[t1][c1])/(cap_index[c2]-cap_index[c1]));
                temp=temp*(cap_out-cap_index[c1]);
                temp=temp+nand_delay_rise[t1][c1];
                ptr->cell_del=temp;
                temp = ((nand_trans_rise[t1][c2]-nand_trans_rise[t1][c1])/(cap_index[c2]-cap_index[c1]));
                temp=temp*(cap_out-cap_index[c1]);
                temp=temp+nand_trans_rise[t1][c1];
                ptr->trans_out=temp;
            }
            else if(t2!=-1 && c2==-1)
            {
                temp=(nand_delay_rise[t2][c1]-nand_delay_rise[t1][c1])/(time_index[t2]-time_index[t1]);
                temp=temp*(trans_time-time_index[t1]);
                temp=temp+nand_delay_rise[t1][c1];
                ptr->cell_del=temp;
                temp=(nand_trans_rise[t2][c1]-nand_trans_rise[t1][c1])/(time_index[t2]-time_index[t1]);
                temp=temp*(trans_time-time_index[t1]);
                temp=temp+nand_trans_rise[t1][c1];
                ptr->trans_out=temp;                
            }
            else if(t2!=-1 && c2!=-1)
            {
                temp1=(nand_delay_rise[t1][c2]-nand_delay_rise[t1][c1])/(cap_index[c2]-cap_index[c1]);
                temp1=temp1*(cap_out-cap_index[c1]);
                temp1=temp1+nand_delay_rise[t1][c1];

                temp2=(nand_delay_rise[t2][c2]-nand_delay_rise[t2][c1])/(cap_index[c2]-cap_index[c1]);
                temp2=temp2*(cap_out-cap_index[c1]);
                temp2=temp2+nand_delay_rise[t2][c1];

                temp=(temp2-temp1)/(time_index[t2]-time_index[t1]);
                temp=temp*(trans_time-time_index[t1]);
                temp=temp+temp1;
                ptr->cell_del=temp;

                temp1=(nand_trans_rise[t1][c2]-nand_trans_rise[t1][c1])/(cap_index[c2]-cap_index[c1]);
                temp1=temp1*(cap_out-cap_index[c1]);
                temp1=temp1+nand_trans_rise[t1][c1];

                temp2=(nand_trans_rise[t2][c2]-nand_trans_rise[t2][c1])/(cap_index[c2]-cap_index[c1]);
                temp2=temp2*(cap_out-cap_index[c1]);
                temp2=temp2+nand_trans_rise[t2][c1];

                temp=(temp2-temp1)/(time_index[t2]-time_index[t1]);
                temp=temp*(trans_time-time_index[t1]);
                temp=temp+temp1;
                ptr->trans_out=temp;                 
            }
        }
        else if(ptr->ZN.first==0)   //Output is falling
        {
            if(t2==-1 && c2==-1)
            {
                ptr->cell_del=nand_delay_fall[t1][c1];
                ptr->trans_out=nand_trans_fall[t1][c1];
            }
            else if(t2==-1 && c2!=-1)
            {
                temp = ((nand_delay_fall[t1][c2]-nand_delay_fall[t1][c1])/(cap_index[c2]-cap_index[c1]));
                temp=temp*(cap_out-cap_index[c1]);
                temp=temp+nand_delay_fall[t1][c1];
                ptr->cell_del=temp;
                temp = ((nand_trans_fall[t1][c2]-nand_trans_fall[t1][c1])/(cap_index[c2]-cap_index[c1]));
                temp=temp*(cap_out-cap_index[c1]);
                temp=temp+nand_trans_fall[t1][c1];
                ptr->trans_out=temp;
            }
            else if(t2!=-1 && c2==-1)
            {
                temp=(nand_delay_fall[t2][c1]-nand_delay_fall[t1][c1])/(time_index[t2]-time_index[t1]);
                temp=temp*(trans_time-time_index[t1]);
                temp=temp+nand_delay_fall[t1][c1];
                ptr->cell_del=temp;
                temp=(nand_trans_fall[t2][c1]-nand_trans_fall[t1][c1])/(time_index[t2]-time_index[t1]);
                temp=temp*(trans_time-time_index[t1]);
                temp=temp+nand_trans_fall[t1][c1];
                ptr->trans_out=temp;                
            }
            else if(t2!=-1 && c2!=-1)
            {
                temp1=(nand_delay_fall[t1][c2]-nand_delay_fall[t1][c1])/(cap_index[c2]-cap_index[c1]);
                temp1=temp1*(cap_out-cap_index[c1]);
                temp1=temp1+nand_delay_fall[t1][c1];

                temp2=(nand_delay_fall[t2][c2]-nand_delay_fall[t2][c1])/(cap_index[c2]-cap_index[c1]);
                temp2=temp2*(cap_out-cap_index[c1]);
                temp2=temp2+nand_delay_fall[t2][c1];

                temp=(temp2-temp1)/(time_index[t2]-time_index[t1]);
                temp=temp*(trans_time-time_index[t1]);
                temp=temp+temp1;
                ptr->cell_del=temp;

                temp1=(nand_trans_fall[t1][c2]-nand_trans_fall[t1][c1])/(cap_index[c2]-cap_index[c1]);
                temp1=temp1*(cap_out-cap_index[c1]);
                temp1=temp1+nand_trans_fall[t1][c1];

                temp2=(nand_trans_fall[t2][c2]-nand_trans_fall[t2][c1])/(cap_index[c2]-cap_index[c1]);
                temp2=temp2*(cap_out-cap_index[c1]);
                temp2=temp2+nand_trans_fall[t2][c1];

                temp=(temp2-temp1)/(time_index[t2]-time_index[t1]);
                temp=temp*(trans_time-time_index[t1]);
                temp=temp+temp1;
                ptr->trans_out=temp;                 
            }
        }
        else
        {
            cout<<"error"<<endl;
        }
    }
    else                            // Inverter - Not Gate
    {
        if(ptr->ZN.first==1)        //Output is rising
        {
            if(t2==-1 && c2==-1)
            {
                ptr->cell_del=inv_delay_rise[t1][c1];
                ptr->trans_out=inv_trans_rise[t1][c1];
            }
            else if(t2==-1 && c2!=-1)
            {
                temp = ((inv_delay_rise[t1][c2]-inv_delay_rise[t1][c1])/(cap_index[c2]-cap_index[c1]));
                temp=temp*(cap_out-cap_index[c1]);
                temp=temp+inv_delay_rise[t1][c1];
                ptr->cell_del=temp;
                temp = ((inv_trans_rise[t1][c2]-inv_trans_rise[t1][c1])/(cap_index[c2]-cap_index[c1]));
                temp=temp*(cap_out-cap_index[c1]);
                temp=temp+inv_trans_rise[t1][c1];
                ptr->trans_out=temp;
            }
            else if(t2!=-1 && c2==-1)
            {
                temp=(inv_delay_rise[t2][c1]-inv_delay_rise[t1][c1])/(time_index[t2]-time_index[t1]);
                temp=temp*(trans_time-time_index[t1]);
                temp=temp+inv_delay_rise[t1][c1];
                ptr->cell_del=temp;
                temp=(inv_trans_rise[t2][c1]-inv_trans_rise[t1][c1])/(time_index[t2]-time_index[t1]);
                temp=temp*(trans_time-time_index[t1]);
                temp=temp+inv_trans_rise[t1][c1];
                ptr->trans_out=temp;                
            }
            else if(t2!=-1 && c2!=-1)
            {
                temp1=(inv_delay_rise[t1][c2]-inv_delay_rise[t1][c1])/(cap_index[c2]-cap_index[c1]);
                temp1=temp1*(cap_out-cap_index[c1]);
                temp1=temp1+inv_delay_rise[t1][c1];

                temp2=(inv_delay_rise[t2][c2]-inv_delay_rise[t2][c1])/(cap_index[c2]-cap_index[c1]);
                temp2=temp2*(cap_out-cap_index[c1]);
                temp2=temp2+inv_delay_rise[t2][c1];

                temp=(temp2-temp1)/(time_index[t2]-time_index[t1]);
                temp=temp*(trans_time-time_index[t1]);
                temp=temp+temp1;
                ptr->cell_del=temp;

                temp1=(inv_trans_rise[t1][c2]-inv_trans_rise[t1][c1])/(cap_index[c2]-cap_index[c1]);
                temp1=temp1*(cap_out-cap_index[c1]);
                temp1=temp1+inv_trans_rise[t1][c1];

                temp2=(inv_trans_rise[t2][c2]-inv_trans_rise[t2][c1])/(cap_index[c2]-cap_index[c1]);
                temp2=temp2*(cap_out-cap_index[c1]);
                temp2=temp2+inv_trans_rise[t2][c1];

                temp=(temp2-temp1)/(time_index[t2]-time_index[t1]);
                temp=temp*(trans_time-time_index[t1]);
                temp=temp+temp1;
                ptr->trans_out=temp;                 
            }
        }
        else if(ptr->ZN.first==0)   //Output is falling
        {
            if(t2==-1 && c2==-1)
            {
                ptr->cell_del=inv_delay_fall[t1][c1];
                ptr->trans_out=inv_trans_fall[t1][c1];
            }
            else if(t2==-1 && c2!=-1)
            {
                temp = ((inv_delay_fall[t1][c2]-inv_delay_fall[t1][c1])/(cap_index[c2]-cap_index[c1]));
                temp=temp*(cap_out-cap_index[c1]);
                temp=temp+inv_delay_fall[t1][c1];
                ptr->cell_del=temp;
                temp = ((inv_trans_fall[t1][c2]-inv_trans_fall[t1][c1])/(cap_index[c2]-cap_index[c1]));
                temp=temp*(cap_out-cap_index[c1]);
                temp=temp+inv_trans_fall[t1][c1];
                ptr->trans_out=temp;
            }
            else if(t2!=-1 && c2==-1)
            {
                temp=(inv_delay_fall[t2][c1]-inv_delay_fall[t1][c1])/(time_index[t2]-time_index[t1]);
                temp=temp*(trans_time-time_index[t1]);
                temp=temp+inv_delay_fall[t1][c1];
                ptr->cell_del=temp;
                temp=(inv_trans_fall[t2][c1]-inv_trans_fall[t1][c1])/(time_index[t2]-time_index[t1]);
                temp=temp*(trans_time-time_index[t1]);
                temp=temp+inv_trans_fall[t1][c1];
                ptr->trans_out=temp;                
            }
            else if(t2!=-1 && c2!=-1)
            {
                temp1=(inv_delay_fall[t1][c2]-inv_delay_fall[t1][c1])/(cap_index[c2]-cap_index[c1]);
                temp1=temp1*(cap_out-cap_index[c1]);
                temp1=temp1+inv_delay_fall[t1][c1];

                temp2=(inv_delay_fall[t2][c2]-inv_delay_fall[t2][c1])/(cap_index[c2]-cap_index[c1]);
                temp2=temp2*(cap_out-cap_index[c1]);
                temp2=temp2+inv_delay_fall[t2][c1];

                temp=(temp2-temp1)/(time_index[t2]-time_index[t1]);
                temp=temp*(trans_time-time_index[t1]);
                temp=temp+temp1;
                ptr->cell_del=temp;

                temp1=(inv_trans_fall[t1][c2]-inv_trans_fall[t1][c1])/(cap_index[c2]-cap_index[c1]);
                temp1=temp1*(cap_out-cap_index[c1]);
                temp1=temp1+inv_trans_fall[t1][c1];

                temp2=(inv_trans_fall[t2][c2]-inv_trans_fall[t2][c1])/(cap_index[c2]-cap_index[c1]);
                temp2=temp2*(cap_out-cap_index[c1]);
                temp2=temp2+inv_trans_fall[t2][c1];

                temp=(temp2-temp1)/(time_index[t2]-time_index[t1]);
                temp=temp*(trans_time-time_index[t1]);
                temp=temp+temp1;
                ptr->trans_out=temp;                 
            }
        }
        else
        {
            cout<<"error"<<endl;
        }
    }

}
void longShortPath(vector<vector<string>>& full_path,vector<node*>& prim_out_pointer, fstream& delay_path)
{
    sort(prim_out_pointer.begin(), prim_out_pointer.end(),compare_delay);
    for(int i=0;i<full_path.size();i++)
    {
        if(prim_out_pointer[0]->ZN.second==full_path[i][0])
        {
            long_path=full_path[i];
        }
        else if(prim_out_pointer[prim_out_pointer.size()-1]->ZN.second==full_path[i][0])
        {
            short_path=full_path[i];
        }
        if(long_path.size()!=0 && short_path.size()!=0)
        {
            break;
        }
    }

    delay_path<<"Longest delay = "<<prim_out_pointer[0]->delay_out<<", the path is: ";
    for(int i=long_path.size()-1;i>=0;i--)
    {
        if(i==0)
            delay_path<<long_path[i]<<endl;
        else
            delay_path<<long_path[i]<<" -> ";
    }
    delay_path<<"Shortest delay = "<<prim_out_pointer[prim_out_pointer.size()-1]->delay_out<<", the path is: ";
    for(int i=short_path.size()-1;i>=0;i--)
    {
        if(i==0)
            delay_path<<short_path[i];
        else
            delay_path<<short_path[i]<<" -> ";
    }
}
void detPaths(vector<node*>& prim_out_pointer)
{
    for(int i=0;i<prim_out_pointer.size();i++)
    {
        vector<string> single_path;
        node* ptr=prim_out_pointer[i];
        single_path.push_back(ptr->ZN.second);
        while(ptr!=NULL)
        {
            if((ptr->cell_del+ptr->delay_a1 == ptr->delay_out) && ptr->gate!="INVX1" && (ptr->cell_del+ptr->delay_a2 == ptr->delay_out))
            {
                if(ptr->gate=="NOR2X1")
                {
                    if(ptr->A1.first==1)
                    {
                        single_path.push_back(ptr->A1.second);
                        ptr=ptr->prev_1;
                    }
                    else
                    {  
                        single_path.push_back(ptr->A2.second);
                        ptr=ptr->prev_2;
                    }
                }
                else if(ptr->gate=="NANDX1")
                {
                    if(ptr->A1.first==0)
                    {
                        single_path.push_back(ptr->A1.second);
                        ptr=ptr->prev_1;
                    }
                    else
                    {   
                        single_path.push_back(ptr->A2.second);
                        ptr=ptr->prev_2;
                    }
                }                 
            }
            else if(ptr->cell_del+ptr->delay_a1 == ptr->delay_out)
            {
                single_path.push_back(ptr->A1.second);
                ptr=ptr->prev_1;
            }
            else if(ptr->cell_del+ptr->delay_a2 == ptr->delay_out)
            {
                single_path.push_back(ptr->A2.second);
                ptr=ptr->prev_2;
            }            
        }
        full_path.push_back(single_path);
    }
}