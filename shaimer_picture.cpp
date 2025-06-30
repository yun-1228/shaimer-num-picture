//---------------------------------------------------------------------------

#include <vcl.h>
#include <iostream>
#include <vector>
#include <cstdlib>
#include <time.h>
#include <ctime>
#include <cmath>
#pragma hdrstop
#define SIZE 100
Graphics::TBitmap * * Z;
int indexZ = 0;

#include "Unit1.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TForm1 *Form1;
unsigned char p[100];
unsigned char mark[256]={0};
unsigned short exp_table1[8];
unsigned short log_table1[8];
unsigned short exp_table2[256];
unsigned short log_table2[256];
unsigned char data[1000]={0};
unsigned char ****datap ;
unsigned char n1=0;
bool c=1;
unsigned short GF[2] = {8, 256};
unsigned short PP[2] = {11, 285};
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
void allocateDatap(unsigned char ****&datap, int width, int height, int shares, int channels)
{
	datap = new unsigned char***[height];
	for (int i = 0; i < height; i++)
	{
		datap[i] = new unsigned char**[width];
		for (int j = 0; j < width; j++)
		{
			datap[i][j] = new unsigned char*[shares];
			for (int k = 0; k < shares; k++)
			{
				datap[i][j][k] = new unsigned char[channels];
            }
        }
    }
}
void releaseDatap(unsigned char ****&datap, int width, int height, int shares)
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
unsigned char add(unsigned char a, unsigned char b)
{
	return a ^ b;
}
// 在Galois Field中的減法
unsigned char subtract(unsigned char a, unsigned char b)
{
	return a ^ b;
}
// 在Galois Field中的乘法
unsigned char multiply(unsigned char a, unsigned char b, bool c)
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
int divide(unsigned char a, unsigned char b, bool c)
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
void Lar(Graphics::TBitmap * M, Graphics::TBitmap * BMP, bool c,unsigned char count)
{
	for (int k = 0; k < BMP->Height; k++) {
		for (int z = 0; z < BMP->Width; z++) {
			int rgb[3] = {0};

			for (int ch = 0; ch < 3; ch++) {
				unsigned char sum = 0;
				for (int i = 0; i < count; i++) {
					unsigned char xi = mark[i]+1;
					unsigned char yi = datap[z][k][xi-1][ch];  // channel value at (z,k) for share xi

					int li = 1;
					for (int j = 0; j < count; j++) {
						if (j == i) continue;
						unsigned char xj = mark[j]+1;
						unsigned char denom = subtract(xj, xi);
						unsigned char inv = divide(xj, denom, c);  // xj / (xj - xi)
						li = multiply(li, inv, c);       // li *= xj / (xj - xi)
					}

					sum = add(sum, multiply(yi, li, c));
				}
				rgb[ch] = sum;
			}

			M->Canvas->Pixels[z][k] = (TColor)RGB(rgb[0], rgb[1], rgb[2]);
		}
	}
}

unsigned char** Matrix(unsigned char n, unsigned char m) {
	unsigned char** A = new unsigned char*[n];
	if (!A)
		return NULL;
	for (int i = 0; i < n; ++i) {
		A[i] = new unsigned char[m];
		if (!A[i])
			return NULL;
	}
	return A;
}
void DeleteMatrix(unsigned char** A, unsigned char n, unsigned char m) {
	for (int i = 0; i < n; ++i)
		delete[] A[i];
	delete[] A;
	A = NULL;
}

int Det(unsigned char** A, unsigned char n,bool c) {
	if (n == 1)
		return A[0][0];
	if (n == 2)
		return subtract(multiply(A[0][0],A[1][1],c),multiply(A[0][1],A[1][0],c));

	int det = 0;
	unsigned char** M = Matrix(n - 1, n - 1);

	// for all elements in the 0th row
	for (int a = 0; a < n; ++a) {
        // create minor matrix
		for (int i = 1, minor_i = 0; i < n; ++i, ++minor_i) {
            for (int j = 0, minor_j = 0; j < n; ++j) {
				if (j == a)
                    continue;
				M[minor_i][minor_j++] = A[i][j];
			}
		}
		det = add(det,multiply(A[0][a],Det(M, n - 1,c),c));
	}
    DeleteMatrix(M, n - 1, n - 1);
	return det;
}
void Deter(Graphics::TBitmap * M,Graphics::TBitmap * BMP,bool c,unsigned char count)
{
	unsigned char rgb[3]={0};
	unsigned char** A = Matrix(count, count);
	for(int i=0;i<count;i++)
	{
		for(int j=0;j<count;j++)
		{
			if(j==0)
				A[i][j]=1;
			else
				A[i][j]=multiply(A[i][j-1],mark[i]+1,c);
		}
	}
	unsigned char AA=Det(A, count,c);
	unsigned char** B = Matrix(count, count);
	for (int i = 0; i < count; i++)
		for (int j = 0; j < count; j++)
			B[i][j] = A[i][j];
	for (int k=0 ; k<BMP->Height ; k++)
	{
		for (int z=0 ; z<BMP->Width ; z++)
		{
			for(int j=0;j<3;j++)
			{
				for(int i=0;i<count;i++)
				{
					B[i][0]=datap[z][k][mark[i]][j];
				}
				unsigned char BB=Det(B, count,c);
				rgb[j]=divide(BB,AA,c);
			}
			M -> Canvas -> Pixels[z][k] = (TColor)RGB(rgb[0], rgb[1], rgb[2]);
		}
	}
	DeleteMatrix(A, count, count);
	DeleteMatrix(B, count, count);
}

void GE(unsigned char** A, unsigned char n, unsigned char m, bool c) {
	int ge_level = (n < m) ? n : m;

	for (int p = 0; p < ge_level; ++p) {
		// 找 pivot（在 GF(c) 中不為 0）
		unsigned char pivot_row = p;
		while (pivot_row < n && A[pivot_row][p] == 0)
			++pivot_row;

		if (pivot_row == n)
			continue; // 此列皆為 0，無法進一步消去

		// 將 pivot 行換到目前這一行
		if (pivot_row != p) {
			unsigned char* temp = A[p];
			A[p] = A[pivot_row];
			A[pivot_row] = temp;
		}

		int pivot = A[p][p];
		if (pivot == 0)
			continue; // 無法作為消元基準

		// 對 pivot 下方的所有列進行消去
		for (int i = p + 1; i < n; ++i) {
			unsigned char factor = A[i][p];
			if (factor == 0) continue;

			// 在 GF 中的比例係數 = factor / pivot
			int scale = divide(factor, pivot, c);

			for (int j = p; j < m; ++j) {
				unsigned char scaled = multiply(A[p][j], scale, c);
				A[i][j] = subtract(A[i][j], scaled);
			}
		}
	}
}
void Gauss(Graphics::TBitmap * M,Graphics::TBitmap * BMP,bool c,unsigned char count)
{
	unsigned char rgb[3]={0};
	unsigned char** A = Matrix(count, count+1);
	for (int k=0 ; k<BMP->Height ; k++)
	{
		for (int z=0 ; z<BMP->Width ; z++)
		{
			for(int u=0;u<3;u++)
			{
				for(int i=0;i<count;i++)
				{
					for(int j=count;j>=0;j--)
					{
                        A[i][j]=0;
						if(j==count)
							A[i][j]=datap[z][k][mark[i]][u];
						else if(j==count-1)
							A[i][j]=1;
						else
							A[i][j]=multiply(A[i][j+1],mark[i]+1,c);
					}
				}
				GE(A, count, count+1,c);
				/*int* x = new int[count];
				for (int i = count - 1; i >= 0; i--) {
					int sum = A[i][count];
					for (int j = i + 1; j < count; j++)
						sum = subtract(sum, multiply(A[i][j], x[j], c));
					if (A[i][i] == 0)
						x[i] = 0; // 或者標記錯誤
					else
						x[i] = divide(sum, A[i][i], c);
				}
				rgb[u] = x[count-1];
				delete[] x;

				if (A[count-1][count-1] == 0)
					rgb[u] = 0;  // or fallback
				else
					rgb[u] = divide(A[count-1][count], A[count-1][count-1], c);*/
				rgb[u]=divide(A[count-1][count],A[count-1][count-1],c);
			}
			M -> Canvas -> Pixels[z][k] = (TColor)RGB(rgb[0], rgb[1], rgb[2]);
		}
	}
	DeleteMatrix(A, count, count+1);
}
//---------------------------------------------------------------------------
Graphics::TBitmap * BMP, * Y, * P, * N, * M; // define global var.
String fname;
void __fastcall TForm1::Button1Click(TObject *Sender)
{
	PageControl1->ActivePage = TabSheet1;
	unsigned char secret=StrToInt(Edit1->Text);
	unsigned char n=StrToInt(Edit2->Text);
	unsigned char k=StrToInt(Edit3->Text);
	if(n<k||n>255)
	{
		ShowMessage("n應大於等於k且小於256");
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
			//Memo1 -> Lines -> Add(IntToStr(i)+","+IntToStr(p[i]));
			i++;
		}
	}
	for(unsigned char i=1;i<=n;i++)
	{
		unsigned char m;
		for(unsigned char j=0;j<k;j++)
		{
			if(j==0)
			{
				data[i]=secret;
			}
			else
			{
				m=1;
				for(unsigned char l=0;l<j;l++)
				{
					m=multiply(m, i, c);
				}
				data[i]=add(data[i],multiply(m, p[j-1], c));
			}
		}
		Memo1 -> Lines -> Add("("+IntToStr(i)+" , "+IntToStr(data[i])+")");
	}
	ListBox1->Clear();
	for (unsigned char i=1; i<=n; i++)
		ListBox1->Items->Add("Share "+IntToStr(i)+" : ("+IntToStr(i)+", "+IntToStr(data[i])+")");

}
//---------------------------------------------------------------------------
void __fastcall TForm1::Button2Click(TObject *Sender)
{
	unsigned char secret=StrToInt(Edit1->Text);
	unsigned char n=StrToInt(Edit2->Text);
	unsigned char k=StrToInt(Edit3->Text);
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
	unsigned char s=0;
	for (int i=0; i<ListBox1->Count; i++)
	{
		if(ListBox1->Selected[i])
		{
			unsigned char temp=data[i+1];
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
	if(s!= secret)
		Memo1 -> Lines -> Add("解密失敗");
	else
        Memo1 -> Lines -> Add("解密成功");
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
    c=1;
	if (datap != nullptr)
	{
		releaseDatap(datap, Y->Height, Y->Width, n1); // 釋放之前分配的空間
		//ShowMessage("釋放");
	}
	if(BMP == nullptr)
	{
        ShowMessage("請先load圖片");
		return;
    }
	Y -> Width = BMP -> Width;
	Y -> Height = BMP -> Height;
	unsigned char n=StrToInt(Edit5->Text);
	unsigned char k=StrToInt(Edit6->Text);
	n1=n;
	allocateDatap(datap, Y->Height, Y->Width, n, 3); // 分配所需的空間
	for (int i=0 ; i<BMP->Height ; i++)
	{
		for (int j=0 ; j<BMP->Width ; j++)
		{
			unsigned char a[3];
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
					unsigned char m;
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

void __fastcall TForm1::Button5Click(TObject *Sender)
{
	c=1;// 控制是2的幾次方,0是2^3,1是2^8
	bool correct=1;
	unsigned char k=StrToInt(Edit6->Text);
    M = new Graphics::TBitmap();
	M -> Width = BMP -> Width;
	M -> Height = BMP -> Height;
	clock_t t_begin,t_end;
	unsigned char count=0;
	for (int i=0; i<ListBox2->Count; i++)
	{
		if(ListBox2->Selected[i])
		{
			mark[count++]=i;
		}
	}
	if(RadioButton3->Checked)
	{
		for (int i=0 ; i<BMP->Height ; i++)
		{
			for (int j=0 ; j<BMP->Width ; j++)
			{
				M -> Canvas -> Pixels[j][i] = (TColor)RGB(0, 0, 0);
			}
		}
		t_begin=clock();
		Lar(M,BMP,c,count);
		t_end=clock();
		Memo1->Lines->Add("拉格朗日CPU time (sec.) = "+FloatToStr((float)(t_end-t_begin)/CLOCKS_PER_SEC));
        for (int i=0 ; i<BMP->Height ; i++)
		{
			for (int j=0 ; j<BMP->Width ; j++)
			{
				if(M -> Canvas -> Pixels[j][i]!=BMP -> Canvas -> Pixels[j][i])
					correct=0;
			}
		}
		if(correct)
			Memo1->Lines->Add("解密成功");
		else
			Memo1->Lines->Add("解密失敗");
	}
	else if(RadioButton4->Checked)
	{
		for (int i=0 ; i<BMP->Height ; i++)
		{
			for (int j=0 ; j<BMP->Width ; j++)
			{
				M -> Canvas -> Pixels[j][i] = (TColor)RGB(0, 0, 0);
			}
		}
		t_begin=clock();
		Deter(M,BMP,c,count);
        t_end=clock();
		Memo1->Lines->Add("行列式CPU time (sec.) = "+FloatToStr((float)(t_end-t_begin)/CLOCKS_PER_SEC));
		for (int i=0 ; i<BMP->Height ; i++)
		{
			for (int j=0 ; j<BMP->Width ; j++)
			{
				if(M -> Canvas -> Pixels[j][i]!=BMP -> Canvas -> Pixels[j][i])
					correct=0;
			}
		}
		if(correct)
			Memo1->Lines->Add("解密成功");
		else
			Memo1->Lines->Add("解密失敗");
	}
	else if(RadioButton5->Checked)
	{
		for (int i=0 ; i<BMP->Height ; i++)
		{
			for (int j=0 ; j<BMP->Width ; j++)
			{
				M -> Canvas -> Pixels[j][i] = (TColor)RGB(0, 0, 0);
			}
		}
		t_begin=clock();
		Gauss(M,BMP,c,count);
        t_end=clock();
		Memo1->Lines->Add("高斯CPU time (sec.) = "+FloatToStr((float)(t_end-t_begin)/CLOCKS_PER_SEC));
        for (int i=0 ; i<BMP->Height ; i++)
		{
			for (int j=0 ; j<BMP->Width ; j++)
			{
				if(M -> Canvas -> Pixels[j][i]!=BMP -> Canvas -> Pixels[j][i])
					correct=0;
			}
		}
		if(correct)
			Memo1->Lines->Add("解密成功");
		else
			Memo1->Lines->Add("解密失敗");
	}
	Image3 -> Picture -> Assign(M);
	PageControl1->ActivePage = TabSheet6;
}
//---------------------------------------------------------------------------
