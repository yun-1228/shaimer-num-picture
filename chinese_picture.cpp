//---------------------------------------------------------------------------

#include <vcl.h>
#include <iostream>
#include <vector>
#include <cstdlib>
#include <time.h>
#include <ctime>
#include <cmath>
#include <algorithm>  // for std::shuffle
#include <random>
#include <chrono>
#pragma hdrstop

#include "Unit1.h"
#define SIZE 100
Graphics::TBitmap * * Z;
int indexZ = 0;
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TForm1 *Form1;
Graphics::TBitmap * BMP,* Y,* P,* M; // define global var.
String fname;
unsigned int ****datap ;
unsigned char n1=0,d,n,k;
unsigned char allPrimes[54] = {2, 3, 5, 7, 11, 13, 17, 19, 23, 29,
						31, 37, 41, 43, 47, 53, 59, 61, 67, 71,
						73, 79, 83, 89, 97, 101, 103, 107, 109, 113,
						127, 131, 137, 139, 149, 151, 157, 163, 167, 173,
						179, 181, 191, 193, 197, 199, 211, 223, 227, 229,
						233, 239, 241, 251};
unsigned int alltPrimes[10] = {257, 263, 269, 271, 277, 281, 283, 293, 307, 311};
unsigned long long alphaval=1,betaval=1;
unsigned char mark[256]={0},pmark[256]={0};
unsigned long long table[256]={0};
unsigned int primenumbers[100];
//---------------------------------------------------------------------------
__fastcall TForm1::TForm1(TComponent* Owner)
	: TForm(Owner)
{
    PageControl1->ActivePage = TabSheet1;
	PageControl2->ActivePage = TabSheet3;
    Z = new Graphics::TBitmap * [SIZE];
	for (int i = 0 ; i < SIZE ; i++)
	{
		Z[i] = new Graphics::TBitmap();
		Z[i] -> PixelFormat = pf24bit;
	}
	Memo1->Clear();

}

std::vector<unsigned char> GetBitmapBytes(Graphics::TBitmap* bmp) {
    bmp->PixelFormat = pf24bit;
    int width = bmp->Width;
    int height = bmp->Height;

    std::vector<unsigned char> data;

    for (int y = 0; y < height; y++) {
        unsigned char* row = (unsigned char*)bmp->ScanLine[y];
        for (int x = 0; x < width * 3; x++) {
            data.push_back(row[x]);
        }
    }
    return data;
}

// 把 byte 陣列轉成 bit 陣列
std::vector<bool> BytesToBits(const std::vector<unsigned char>& bytes) {
    std::vector<bool> bits;
    for (unsigned char byte : bytes) {
        for (int i = 7; i >= 0; i--) {
            bits.push_back((byte >> i) & 1);
        }
    }
    return bits;
}

// 每次取 d 個 bit，最後不足補 0
std::vector<unsigned int> ExtractBitChunks_Padded(const std::vector<bool>& bits, int d) {
    std::vector<unsigned int> result;

	// 計算補幾個 0 到前面
	int remainder = bits.size() % d;
	int padding = (remainder == 0) ? 0 : (d - remainder);

	// 先補 padding 個 0 到前面
	std::vector<bool> paddedBits(padding, 0);
	paddedBits.insert(paddedBits.end(), bits.begin(), bits.end());

	// 每 d 個 bit 擷取一次
	for (size_t i = 0; i < paddedBits.size(); i += d) {
		unsigned int val = 0;
		for (int j = 0; j < d; j++) {
			val = (val << 1) | paddedBits[i + j];
		}
		result.push_back(val);
	}

	return result;
}
void allocateDatap(unsigned int ****&datap, int width, int height, int shares, int channels)
{
	datap = new unsigned int***[height];
	for (int i = 0; i < height; i++)
	{
		datap[i] = new unsigned int**[width];
		for (int j = 0; j < width; j++)
		{
			datap[i][j] = new unsigned int*[shares];
			for (int k = 0; k < shares; k++)
			{
				datap[i][j][k] = new unsigned int[channels];
            }
        }
    }
}
void releaseDatap(unsigned int ****&datap, int width, int height, int shares)
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
void CRT_P(Graphics::TBitmap * M, Graphics::TBitmap * BMP,unsigned char count)
{
	unsigned int Q=1;
	for (int i=0 ; i<count ; i++)
	{
		Q=Q*primenumbers[mark[i]];
	}
	for (int i=0 ; i<count ; i++)
	{
		unsigned int mi=Q/primenumbers[mark[i]];
		for (int j=1;j<primenumbers[mark[i]];j++)
		{
			if((mi%primenumbers[mark[i]])*j==1)
			{
				table[i]=mi*j;
				break;
			}
		}
	}
	for (int i=0 ; i<BMP->Height ; i++)
	{
		for (int j=0 ; j<BMP->Width ; j++)
		{
			unsigned int rgb[3];
			for(int h=0;h<3;h++)
			{
				unsigned int sum=0;
				for(int z=0;z<count;z++)
				{
					sum=sum+datap[j][i][mark[z]][h]*table[z];
				}
				rgb[h] = sum%Q;
			}
			M->Canvas->Pixels[j][i] = (TColor)RGB(rgb[0], rgb[1], rgb[2]);
		}
	}
}
unsigned int modinv(unsigned int a, unsigned int m) {
	int m0 = m, t, q;
	int x0 = 0, x1 = 1;

	if (m == 1) return 0;

	while (a > 1) {
		q = a / m;
		t = m;

		m = a % m;
		a = t;
		t = x0;

		x0 = x1 - q * x0;
		x1 = t;
	}

	if (x1 < 0)
		x1 += m0;

	return x1;
}
unsigned long long CRT_PN(unsigned long long Q,unsigned char count)
{
	Form1->Memo1->Lines->Add("Q:"+UIntToStr(Q));
	unsigned long long sum = 0;
	for (int i = 0; i < count; i++)
	{
		unsigned int mi = Q / primenumbers[mark[i]];
		unsigned int inv = modinv(mi, primenumbers[mark[i]]);
		table[i]= (unsigned long long)mi * (unsigned long long)inv;
		sum += (unsigned long long)mi * (unsigned long long)inv * (unsigned long long)pmark[mark[i]];
		//Form1->Memo1->Lines->Add(IntToStr(i)+": mi:"+IntToStr((int)mi)+" ,inv: "+IntToStr((int)inv)+" ,sumi : "+UIntToStr(sum));
	}
	return sum % Q;
}

void Miggnotte(Graphics::TBitmap * M, Graphics::TBitmap * BMP,unsigned char count)
{
	//CRT_P(M,BMP,count);
	unsigned long long Q=1;
	for (int i=0 ; i<count ; i++)
	{
		Q *= (unsigned long long)primenumbers[mark[i]];
	}
	unsigned long long p=CRT_PN(Q,count);
	Form1->Memo1->Lines->Add("p:"+IntToStr((int)p));
	std::mt19937 prand(p);  // 固定亂數種子
	std::uniform_int_distribution<> distXOR(0, 255);  // 區間 [0, 255]
	for (int i=0 ; i<BMP->Height ; i++)
	{
		for (int j=0 ; j<BMP->Width ; j++)
		{
			unsigned char rgb[3];
			unsigned long long temp = distXOR(prand); // 產生一個隨機整數
			for(int h=0;h<3;h++)
			{
				unsigned long long sum=0;
				for(int z=0;z<count;z++)
				{
					sum+=(unsigned long long)datap[j][i][mark[z]][h]*table[z];
				}
                sum=sum%Q;
				rgb[h] = (sum-p)^temp;
			}
			M->Canvas->Pixels[j][i] = (TColor)RGB(rgb[0], rgb[1], rgb[2]);
		}
	}
	/*for (int i=0 ; i<BMP->Height ; i++)
	{
		for (int j=0 ; j<BMP->Width ; j++)
		{
			unsigned char a[3];
			unsigned int temp = distXOR(prand); // 產生一個隨機整數
			a[0]=GetRValue(M -> Canvas -> Pixels[j][i]); //R
			a[1]=GetGValue(M -> Canvas -> Pixels[j][i]); //G
			a[2]=GetBValue(M -> Canvas -> Pixels[j][i]); //B
			for(int h=0;h<3;h++)
			{
				a[h]=(a[h]-p)^temp;
			}
			M->Canvas->Pixels[j][i] = (TColor)RGB(a[0], a[1], a[2]);
		}
	} */
}

void AB(Graphics::TBitmap * M, Graphics::TBitmap * BMP,unsigned char count)
{
	unsigned long long Q=1;
	for (int i=0 ; i<count ; i++)
	{
		Q *= (unsigned long long)primenumbers[mark[i]];
	}
	unsigned long long p=CRT_PN(Q,count);
	Form1->Memo1->Lines->Add("p:"+IntToStr((int)p));
	for (int i=0 ; i<BMP->Height ; i++)
	{
		for (int j=0 ; j<BMP->Width ; j++)
		{
			unsigned char rgb[3];
			for(int h=0;h<3;h++)
			{
				unsigned long long sum=0;
				for(int z=0;z<count;z++)
				{
					sum+=(unsigned long long)datap[j][i][mark[z]][h]*table[z];
				}
				sum=sum%Q;
				rgb[h] = sum%p;
			}
			M->Canvas->Pixels[j][i] = (TColor)RGB(rgb[0], rgb[1], rgb[2]);
		}
	}
}
//---------------------------------------------------------------------------
void __fastcall TForm1::Button1Click(TObject *Sender)
{
	unsigned int secret=StrToInt(Edit1->Text);
	unsigned int n=StrToInt(Edit2->Text);
	unsigned int k=StrToInt(Edit3->Text);

}
//---------------------------------------------------------------------------

void __fastcall TForm1::Button5Click(TObject *Sender)
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
	PageControl2->ActivePage = TabSheet4;
	/*if (!OpenPictureDialog1->Execute()){
		return;
    }

    try {
        Image1->Picture->LoadFromFile(OpenPictureDialog1->FileName);
		bmp = Image1->Picture->Bitmap;
		Memo1->Lines->Add("總共有 " + IntToStr((int)chunks.size()) + " 組 " + IntToStr(d) + "-bit 整數：");
        for (int i = 0; i < std::min(50, (int)chunks.size()); i++) {
            Memo1->Lines->Add("#" + IntToStr(i) + ": " + IntToStr(chunks[i]));
		}
    }
    catch (Exception &e) {
        ShowMessage("發生錯誤：" + e.Message);
	}
	PageControl2->ActivePage = TabSheet4; */
}
//---------------------------------------------------------------------------

void __fastcall TForm1::Button3Click(TObject *Sender)
{
	Y = new Graphics::TBitmap();
	clock_t t_begin,t_end;
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
	d=StrToInt(Edit7->Text);
	n=StrToInt(Edit6->Text);
	k=StrToInt(Edit4->Text);
	n1=n;
	allocateDatap(datap, BMP->Height, BMP->Width, n, 3); // 分配所需的空間
	unsigned long long p;
	bool validRange = false;
    ListBox2->Clear();
	if(RadioButton3->Checked)
	{
		t_begin=clock();
		while(!validRange)
		{
			alphaval=1,betaval=1;
			for (int i = 0; i < 100; i++)
				primenumbers[i] = 0;
			std::vector<int> primeIndices(54);
			for (int i = 0; i < 54; i++) primeIndices[i] = i;
			// 隨機打亂
			unsigned int seed = static_cast<unsigned int>(std::chrono::system_clock::now().time_since_epoch().count());
			std::mt19937 g(seed);
			std::shuffle(primeIndices.begin(), primeIndices.end(), g);
			for(int i=0;i<n;i++)
			{
				primenumbers[i]=allPrimes[primeIndices[i]];
			}
			std::sort(primenumbers, primenumbers + n);
			for(int i=0;i<n;i++)
			{
				if(i<k)
					betaval=betaval*(unsigned long long)primenumbers[i];
				if(i>n-k)
					alphaval=alphaval*(unsigned long long)primenumbers[i];
			}
			unsigned long long lower = alphaval + 1;
			unsigned long long upper = betaval - 257;
			if (lower <= upper) {
				validRange = true;
			}
		}
		std::random_device rd;  // 隨機種子來源（硬體隨機）
		std::mt19937 gen(rd()); // 隨機數產生器（梅森旋轉法）
		std::uniform_int_distribution<unsigned long long> distP(alphaval+1, betaval-257); // 定義在 alpha 到 beta 間的整數分布
		p = distP(gen); // 產生一個隨機整數
		t_end=clock();
		Memo1->Lines->Add("正確p:"+UIntToStr(p)+"正確alphaval:"+UIntToStr(alphaval)+"正確betaval:"+UIntToStr(betaval));
		Memo1->Lines->Add("選取適當質數CPU time (sec.) = "+FloatToStr((float)(t_end-t_begin)/CLOCKS_PER_SEC));
		t_begin=clock();
		std::mt19937 prand(p);  // 固定亂數種子
		std::uniform_int_distribution<> distXOR(0, 255);  // 區間 [0, 255]
		for (int i=0 ; i<BMP->Height ; i++)
		{
			for (int j=0 ; j<BMP->Width ; j++)
			{
				unsigned char a[3];
				unsigned int temp = distXOR(prand); // 產生一個隨機整數
				a[0]=GetRValue(BMP -> Canvas -> Pixels[j][i]); //R
				a[1]=GetGValue(BMP -> Canvas -> Pixels[j][i]); //G
				a[2]=GetBValue(BMP -> Canvas -> Pixels[j][i]); //B
				for(int h=0;h<3;h++)
				{
					for(int z=0;z<n;z++)
					{
						datap[j][i][z][h]=((a[h]^temp)+p)%primenumbers[z];
					}
				}
			}
		}
		t_end=clock();
		Memo1->Lines->Add("Miggnotte加密 CPU time (sec.) = "+FloatToStr((float)(t_end-t_begin)/CLOCKS_PER_SEC));
		for (int i=1; i<=n; i++)
		{
			pmark[i-1]=p%primenumbers[i-1];
			ListBox2->Items->Add("Share "+IntToStr(i)+", q"+IntToStr(i)+" : "+IntToStr((int)primenumbers[i-1])+", b"+IntToStr(i)+" : "+IntToStr((int)(pmark[i-1])));
		}
	}
	else if(RadioButton4->Checked)
	{
		n=4;
		k=3;
		t_begin=clock();
		int loop=0;
		/*while(!validRange && loop<100)
		{
            alphaval=1,betaval=1;
			for (int i = 0; i < 100; i++)
				primenumbers[i] = 0;
			std::vector<int> primeIndices(54);
			for (int i = 0; i < 54; i++) primeIndices[i] = i;
			// 隨機打亂
			unsigned int seed = static_cast<unsigned int>(std::chrono::system_clock::now().time_since_epoch().count());
			std::mt19937 g(seed);
			std::shuffle(primeIndices.begin(), primeIndices.end(), g);
			for(int i=0;i<n;i++)
			{
				primenumbers[i]=allPrimes[primeIndices[i]];
			}
			std::sort(primenumbers, primenumbers + n);
			for(int i=0;i<n;i++)
			{
				if(i<k)
					betaval=betaval*(unsigned long long)primenumbers[i];
				if(i>n-k)
					alphaval=alphaval*(unsigned long long)primenumbers[i];
			}
			// 用 mt19937 產生 t（0~9）
			//std::uniform_int_distribution<unsigned int> distP(0, 9);  // 產生範圍 0~9 的整數
			//unsigned int p = distP(g);
			//primenumbers[n] = alltPrimes[p];
			if (betaval/alphaval>256){
				validRange = true;
			}
            Memo1->Lines->Add("betaval/alphaval:"+UIntToStr(betaval/alphaval)+"正確alphaval:"+UIntToStr(alphaval)+"正確betaval:"+UIntToStr(betaval));
            loop++;
		}*/
		//Memo1->Lines->Add("p: "+UIntToStr(primenumbers[n]));
		primenumbers[0]=202;
		primenumbers[1]=209;
		primenumbers[2]=217;
		primenumbers[3]=255;
		primenumbers[4]=257;
        for(int i=0;i<n;i++)
		{
			if(i<k)
				betaval=betaval*(unsigned long long)primenumbers[i];
			if(i>n-k)
				alphaval=alphaval*(unsigned long long)primenumbers[i];
		}
		std::random_device rd;  // 隨機種子來源（硬體隨機）
		std::mt19937 gen(rd()); // 隨機數產生器（梅森旋轉法）
		//std::uniform_int_distribution<unsigned int> distP(256, betaval/alphaval); // 定義在 alpha 到 beta 間的整數分布
		//primenumbers[n] = distP(gen); // 產生一個隨機整數
		Memo1->Lines->Add("正確p:"+UIntToStr(primenumbers[n])+"正確alphaval:"+UIntToStr(alphaval)+"正確betaval:"+UIntToStr(betaval));
		unsigned long long lower = alphaval/(unsigned long long)primenumbers[n] + 1;
		unsigned long long upper = (betaval-256)/(unsigned long long)primenumbers[n] - 1;
		//Memo1->Lines->Add("正確lower:"+UIntToStr(lower)+"正確upper:"+UIntToStr(upper));
		std::uniform_int_distribution<unsigned long long> distA(lower, upper); // 定義在 alpha 到 beta 間的整數分布
		t_end=clock();
		Memo1->Lines->Add("選取適當質數CPU time (sec.) = "+FloatToStr((float)(t_end-t_begin)/CLOCKS_PER_SEC));
		t_begin=clock();
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
					unsigned long long A = distA(gen); // 產生一個隨機整數
					for(int z=0;z<n;z++)
					{
						datap[j][i][z][h]=(a[h]+(A*primenumbers[n]));
						datap[j][i][z][h]=datap[j][i][z][h]%primenumbers[z];
						if(i==0&&j==0)
						{
							//Memo1->Lines->Add("A:"+UIntToStr(A)+"正確datap[j][i][z][h]:"+IntToStr((int)a[h])+"後datap[j][i][z][h]:"+UIntToStr(datap[j][i][z][h]));
						}
					}
				}
			}
		}
		t_end=clock();
		Memo1->Lines->Add("A-B加密 CPU time (sec.) = "+FloatToStr((float)(t_end-t_begin)/CLOCKS_PER_SEC));
		ListBox2->Clear();
		for (int i=1; i<=n; i++)
		{
			pmark[i-1]=primenumbers[n]%primenumbers[i-1];
			ListBox2->Items->Add("Share "+IntToStr(i)+", q"+IntToStr(i)+" : "+IntToStr((int)primenumbers[i-1])+", a"+IntToStr(i)+" : "+IntToStr((int)(pmark[i-1])));
		}
	}
	for (int i=0 ; i<BMP->Height ; i++)
	{
		for (int j=0 ; j<BMP->Width ; j++)
		{
			Y -> Canvas -> Pixels[j][i] = (TColor)RGB(datap[j][i][0][0], datap[j][i][0][1], datap[j][i][0][2]);
		}
	}
	Image2 -> Picture -> Assign(Y);
	PageControl2->ActivePage = TabSheet5;
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
	PageControl2->ActivePage = TabSheet5;
}
//---------------------------------------------------------------------------

void __fastcall TForm1::Button4Click(TObject *Sender)
{
	bool correct=1;
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
	/*for (int i=0; i<count; i++) {
		Memo1->Lines->Add("解密用質數：" + IntToStr((int)primenumbers[mark[i]]) + ", pmark: " + IntToStr((int)pmark[mark[i]]));
		Memo1->Lines->Add("mark[" + IntToStr(i) + "] = " + IntToStr((int)mark[i]));
    	Memo1->Lines->Add("用來解密的質數: " + IntToStr((int)primenumbers[mark[i]]));
	}*/
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
		Miggnotte(M,BMP,count);
		t_end=clock();
		Memo1->Lines->Add("Miggnotte解密 CPMU time (sec.) = "+FloatToStr((float)(t_end-t_begin)/CLOCKS_PER_SEC));
		for (int i=0 ; i<BMP->Height ; i++)
		{
			for (int j=0 ; j<BMP->Width ; j++)
			{
				if(M -> Canvas -> Pixels[j][i]!=BMP -> Canvas -> Pixels[j][i])
				{
					correct=0;
					break;
				}
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
		AB(M,BMP,count);
		t_end=clock();
		Memo1->Lines->Add("A-B解密 CPMU time (sec.) = "+FloatToStr((float)(t_end-t_begin)/CLOCKS_PER_SEC));
		for (int i=0 ; i<BMP->Height ; i++)
		{
			for (int j=0 ; j<BMP->Width ; j++)
			{
				if(M -> Canvas -> Pixels[j][i]!=BMP -> Canvas -> Pixels[j][i])
				{
					correct=0;
					break;
				}
			}
		}
		if(correct)
			Memo1->Lines->Add("解密成功");
		else
			Memo1->Lines->Add("解密失敗");
	}
	Image3 -> Picture -> Assign(M);
	PageControl2->ActivePage = TabSheet6;
}
//---------------------------------------------------------------------------
