//---------------------------------------------------------------------------

#include <vcl.h>
#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>
#pragma hdrstop
#define SIZE 100
Graphics::TBitmap * * Z;
int indexZ = 0;

#include "Unit1.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TForm1 *Form1;
int p[100];
int exp_table1[8];
int log_table1[8];
int exp_table2[256];
int log_table2[256];
int data[1000]={0};
int ****datap ;
int n1=0;
int c=1;
int GF[2] = {8, 256};  // 使用大括號進行初始化
int PP[2] = {11, 285};
//---------------------------------------------------------------------------
__fastcall TForm1::TForm1(TComponent* Owner)
	: TForm(Owner)
{
	PageControl1->ActivePage = TabSheet1;
    PageControl2->ActivePage = TabSheet2;
    Z = new Graphics::TBitmap * [SIZE];
	for (int i = 0 ; i < SIZE ; i++)
	{
		Z[i] = new Graphics::TBitmap();
		Z[i] -> PixelFormat = pf24bit;
	}
	exp_table1[0] = 1;
	log_table1[0] = 1-GF[0];
	for (int i = 1; i < GF[0]; i++)
	{
		exp_table1[i] = exp_table1[i-1] <<1;
		if (exp_table1[i] >= GF[0])
			exp_table1[i] ^= PP[0];
		log_table1[exp_table1[i]] = i;
	}
	exp_table2[0] = 1;
	log_table2[0] = 1-GF[1];
	for (int i = 1; i < GF[1]; i++)
	{
		exp_table2[i] = exp_table2[i-1] <<1;
		if (exp_table2[i] >= GF[1])
			exp_table2[i] ^= PP[1];
		log_table2[exp_table2[i]] = i;
	}
}
void resetSeed()
{
    srand(static_cast<unsigned int>(time(nullptr)));
}
void allocateDatap(int ****&datap, int width, int height, int shares, int channels)
{
    datap = new int***[height];
	for (int i = 0; i < height; i++)
	{
        datap[i] = new int**[width];
		for (int j = 0; j < width; j++)
		{
            datap[i][j] = new int*[shares];
			for (int k = 0; k < shares; k++)
			{
                datap[i][j][k] = new int[channels];
            }
        }
    }
}
void releaseDatap(int ****&datap, int width, int height, int shares)
{
	for (int i = 0; i < height; i++)
	{
		for (int j = 0; j < width; j++)
		{
			for (int k = 0; k < shares; k++)
			{
                delete[] datap[i][j][k];
            }
            delete[] datap[i][j];
        }
        delete[] datap[i];
    }
    delete[] datap;
	datap = nullptr;  // 避免懸掛指標
}
# define K  (TColor)RGB(0 , 0 , 0)
# define W  (TColor)RGB(255 , 255 , 255)
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
int multiply(int a, int b, int c)
{
	if(a==0||b==0)
		return 0;
	else
	{
		if(c==0)
			return exp_table1[(log_table1[a] + log_table1[b]) % (GF[0]-1)];
		else
			return exp_table2[(log_table2[a] + log_table2[b]) % (GF[1]-1)];
	}
}
// 在Galois Field中的除法
int divide(int a, int b, int c)
{
	if (b == 0)
		throw std::invalid_argument("Division by zero");
	else
	{
		if(c==0)
		{
			if((log_table1[a] - log_table1[b])<0)
				return exp_table1[log_table1[a] - log_table1[b] + (GF[0]-1)];
			else
				return exp_table1[log_table1[a] - log_table1[b]];
		}
		else
		{
			if((log_table2[a] - log_table2[b])<0)
				return exp_table2[log_table2[a] - log_table2[b] + (GF[1]-1)];
			else
				return exp_table2[log_table2[a] - log_table2[b]];
        	}
	}
}
//---------------------------------------------------------------------------
Graphics::TBitmap * BMP, * Y, * P, * N, * M; // define global var.
String fname;
void __fastcall TForm1::Button1Click(TObject *Sender)
{
	PageControl1->ActivePage = TabSheet1;
	int secret=StrToInt(Edit1->Text);
	int n=StrToInt(Edit2->Text);
	int k=StrToInt(Edit3->Text);
	if(n<k)
	{
		ShowMessage("n應大於等於k");
		return;
	}
	if (RadioButton1->Checked)
		c=0;
	else
		c=1;
	resetSeed();
	for(int i=0;i<k-1;)
	{
		p[i]=rand()%GF[c];
		if(p[i]!=0)
		{
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
			}
			else
			{
				m=1;
				for(int l=0;l<j;l++)
				{
					m=multiply(m, i, c);
				}
				data[i]=add(data[i],multiply(m, p[j-1], c));
			}
		}
		Memo1 -> Lines -> Add("("+IntToStr(i)+" , "+IntToStr(data[i])+")");
	}
	ListBox1->Clear();
	for (int i=1; i<=n; i++)
		ListBox1->Items->Add("Share "+IntToStr(i));

}
//---------------------------------------------------------------------------
void __fastcall TForm1::Button2Click(TObject *Sender)
{
    int n=StrToInt(Edit2->Text);
	int k=StrToInt(Edit3->Text);
	PageControl1->ActivePage = TabSheet1;
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
			Memo1 -> Lines -> Add("("+IntToStr(i+1)+" , "+IntToStr(data[i+1])+")");
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
						temp=divide(multiply(temp,j+1,c),subtract(i+1, j+1),c);
					}
				}
			}
			s=add(s, temp);
		}
	}
	Memo1 -> Lines -> Add("密碼是 : "+IntToStr(s));
}
//---------------------------------------------------------------------------

void __fastcall TForm1::Button3Click(TObject *Sender)
{
	if (OpenPictureDialog1->Execute()) // 讀image
	{
		BMP = new Graphics::TBitmap();   //宣告TBitmap物件
		BMP -> PixelFormat = pf24bit;
		BMP -> LoadFromFile(OpenPictureDialog1 -> FileName); //使用TBitmap物件讀取圖檔
		Image1 -> Picture -> Assign(BMP);  //show 圖檔 to Image1
		//Edit1 -> Text = BMP -> Width;           //圖的寬
		//Edit2 -> Text = BMP -> Height;  //圖的高
		String dir = ExtractFileDir(OpenPictureDialog1 -> FileName);
		fname = OpenPictureDialog1 -> FileName.SubString(dir.Length()+2 , OpenPictureDialog1 -> FileName.Length()-dir.Length()-5);
		//ListBox1 -> Items -> Add(fname);
		Z[indexZ] = BMP;
		Z[indexZ] -> Width = BMP -> Width;
		Z[indexZ] -> Height = BMP -> Height;
		if (++indexZ == SIZE)
			Memo1 -> Lines -> Add("Image SIZE overflow!");
	}
	PageControl1->ActivePage = TabSheet4;
}
//---------------------------------------------------------------------------

void __fastcall TForm1::ListBox2Click(TObject *Sender)
{
	int ind = ListBox2 -> ItemIndex;    //判斷點到ListBox的哪一個
	P = new Graphics::TBitmap();
	P -> Width = BMP -> Width;
	P -> Height = BMP -> Height;
	for (int i=0 ; i<BMP->Height ; i++)
	{
		for (int j=0 ; j<BMP->Width ; j++)
		{
			P -> Canvas -> Pixels[j][i] = (TColor)RGB(datap[j][i][ind][0], datap[j][i][ind][1], datap[j][i][ind][2]);
		}
	}
	Image2 -> Picture -> Assign(P);
	PageControl1->ActivePage = TabSheet5;
}
//---------------------------------------------------------------------------

void __fastcall TForm1::Button4Click(TObject *Sender)
{
	Y = new Graphics::TBitmap();
	Y -> Width = BMP -> Width;
	Y -> Height = BMP -> Height;
    c=1;
	if (datap != nullptr)
	{
		releaseDatap(datap, BMP->Height, BMP->Width, n1); // 釋放之前分配的空間
		ShowMessage("釋放");
	}
	int n=StrToInt(Edit5->Text);
	int k=StrToInt(Edit6->Text);
	n1=n;
	allocateDatap(datap, BMP->Height, BMP->Width, n, 3); // 分配所需的空間
	for (int i=0 ; i<BMP->Height ; i++)
	{
		for (int j=0 ; j<BMP->Width ; j++)
		{
			int a[3];
			a[0]=GetRValue(BMP -> Canvas -> Pixels[j][i]); //R
			a[1]=GetGValue(BMP -> Canvas -> Pixels[j][i]); //G
			a[2]=GetBValue(BMP -> Canvas -> Pixels[j][i]); //B
			for(int h=0;h<3;h++)
			{
                		for(int l=0;l<k-1;)
				{
					p[l]=random(256);
					if(p[l]!=0)
					{
						l++;
					}
				}
				for(int z=1;z<=n;z++)
				{
					int m;
					for(int g=0;g<k;g++)
					{
						if(g==0)
						{
							datap[j][i][z-1][h]=a[h];
						}
						else
						{
							m=1;
							for(int l=0;l<g;l++)
							{
								m=multiply(m, z, c);
							}
							datap[j][i][z-1][h]=add(datap[j][i][z-1][h],multiply(m, p[g-1], c));
						}
					}
				}
			}
		}
	}
	ListBox2->Clear();
	for (int i=1; i<=n; i++)
		ListBox2->Items->Add("Share "+IntToStr(i));
	for (int i=0 ; i<BMP->Height ; i++)
	{
		for (int j=0 ; j<BMP->Width ; j++)
		{
			Y -> Canvas -> Pixels[j][i] = (TColor)RGB(datap[j][i][0][0], datap[j][i][0][1], datap[j][i][0][2]);
		}
	}
	Image2 -> Picture -> Assign(Y);
	PageControl1->ActivePage = TabSheet5;
}
//---------------------------------------------------------------------------

void __fastcall TForm1::Button6Click(TObject *Sender)
{
	/*for(int i=0;i<8;i++)
	{
		String temp1="";
		String temp2="";
		for(int j=0;j<8;j++)
		{
			if(add(i,j)==0)
				temp1=temp1+","+IntToStr(j);
			if(multiply(i,j)==1)
				temp2=temp2+"-"+IntToStr(j);

		}
		Memo1 -> Lines -> Add(IntToStr(exp_table[i])+","+IntToStr(log_table[i])+"  "+temp1+temp2);
	}*/
}
//---------------------------------------------------------------------------

void __fastcall TForm1::Button5Click(TObject *Sender)
{
	int rr,gg,bb;
	c=1;
	M = new Graphics::TBitmap();
	M -> Width = BMP -> Width;
	M -> Height = BMP -> Height;
	for (int i=0 ; i<BMP->Height ; i++)
	{
		for (int j=0 ; j<BMP->Width ; j++)
		{
			M -> Canvas -> Pixels[j][i] = (TColor)RGB(0, 0, 0);
		}
	}
	for (int i=0; i<ListBox2->Count; i++)
	{
		if(ListBox2->Selected[i])
		{
			for (int k=0 ; k<BMP->Height ; k++)
			{
				for (int z=0 ; z<BMP->Width ; z++)
				{
					int temp1=datap[z][k][i][0];//R
					int temp2=datap[z][k][i][1];//G
					int temp3=datap[z][k][i][2];//B
					for (int j=0; j<ListBox2->Count; j++)
					{
						if(ListBox2->Selected[j])
						{
							if(j!=i)
							{
								temp1=divide(multiply(temp1,j+1,c),subtract(i+1, j+1),c);
								temp2=divide(multiply(temp2,j+1,c),subtract(i+1, j+1),c);
								temp3=divide(multiply(temp3,j+1,c),subtract(i+1, j+1),c);
							}
						}
					}
					rr=add(GetRValue(M -> Canvas -> Pixels[z][k]), temp1);
					gg=add(GetGValue(M -> Canvas -> Pixels[z][k]), temp2);
					bb=add(GetBValue(M -> Canvas -> Pixels[z][k]), temp3);
					M -> Canvas -> Pixels[z][k] = (TColor)RGB(rr, gg, bb);
				}
			}
		}
	}
	Image3 -> Picture -> Assign(M);
    PageControl1->ActivePage = TabSheet6;
}
//---------------------------------------------------------------------------
