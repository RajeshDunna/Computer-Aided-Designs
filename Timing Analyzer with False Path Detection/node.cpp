#include<iostream>
#include<fstream>
#include<sstream>
#include<iterator>
#include<string>
#include"node.h"

vector<vector<double>> nand_trans_rise;
vector<vector<double>> nand_trans_fall;
vector<vector<double>> nand_delay_rise;
vector<vector<double>> nand_delay_fall;

vector<vector<double>> nor_trans_rise;
vector<vector<double>> nor_trans_fall;
vector<vector<double>> nor_delay_rise;
vector<vector<double>> nor_delay_fall;

vector<vector<double>> inv_trans_rise;
vector<vector<double>> inv_trans_fall;
vector<vector<double>> inv_delay_rise;
vector<vector<double>> inv_delay_fall;

vector<double> cap_index;
vector<double> time_index;

double inv_i;
double nand_a1, nand_a2;
double nor_a1, nor_a2;

int truthTable(string gate, int x1, int x2)
{
    int y;
    if(gate=="INVX1")
    {
        if(x1==0)
            y=1;
        else
            y=0;
    }
    else if(gate =="NANDX1")
    {
        if(x1==0 || x2==0)
            y=1;
        else
            y=0;
    }
    else
    {
        if(x1==0 && x2==0)
            y=1;
        else
            y=0;
    }
    return y;
}

void parseNode(fstream& netlist, vector<string>& full_data)
{
    string str;
    string delims{ " .,:;()\t'//''/*''*/'" };
    string v_data;
    while(!netlist.eof())
    {
        getline(netlist,str);
        size_t beg=0, pos = 0;
        int flag=0;
        while((beg = str.find_first_of(" .,:;()\t'//''/*''*/'", pos)) !=string::npos)
        {
            if((pos = str.find_first_not_of(" .,:;()\t'//''/*''*/'", beg+1))!=string::npos)
            {
                v_data = str.substr(beg, pos - beg);
                if (v_data.find("//") != string::npos)
                {
                    size_t start_pos = str.find("//",beg);
                    str.erase(start_pos,string::npos);
                    break;
                }
                else if(v_data.find("/*") != string::npos)
                {
                    size_t start_pos = str.find("/*",beg);
                    while(!netlist.eof())
                    {
                         size_t end_pos = str.find("*/",start_pos+2);
                         if(end_pos==string::npos)
                         {
                            if(flag==1)
                            {
                                str.erase(0,string::npos);
                            }
                            else
                            {
                                str.erase(start_pos,string::npos);
                                if(str.size()!=0)
                                {
                                    size_t beg1, pos1 = 0;
                                    while ((beg1 = str.find_first_not_of(" .,:;()\t'//''/*''*/'", pos1)) !=string::npos)
                                    {
                                        pos1 = str.find_first_of(" .,:;()\t'//''/*''*/'", beg1 + 1);
                                        v_data = str.substr(beg1, pos1 - beg1);
                                        full_data.push_back(v_data);
                                    }
                                }
                            }
                            start_pos=-2;
                         }
                         else                       // This is the case when */ is found
                         {
                            if(start_pos!=-2)
                            {
                                str.erase(start_pos,(end_pos+2)-start_pos);
                                break;
                            }
                            else
                            {
                                str.erase(0,(end_pos+2));
                                break;
                            }
                         }
                         getline(netlist,str);
                         flag=1;
                    }
                    if(start_pos==-2)
                        pos=0;
                    else
                        pos=start_pos;
                }
            }
            
        }   

        size_t beg1, pos1 = 0;
        while ((beg1 = str.find_first_not_of(" .,:;()\t'//''/*''*/'", pos1)) !=string::npos)
        {
            
            pos1 = str.find_first_of(" .,:;()\t'//''/*''*/'", beg1 + 1);
            v_data = str.substr(beg1, pos1 - beg1);
            full_data.push_back(v_data);
        }
    }  
}

void separate(vector<string>& full_data, vector<string>& prim_input,vector<string>& prim_out,vector<string>& wire,vector<string>& circuit,string& case_name)
{
    int flag_in, flag_out, flag_wire;
    case_name = full_data[1];

    int i;
    for(i=2; full_data[i]!="input" && full_data[i]!="output" && full_data[i]!="wire";i++)
    {}

    while(full_data[i]!="NOR2X1" && full_data[i]!="INVX1" && full_data[i]!="NANDX1")
    {
        if(full_data[i]=="input")
        {
            flag_in=1;
            flag_out=0;
            flag_wire=0;
        }
        else if(full_data[i]=="output")
        {
            flag_in=0;
            flag_out=1;
            flag_wire=0;
        }
        else if(full_data[i]=="wire")
        {
            flag_in=0;
            flag_out=0;
            flag_wire=1;
        }
        else
        {
            if(flag_in==1)
            {
                prim_input.push_back(full_data[i]);
            }
            else if(flag_out==1)
            {
                prim_out.push_back(full_data[i]);
            }
            else if(flag_wire==1)
            {
                wire.push_back(full_data[i]);
            }
        }
        i++;
    }

    while(full_data[i]!="endmodule")
    {
        circuit.push_back(full_data[i]);
        i++;
        
    }
    
}

void parseInput(fstream& input_pat,vector<string>& input_pattern, vector<vector<int>>& input_data, int n_inputs)
{

    string str;
    string delims{ " .,:;()\t'//''/*''*/'" };
    string v_data;
    vector<string> full_data;
    while(!input_pat.eof())
    {
        getline(input_pat,str);
        size_t beg=0, pos = 0;
        int flag=0;
        while((beg = str.find_first_of(" .,:;()\t'//''/*''*/'", pos)) !=string::npos)
        {
            if((pos = str.find_first_not_of(" .,:;()\t'//''/*''*/'", beg+1))!=string::npos)
            {
                v_data = str.substr(beg, pos - beg);
                if (v_data.find("//") != string::npos)
                {
                    size_t start_pos = str.find("//",beg);
                    str.erase(start_pos,string::npos);
                    break;
                }
                else if(v_data.find("/*") != string::npos)
                {
                    size_t start_pos = str.find("/*",beg);
                    while(!input_pat.eof())
                    {
                         size_t end_pos = str.find("*/",start_pos+2);
                         if(end_pos==string::npos)
                         {
                            if(flag==1)
                            {
                                str.erase(0,string::npos);
                            }
                            else
                            {
                                str.erase(start_pos,string::npos);
                                if(str.size()!=0)
                                {
                                    size_t beg1, pos1 = 0;
                                    while ((beg1 = str.find_first_not_of(" .,:;()\t'//''/*''*/'", pos1)) !=string::npos)
                                    {
                                        pos1 = str.find_first_of(" .,:;()\t'//''/*''*/'", beg1 + 1);
                                        v_data = str.substr(beg1, pos1 - beg1);
                                        full_data.push_back(v_data);
                                    }
                                }
                            }
                            start_pos=-2;
                         }
                         else                       // This is the case when */ is found
                         {
                            if(start_pos!=-2)
                            {
                                str.erase(start_pos,(end_pos+2)-start_pos);
                                break;
                            }
                            else
                            {
                                str.erase(0,(end_pos+2));
                                break;
                            }
                         }
                         getline(input_pat,str);
                         flag=1;
                    }
                    if(start_pos==-2)
                        pos=0;
                    else
                        pos=start_pos;
                }
            }
            
        }   

        size_t beg1, pos1 = 0;
        while ((beg1 = str.find_first_not_of(" .,:;()\t'//''/*''*/'", pos1)) !=string::npos)
        {
            
            pos1 = str.find_first_of(" .,:;()\t'//''/*''*/'", beg1 + 1);
            v_data = str.substr(beg1, pos1 - beg1);
            full_data.push_back(v_data);
        }
    }

    vector<int> single_row;
    for(int i=1;i<full_data.size()-1;i++)
    {
        if(input_pattern.size()!=n_inputs)
        {
            input_pattern.push_back(full_data[i]);
        }
        else if(single_row.size()==n_inputs)
        {
            input_data.push_back(single_row);
            single_row.clear();
            single_row.push_back(stoi(full_data[i]));
        }
        else
            single_row.push_back(stoi(full_data[i]));
    }
    input_data.push_back(single_row);
}
void parseLiberty(fstream& liberty, vector<string>& libfile)
{
    string str;
    string delims1{" ,:;('{')\t\'}'"};
    string v_data;
    int flag_nor=0, flag_nand=0, flag_not=0;
    int count=0;

    while(getline(liberty,str))
    {
        count++;// 
        if(count==1)
            continue;

        size_t beg, pos = 0;     
        while((beg = str.find_first_not_of(" ,:;(){}\t'\"''\\''\n'", pos)) !=string::npos)
        {
            if((pos = str.find_first_of(" ,:;(){}\t'\"''\\''\n'", beg + 1))!=string::npos)
            {
                v_data = str.substr(beg, pos - beg);
                libfile.push_back(v_data);
                if(v_data=="INVX1")
                {
                    flag_not=1;
                    flag_nor=0;
                    flag_nand=0;
                }
                else if(v_data=="NOR2X1")
                {
                    flag_nor=1;
                    flag_nand=0;
                    flag_not=0;
                }
                else if(v_data=="NANDX1")
                {
                    flag_nand=1;
                    flag_nor=0;
                    flag_not=0;
                }
            }
        }
    }  
}
void parseData(vector<string>& libfile)
{
    int flag_nor=0, flag_nand=0, flag_not=0;
    int i=0, columns, count=-1;
    while(i<libfile.size())
    {
        if(libfile[i]=="index_1")
        {
            i++;
            while(libfile[i]!="index_2")
            {
                cap_index.push_back(stod(libfile[i]));
                i++;
            }
            i++;
            while(libfile[i]!="cell")
            {
                time_index.push_back(stod(libfile[i]));
                i++;
            }
        }

        if(libfile[i]=="NOR2X1")
        {
            count=0;
            flag_nor=1;
            flag_nand=0;
            flag_not=0;
        }
        else if(libfile[i]=="NANDX1")
        {
            count=0;
            flag_nor=0;
            flag_nand=1;
            flag_not=0;
        }
        else if(libfile[i]=="INVX1")
        {
            count=0;
            flag_nor=0;
            flag_nand=0;
            flag_not=1;
        }
        if(flag_nor==1 && flag_nand==0 && flag_not==0)
        {
            
            if(libfile[i]=="capacitance" && (++count)==1)
            {
                i++;
                nor_a1=stod(libfile[i]);
            }
            else if(libfile[i]=="capacitance" && count==2)
            {
                i++;
                nor_a2=stod(libfile[i]);
            }
            else if(libfile[i]=="cell_rise")
            {
                i=i+2;
                for(int k=0;k<time_index.size();k++)
                {
                    vector<double> v;
                    for(int l=0;l<cap_index.size();l++)
                    {
                        i++;
                        v.push_back(stod(libfile[i]));
                    }
                    nor_delay_rise.push_back(v);
                }
            }
            else if(libfile[i]=="cell_fall")
            {
                i=i+2;
                for(int k=0;k<time_index.size();k++)
                {
                    vector<double> v;
                    for(int l=0;l<cap_index.size();l++)
                    {
                        i++;
                        v.push_back(stod(libfile[i]));
                    }
                    nor_delay_fall.push_back(v);
                }
            }
            else if(libfile[i]=="rise_transition")
            {
                i=i+2;
                for(int k=0;k<time_index.size();k++)
                {
                    vector<double> v;
                    for(int l=0;l<cap_index.size();l++)
                    {
                        i++;
                        v.push_back(stod(libfile[i]));
                    }
                    nor_trans_rise.push_back(v);
                }
            }
            else if(libfile[i]=="fall_transition")
            {
                i=i+2;
                for(int k=0;k<time_index.size();k++)
                {
                    vector<double> v;
                    for(int l=0;l<cap_index.size();l++)
                    {
                        i++;
                        v.push_back(stod(libfile[i]));
                    }
                    nor_trans_fall.push_back(v);
                }
            }

        }
        else if(flag_nor==0 && flag_nand==1 && flag_not==0)
        {
            
            if(libfile[i]=="capacitance" && (++count)==1)
            {
                i++;
                nand_a1=stod(libfile[i]);
            }
            else if(libfile[i]=="capacitance" && count==2)
            {
                i++;
                nand_a2=stod(libfile[i]);
            }
            else if(libfile[i]=="cell_rise")
            {
                i=i+2;
                for(int k=0;k<time_index.size();k++)
                {
                    vector<double> v;
                    for(int l=0;l<cap_index.size();l++)
                    {
                        i++;
                        v.push_back(stod(libfile[i]));
                    }
                    nand_delay_rise.push_back(v);
                }
            }
            else if(libfile[i]=="cell_fall")
            {
                i=i+2;
                for(int k=0;k<time_index.size();k++)
                {
                    vector<double> v;
                    for(int l=0;l<cap_index.size();l++)
                    {
                        i++;
                        v.push_back(stod(libfile[i]));
                    }
                    nand_delay_fall.push_back(v);
                }
            }
            else if(libfile[i]=="rise_transition")
            {
                i=i+2;
                for(int k=0;k<time_index.size();k++)
                {
                    vector<double> v;
                    for(int l=0;l<cap_index.size();l++)
                    {
                        i++;
                        v.push_back(stod(libfile[i]));
                    }
                    nand_trans_rise.push_back(v);
                }
            }
            else if(libfile[i]=="fall_transition")
            {
                i=i+2;
                for(int k=0;k<time_index.size();k++)
                {
                    vector<double> v;
                    for(int l=0;l<cap_index.size();l++)
                    {
                        i++;
                        v.push_back(stod(libfile[i]));
                    }
                    nand_trans_fall.push_back(v);
                }
            }
        }
        else if(flag_nor==0 && flag_nand==0 && flag_not==1)
        {
            
            if(libfile[i]=="capacitance" && (++count)==1)
            {
                i++;
                inv_i=stod(libfile[i]);
            }
            else if(libfile[i]=="cell_rise")
            {
                i=i+2;
                for(int k=0;k<time_index.size();k++)
                {
                    vector<double> v;
                    for(int l=0;l<cap_index.size();l++)
                    {
                        i++;
                        v.push_back(stod(libfile[i]));
                    }
                    inv_delay_rise.push_back(v);
                }
            }
            else if(libfile[i]=="cell_fall")
            {
                i=i+2;
                for(int k=0;k<time_index.size();k++)
                {
                    vector<double> v;
                    for(int l=0;l<cap_index.size();l++)
                    {
                        i++;
                        v.push_back(stod(libfile[i]));
                    }
                    inv_delay_fall.push_back(v);
                }
            }
            else if(libfile[i]=="rise_transition")
            {
                i=i+2;
                for(int k=0;k<time_index.size();k++)
                {
                    vector<double> v;
                    for(int l=0;l<cap_index.size();l++)
                    {
                        i++;
                        v.push_back(stod(libfile[i]));
                    }
                    inv_trans_rise.push_back(v);
                }
            }
            else if(libfile[i]=="fall_transition")
            {
                i=i+2;
                for(int k=0;k<time_index.size();k++)
                {
                    vector<double> v;
                    for(int l=0;l<cap_index.size();l++)
                    {
                        i++;
                        v.push_back(stod(libfile[i]));
                    }
                    inv_trans_fall.push_back(v);
                }
            }
        }
        i++;
    }
}