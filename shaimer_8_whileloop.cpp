#include<iostream>
#include<cstdlib>
#include<ctime>
#include <stdexcept>
using namespace std;
int exp_table[256];
int log_table[256];
int GF=8;
int PP=11;
// 在Galois Field中的加法
int add(int a, int b)
{
	return a ^ b;
}
// 在Galois Field中的減法
int subtract(int a, int b)
{
	return a ^ b;
}
// 在Galois Field中的乘法
int multiply(int a, int b)
{
	if(a==0||b==0)
		return 0;
	else
		return exp_table[(log_table[a] + log_table[b]) % (GF-1)];
}
// 在Galois Field中的除法
int divide(int a, int b)
{
	if (b == 0)
		throw std::invalid_argument("Division by zero");
	else if((log_table[a] - log_table[b])<0)
		return exp_table[log_table[a] - log_table[b] + (GF-1)];
	else
		return exp_table[log_table[a] - log_table[b]];
}
int main()
{
	exp_table[0] = 1;
	log_table[0] = 1-GF;
	for (int i = 1; i < GF; i++)
	{
		exp_table[i] = exp_table[i-1] <<1;
		if (exp_table[i] >= GF)
			exp_table[i] ^= PP;
		log_table[exp_table[i]] = i;
	}
	cout<<"加法表:"<<endl<<"  0 1 2 3 4 5 6 7"<<endl<<"  ---------------"<<endl; 
	for(int i=0;i<GF;i++)
	{
		cout<<i<<"|";
		for(int j=0;j<GF;j++)
		{
			cout<<add(i,j)<<" ";		
		}
		cout<<endl;	
	}
	cout<<"乘法表:"<<endl<<"  0 1 2 3 4 5 6 7"<<endl<<"  ---------------"<<endl; 
	for(int i=0;i<GF;i++)
	{
		cout<<i<<"|";
		for(int j=0;j<GF;j++)
		{
			cout<<multiply(i,j)<<" ";		
		}
		cout<<endl;	
	}
	int a,b;
	while(1)
	{
		cout<<"請輸入a,b: ";
		cin>>a>>b;
		cout<<"a+b="<<add(a,b)<<endl;
		cout<<"a-b="<<subtract(a,b)<<endl;
		cout<<"a*b="<<multiply(a,b)<<endl;	
		cout<<"a/b="<<divide(a,b)<<endl;		
	}	
} 
