//---------------------------------------------------------------------------

#include <vcl.h>
#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>
#pragma hdrstop

#include "Unit1.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TForm1 *Form1;
int p[100];
int exp_table[8];
int log_table[8];
int data[10000]={0};
const int size = 8; // GF(2^3)中的元素數量 8
int GF=8;//8//256
int PP=11;//11//301
//---------------------------------------------------------------------------
__fastcall TForm1::TForm1(TComponent* Owner)
	: TForm(Owner)
{
	/*exp_table[0] = 1;
	for (int i = 1; i < GF; ++i)
	{
		int temp = exp_table[i-1] << 1;  // g(i) = g(i-1) * g
		//Memo1 -> Lines -> Add(IntToStr(temp));
		if (temp & size)
			temp ^= 0b1011; // 假設原始多項式為 x^3 + x^1 + 1
		exp_table[i] = temp;
		//Memo1 -> Lines -> Add(IntToStr(exp_table[i]));
	}
	for (int i = 1; i < GF; i++)
	{
		log_table[exp_table[i]] = i;
	} */
	exp_table[0] = 1;
	log_table[0] = 1-GF;
	for (int i = 1; i < GF; i++)
	{
		exp_table[i] = exp_table[i-1] <<1;
		if (exp_table[i] >= GF)
			exp_table[i] ^= PP;
		log_table[exp_table[i]] = i;
	}
	/*for(int i=0;i<8;i++)
	{
		Memo1 -> Lines -> Add(IntToStr(exp_table[i])+","+IntToStr(log_table[i]));
	}*/
}
void resetSeed()
{
    srand(static_cast<unsigned int>(time(nullptr)));
}
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
//---------------------------------------------------------------------------
void __fastcall TForm1::Button1Click(TObject *Sender)
{
	int secret=StrToInt(Edit1->Text);
	int n=StrToInt(Edit2->Text);
	int k=StrToInt(Edit3->Text);
	if(n<k)
	{
		ShowMessage("n應大於等於k");
		return;
	}
	resetSeed();
	for(int i=0;i<k-1;)
	{
		p[i]=rand()%GF;
		//p[i]=random(8);
		if(p[i]!=0)
		{
			Memo1 -> Lines -> Add(IntToStr(i)+","+IntToStr(p[i]));
			i++;
		}
	}
	for(int i=1;i<=n;i++)
	{
		int m;
		for(int j=0;j<k;j++)
		{
			if(j==0)
			{
				data[i]=secret;
				//ShowMessage(IntToStr(data[i]));
			}
			else
			{
				m=1;
				for(int l=0;l<j;l++)
				{
					m=multiply(m, i);
					//ShowMessage(IntToStr(m));
				}
				data[i]=add(data[i],multiply(m, p[j-1]));
				//ShowMessage(IntToStr(data[i]));
			}
		}
		//AddToListBox(data[i] , "("+IntToStr(i) +","+IntToStr(data[i])+")");
		Memo1 -> Lines -> Add("("+IntToStr(i)+","+IntToStr(data[i])+")");
	}
	ListBox1->Clear();
	for (int i=1; i<=n; i++)
		ListBox1->Items->Add("Share "+IntToStr(i));
}
//---------------------------------------------------------------------------
void __fastcall TForm1::Button2Click(TObject *Sender)
{
	int secret=StrToInt(Edit1->Text);
	int n=StrToInt(Edit2->Text);
	int k=StrToInt(Edit3->Text);
	int t=StrToInt(Edit4->Text);
	if(n<k)
	{
		ShowMessage("n應大於等於k");
		return;
	}
	Memo1 -> Lines -> Add("解密:");
	for (int i=0; i<ListBox1->Count; i++)
	{
		if(ListBox1->Selected[i])
		{
			Memo1 -> Lines -> Add("("+IntToStr(i+1)+","+IntToStr(data[i+1])+")");
		}
	}
	int s=0;
	for (int i=0; i<ListBox1->Count; i++)
	{
		if(ListBox1->Selected[i])
		{
			int temp=data[i+1];
			for (int j=0; j<ListBox1->Count; j++)
			{
				if(ListBox1->Selected[j])
				{
					if(j!=i)
					{
						temp=divide(multiply(temp,j+1),subtract(i+1, j+1));
					}
					//Memo1 -> Lines -> Add("temp是 : "+IntToStr(temp));
				}
			}
			s=add(s, temp);
			//Memo1 -> Lines -> Add("s是 : "+IntToStr(s));
		}
	}
	Memo1 -> Lines -> Add("密碼是 : "+IntToStr(s));
}
//---------------------------------------------------------------------------
