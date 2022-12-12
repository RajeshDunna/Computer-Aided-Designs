#include<iostream>
#include<vector>
#include<algorithm>
#include<math.h>
#include<stdlib.h>
#include<map>
#include<string>
#include<fstream>
#include<sstream>
#include<iterator>
using namespace std;
vector<int> prime_imp;
vector<int> minimized_imp;

bool myCompare(pair<int,int> p1,pair<int,int> p2)
{
    return p1.first<p2.first;
}
bool m_compare(vector<int> v1, vector<int> v2)
{
    return v1.size()<v2.size();
}
bool compare_desc_len(vector<int> v1, vector<int> v2)
{
    return v1.size()>v2.size();
}
void convertToBinary(vector<pair<int,int>> & p)
{
    int value,count=0,count1=0,quotient, remainder;
    int binary_val=0;
    for(int i=0;i<p.size();i++)
    {
        value=p[i].second;
        while(value!=0)
        {
            remainder=value%2;
            value=value/2;
            if(remainder==1)
            {
                count1++;
                binary_val=binary_val+pow(10,count);
            }
                
            count++;
        }
        p[i].first=count1;
        p[i].second=binary_val;
        count=0;
        count1=0;                //resetting the values
        binary_val=0;            //resetting the values
    }
    sort(p.begin(),p.end(),myCompare);
}
void groupingFirst(vector<pair<int,int>> & p, vector<vector<int>> & grouped)
{
    vector<int> single_grp;
    int join=p[0].first;

    for(int i=0;i<p.size();i++)
    {
        if(join==p[i].first)
        {
            single_grp.push_back(p[i].second);
            single_grp.push_back(-1);
        }
        else
        {
            grouped.push_back(single_grp);
            single_grp.clear();
            join=p[i].first;
            single_grp.push_back(p[i].second);
            single_grp.push_back(-1);
        }
    }
    grouped.push_back(single_grp);
}
int compareAdj(int first, int second)
{
    int count=0,position, bit_pos=0;         //Position starting at zeroth bit
    while(first!=0 || second!=0)
    {
        if(first%10 != second%10)
        {
            count++;
            position=bit_pos;
            if(count>1)
                return -10;      //If number of different bits are more than 1
        }
        first=first/10;
        second=second/10;
        bit_pos++;
    }
    return position;
}
void grouping(vector<vector<int>>* first_grp, vector<vector<int>>* next_grp)
{
    if((*first_grp).size()==0)
        return;

    int first, second, position;
    for(int i=0;i<(*first_grp).size()-1;i++)
    {
        vector<int> single_row;
        for(int j=0;j<(*first_grp)[i].size();j=j+2)
        {
            first=(*first_grp)[i][j];

            for(int k=0;k<(*first_grp)[i+1].size();k=k+2)
            {

                second=(*first_grp)[i+1][k];
                position = compareAdj(first,second);
                
                if(position<0)
                    continue;

                (*first_grp)[i+1][k+1]=3;        //status variable updated to true
                (*first_grp)[i][j+1]=3;          // status variable updated to true
                int divide=first/pow(10,position);
                int remainder=divide%10;            // gives the bit value of the position
                int temp;
                if(remainder==1)
                {
                    temp = first+pow(10,position);
                }
                else
                {
                    temp = first+2*pow(10,position);
                }
                if(find(single_row.begin(),single_row.end(),temp)==single_row.end())
                {
                    single_row.push_back(temp);
                    single_row.push_back(-1);  
                }
            }
            if((*first_grp)[i][j+1]==-1)
                 prime_imp.push_back((*first_grp)[i][j]);
        }
        

        if(single_row.size()!=0)            //Only pushes if the single_row is not equal to zero
            (*next_grp).push_back(single_row);
    }
    int l_index=(*first_grp).size()-1;
    for(int i=0;i<(*first_grp)[l_index].size();i=i+2)
    {
        if((*first_grp)[l_index][i+1]<0)
            prime_imp.push_back((*first_grp)[l_index][i]);
    }
    (*first_grp).clear();
    grouping(next_grp,first_grp);
}
int decimalEq(int b_number)
{
    int position=0, dec=0;
    while(b_number!=0)
    {
        if(b_number%10==1)
        {
            dec=dec+pow(2,position);
        }
        b_number=b_number/10;
        position++;
    }
    return dec;
}
void binaryToDecimal(int binary_val, vector<int> * bToDTemp)
{
    int num=binary_val, flag=0;
    int position =0, count=0;
    while(num!=0)
    {
        if(num%10==2)
        {
            position = count;
            binaryToDecimal((binary_val-(pow(10,position))),bToDTemp);     //121   111,101
            binaryToDecimal((binary_val-(2*pow(10,position))),bToDTemp);   //120   110,100
            return;
        }
        num=num/10;
        count++;
    }

    int dec_val=decimalEq(binary_val);
    (*bToDTemp).push_back(dec_val);

}
int reverse(int num)
{
    int temp,rev=0;
    while(num!=0)
    {
        rev=(rev*10)+(num%10);
        num=num/10;
    }
    return rev;
}
int convertToChar(vector<int> v, int width, map<string,int>& str_sorted)
{  
    int count_switch=0;
    for(int i=0;i<v.size();i++)
    {
        string s;
        int count=0;
        int rev=reverse(v[i]);
        int length = (v[i]==0)?1:(trunc(log10(v[i]))+1); 
        int zeros=width-length;
        if(zeros>0)
        {
            while(zeros!=0)
            {
                s=s+to_string(0);
                count++;
                zeros--;
            } 
        }
        while(rev!=0)
        {
            if(rev%10==2)
            {
                s=s+'-';
                count_switch++;
            }
            else
                s=s+to_string(rev%10);
            count++;
            rev=rev/10;
        }
        while(count<width)
        {
            s=s+'0';
            count++;
        }    
        str_sorted.insert(pair<string,int>(s,v[i]));
    }
    return count_switch;
}
void mintermDataStr(vector<vector<int>>& pi_numbers,vector<int>& minterms,vector<vector<int>>& minterm_grp)
{
    for(int i=0;i<minterms.size();i++)
    {   
        vector<int> single_row;
        single_row.push_back(minterms[i]);
        for(int k=0;k<pi_numbers.size();k++)
            if(find(pi_numbers[k].begin(),pi_numbers[k].end(),minterms[i])!=pi_numbers[k].end())    
                single_row.push_back(pi_numbers[k][0]);

        minterm_grp.push_back(single_row);

    }
}
void removeDC(vector<vector<int>>& pi_numbers, vector<int>& dont_care)
{

    for(int i=0;i<pi_numbers.size();i++)
    {
        for(int j=0;j<pi_numbers[i].size();j++)
        {
            if(find(dont_care.begin(), dont_care.end(), pi_numbers[i][j])!=dont_care.end())
            {
                pi_numbers[i].erase(pi_numbers[i].begin()+j);
                j--;
            }
        }
    }
    for(int i=0;i<pi_numbers.size();i++)
    {
            if(pi_numbers[i].size()==1)
            {
                pi_numbers.erase(pi_numbers.begin()+i);
                i--;
            }
    }
}
void modify(vector<int>:: iterator it_front, vector<int>:: iterator it_end ,vector<vector<int>>* pi_numbers)
{
    if((*pi_numbers).size()==0 || it_front==it_end)
    return;

    for(int i=0;i<(*pi_numbers).size();i++)
    {
        if((*pi_numbers)[i][0]!=-1)
        {
            for(int j=1;j<(*pi_numbers)[i].size();j++)
            {
                if(find(it_front,it_end,(*pi_numbers)[i][j])!=it_end)
                {
                    (*pi_numbers)[i].erase((*pi_numbers)[i].begin()+j);
                    j--;
                }
            }
        }
    }
}
void modifyDataStr(vector<vector<int>>* pi_numbers, vector<vector<int>>* minterm_grp, int grp)
{
    if((*pi_numbers).size()==0 || (*minterm_grp).size()==0)
    return;

    for(int i=0;i<(*pi_numbers).size();i++)
    {
        if((*pi_numbers)[i][0]==grp)
        {    
            for(int k=0;k<(*minterm_grp).size();k++)
            {
                if(find((*pi_numbers)[i].begin(),(*pi_numbers)[i].end(),(*minterm_grp)[k][0])!=(*pi_numbers)[i].end())
                {
                    (*minterm_grp).erase((*minterm_grp).begin()+k);
                    k--;
                }
            }
            (*pi_numbers)[i][0]=-1;
            modify((*pi_numbers)[i].begin(),(*pi_numbers)[i].end(),pi_numbers);
            break;           
        }
    }

    for(int i=0;i<(*pi_numbers).size();i++)
    {
        if((*pi_numbers)[i][0]==-1 || (*pi_numbers)[i].size()==1)
        {
            (*pi_numbers).erase((*pi_numbers).begin()+i);
        }
    }
}

void rmEssentialPrime(vector<vector<int>>* pi_numbers,vector<vector<int>>* minterm_grp)
{
    if((*pi_numbers).size()==0 || (*minterm_grp).size()==0)
    return;

        sort((*minterm_grp).begin(),(*minterm_grp).end(),m_compare);
       if((*minterm_grp)[0].size()==2)
        {
            minimized_imp.push_back((*minterm_grp)[0][1]);
            modifyDataStr(pi_numbers,minterm_grp,(*minterm_grp)[0][1]);
            rmEssentialPrime(pi_numbers,minterm_grp);
        }            
        else
        return;

}
void deleteAfterColDom(vector<vector<int>>* minterm_grp, int del_grp)
{
    if((*minterm_grp).size()==0)
    return;

    for(int i=0;i<(*minterm_grp).size();i++)
    {
        for(int j=0;j<(*minterm_grp)[i].size();j++)
        {
            if((*minterm_grp)[i][j]==del_grp)
            {
                (*minterm_grp)[i].erase((*minterm_grp)[i].begin()+j);
                j--;
            }
        }
            
    }
}
void deleteAfterRowDom(vector<vector<int>>* pi_numbers, int del_grp)
{
    if((*pi_numbers).size()==0)
    return;

    for(int i=0;i<(*pi_numbers).size();i++)
    {
        for(int j=0;j<(*pi_numbers)[i].size();j++)
        {
            if((*pi_numbers)[i][j]==del_grp)
            {
                (*pi_numbers)[i].erase((*pi_numbers)[i].begin()+j);
                j--;
            }
        }
            
    }
}
void rowDominance(vector<vector<int>>* pi_numbers,vector<vector<int>>* minterm_grp)
{
    if((*pi_numbers).size()==0 || (*minterm_grp).size()==0)
    return;

    sort((*minterm_grp).begin(),(*minterm_grp).end(),m_compare);
    for(int i=0;i<(*minterm_grp).size();i++)
    {
        sort((*minterm_grp)[i].begin()+1,(*minterm_grp)[i].end());
    }
    for(int i=0;i<(*minterm_grp).size()-1;i++)
    {
        for(int j=i+1;j<(*minterm_grp).size();j++)
        {
            if(includes((*minterm_grp)[j].begin()+1, (*minterm_grp)[j].end(), (*minterm_grp)[i].begin()+1, (*minterm_grp)[i].end()))
            {
                deleteAfterRowDom(pi_numbers,(*minterm_grp)[j][0]);
                (*minterm_grp).erase((*minterm_grp).begin()+j);
                j--;
            }
        }
           
    }
}
void colDominance(vector<vector<int>>* pi_numbers,vector<vector<int>>* minterm_grp)
{
    if((*pi_numbers).size()==0 || (*minterm_grp).size()==0)
    return;

    sort((*pi_numbers).begin(),(*pi_numbers).end(),m_compare);
    for(int i=0;i<(*pi_numbers).size();i++)
    {
        sort((*pi_numbers)[i].begin()+1,(*pi_numbers)[i].end());
    }
    
    for(int i=0;i<(*pi_numbers).size()-1;i++)
    {
        for(int j=i+1;j<(*pi_numbers).size();j++)
        {       
            if(includes((*pi_numbers)[j].begin()+1, (*pi_numbers)[j].end(), (*pi_numbers)[i].begin()+1, (*pi_numbers)[i].end()))
            {
                deleteAfterColDom(minterm_grp,(*pi_numbers)[i][0]);
                (*pi_numbers).erase((*pi_numbers).begin()+i);
                j--;
            }
        }      
    }
}
void minimization(vector<vector<int>>* pi_numbers,vector<vector<int>>* minterm_grp)
{
    if((*pi_numbers).size()==0 || (*minterm_grp).size()==0)
        return;
    
    sort((*minterm_grp).begin(),(*minterm_grp).end(),m_compare);
    sort((*pi_numbers).begin(),(*pi_numbers).end(),m_compare);

    if((*minterm_grp)[0].size()==2)
    {   
        rmEssentialPrime(pi_numbers,minterm_grp);
    }
    colDominance(pi_numbers,minterm_grp);

    minimization(pi_numbers,minterm_grp);

    rowDominance(pi_numbers,minterm_grp);
    
    minimization(pi_numbers,minterm_grp);

}
int main(int argc, char* argv[])
{
    int n_bits;

    vector<pair<int,int>> minterms_dc;
    vector<int> dont_care,minterms;

    map<string,int> pi_sorted, min_pi_sorted;

    fstream inFile, outFile;
    
    if(argc!=3)
    {
        cout<<"Error in arguments"<<endl;
        exit(0);
    }
    
    inFile.open(argv[1],ios::in);
    if(!inFile.is_open())
    {
        cout<<"Error opening the file for taking input"<<endl;
        exit(0);
    }    
    string line;
    int line_number=0;
    vector<vector<int>> input_data;
    while (getline(inFile,line)) 
    {
        line_number++;
        if(line_number%2==1)
            continue;
        istringstream is(line);
        input_data.push_back(vector<int>(istream_iterator<int>(is),istream_iterator<int>()));
    } 
    inFile.close();
    n_bits=input_data[0][0];

    
    for(int i=1;i<input_data.size();i++)
    {
        for(int j=0;j<input_data[i].size();j++)
        {
            minterms_dc.push_back({0,input_data[i][j]});
        }
    }
    minterms=input_data[1];

    if(input_data.size()==3)
        dont_care=input_data[2];

    convertToBinary(minterms_dc);

    vector<vector<int>> merged;
    groupingFirst(minterms_dc,merged); //first group in merged

    vector<vector<int>> *first, *second;
    vector<vector<int>> next_merged;
    first=&merged;
    second=&next_merged;
    grouping(first,second);

    vector<int> bToDTemp;
    vector<vector<int>> pi_numbers;

    for(int i=0;i<prime_imp.size();i++)
    {

        bToDTemp.push_back(prime_imp[i]);
        binaryToDecimal(prime_imp[i],&bToDTemp);
        pi_numbers.push_back(bToDTemp);
        bToDTemp.clear();
    }

    if(input_data.size()==3)
        removeDC(pi_numbers,dont_care);
    
    vector<vector<int>> minterm_grp;
    
    mintermDataStr(pi_numbers,minterms,minterm_grp);    //stores only 
    sort(minterm_grp.begin(),minterm_grp.end(),m_compare);
      
    minimization(&pi_numbers,&minterm_grp);

    sort( minimized_imp.begin(), minimized_imp.end() );
    minimized_imp.erase( unique( minimized_imp.begin(), minimized_imp.end() ), minimized_imp.end());

    convertToChar(prime_imp, n_bits,pi_sorted);    //sorted solution will be stored in str_sorted 
    int c_switch = convertToChar(minimized_imp, n_bits,min_pi_sorted);

    outFile.open(argv[2],ios::out);
    map<string,int>:: iterator it;
    outFile<<".p "<<prime_imp.size()<<endl;
    int c=1;
    for(it=pi_sorted.begin();it!=pi_sorted.end();it++)
    {   
        if(c==16)
            break;
        outFile<<(*it).first<<endl;
        c++;
    }
    outFile<<endl;
    outFile<<".mc "<<minimized_imp.size()<<endl;
    for(it=min_pi_sorted.begin();it!=min_pi_sorted.end();it++)
    {
        outFile<<(*it).first<<endl;
    }
    outFile<<"literal="<<(minimized_imp.size()*n_bits)-c_switch;
    outFile.close();
    return 0;
}