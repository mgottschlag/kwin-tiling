/***************************************************************************
 *   Copyright (C) 2005 by Olivier Goffart   *
 *   ogoffart@kde.org   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.             *
 ***************************************************************************/
#include "voicesignature.h"
#include "sound.h"
#include <kconfig.h>

#include <math.h>
#ifdef PI
#undef PI
#endif
#define PI (2.0 * asin(1.0))


#include <kdebug.h>
#include <qdatetime.h>

#undef Complex

namespace KHotKeys
{


inline static float ABS(float X)
{
	return (X>0) ? X : -X ;
}
inline static int MAX(int X , int Y)
{
	return (X>Y) ? X : Y ;
}
inline static int MIN(int X , int Y)
{
	return (X<Y) ? X : Y ;
}






class Complex
{
	public:
		Complex () {}
		Complex (double re): _re(re), _im(0.0) {}
		Complex (double re, double im): _re(re), _im(im) {}
		double Re () const { return _re; }
		double Im () const { return _im; }
		void operator += (const Complex& c)
		{
			_re += c._re;
			_im += c._im;
		}
		void operator -= (const Complex& c)
		{
			_re -= c._re;
			_im -= c._im;
		}
		void operator *= (const Complex& c)
		{
			double reT = c._re * _re - c._im * _im;
			_im = c._re * _im + c._im * _re;
			_re = reT;
		}
		Complex operator- ()
		{
			return Complex (-_re, -_im);
		}
		Complex operator- (const Complex& c) const
		{
			return Complex (_re - c._re, _im - c._im);
		}
		Complex operator+ (const Complex& c) const
		{
			return Complex (_re + c._re, _im + c._im);
		}
		Complex operator* (const Complex& c) const
		{
			return Complex (_re * c._re - _im * c._im  , _im * c._re + _re * c._im);
		}
		double Mod () const { return sqrt (_re * _re + _im * _im); }

		static Complex fromExp(double mod, double arg) { return Complex(mod*cos(arg) , mod*sin(arg)); }
	private:
		double _re;
		double _im;
};

static inline double hamming(uint n, uint size)
{
	return HAMMING ? 0.54-0.46*cos( 2*PI*n /(size-1)  )    : 1;
}


static QVector<double> fft(const Sound& sound, unsigned int start, unsigned int stop)
{
	if(start>=stop)
		return QVector<double>();
	
	//We need a sample with a size of a power of two
	uint size=stop-start;
	unsigned short log2size=0;
	while( (1<<log2size)  < size )
		log2size++;

	int diff=(1<<log2size) - size;
	if(diff > size/4 ||  1<<log2size > sound.size()  )
	{
		log2size--;
		diff=(1<<log2size) - size;
	}
	size=1<<log2size;
	int start2=start-diff/2;
	int stop2=start2+ size;
	if(start2<0)
	{
		stop2-=start2;
		start2=0;
	}
	if(stop2>sound.size())
	{
		start2-=  stop2 -  sound.size();
		stop2=sound.size();
		if(start2<0)
		{
			stop2-=start2;
			start2=0;
		}
	}

	//Generate an array to work in
	QVector<Complex> samples(size);

	//Fill it with samples in the "reversed carry" order
	int rev_carry = 0;
	for (uint f = 0; f < size - 1; f++)
	{
		samples[f]=sound.at(start2+rev_carry)* hamming(rev_carry, size);
//		KDEBUG(rev_carry);
		int mask = size>>1;  // N / 2
        // add 1 backwards
		while (rev_carry >= mask)
		{
			rev_carry -= mask; // turn off this bit
			mask >>= 1;
		}
		rev_carry += mask;
	}
	samples[size-1]=sound.at(start2+size-1)*hamming(size-1, size);

	//FFT
	for(uint level=0; level < log2size; level++)
	{
		for( int k=0; k< (size>>1) ; k++)
		{
			uint indice1 = (k << (level+1) ) % (size-1); // (k*2*2^l)%(N-1)
			uint indice2 = indice1 + (1<<level);         // (k*2*2^l)%(N-1) + 2^l

			uint coefW =  ( k << (level+1) ) / (size-1);   //  (k*2*2^l) div (N-1)
			double Wexpn=-2 * PI * coefW /  (2 << level);  //  -2 pi n / 2^(l+1)
			Complex W=Complex::fromExp(1, Wexpn) ;


			//OPERATION BUTTERFLY
			Complex a=samples[indice1];
			Complex b=samples[indice2];
			samples[indice1]=a+W*b;
			samples[indice2]=a-W*b;

//			kdDebug() << k_funcinfo << "PAPILLON   s_" << indice1 << " s_" << indice2 <<  "     W_" << (2<<level) << "^" << coefW << endl;
		}
	}

	QVector<double> result(size);
	for(uint f=0;f<size;f++)
	{
		result[f]=samples[f].Mod()  / size;
		
	}
	return result;
}




QVector<double> VoiceSignature::fft(const Sound& sound, unsigned int start, unsigned int stop)
{
	return KHotKeys::fft(sound, start, stop);
	/*QVector<double> result(8000);
	for(int f=0; f<8000;f++)
	{
		Complex c(0);

		for(uint x=start; x<stop; x++)
		{
			Complex s(sound.at(x));
			double angle=-2*PI*f*x/8000;
			s*= Complex( cos(angle) , sin(angle) );
			c+=s;
		}
		result[f]= c.Mod()/(stop-start)   ;
	}
	return result;*/
}

bool VoiceSignature::window(const Sound& sound, unsigned int *_start, unsigned int *_stop)
{
	bool isNoise=false;
	unsigned int length=sound.size();
	uint unit=WINDOW_UNIT;
	if(length < unit ) 
		return false;

	//Fenêtrage
	unsigned int start=0 , stop=0;
	double moy=0;
	for(uint x=0;x<unit;x++)
	{
		moy+=ABS(sound.at(x));
	}
	
	if(moy>WINDOW_MINIMUM*unit)
		isNoise=true;

	for(uint x=unit; x<length; x++)
	{
		if(moy<WINDOW_MINIMUM*unit)
		{
			if(stop==0)
				start=x-unit/2;
		}
		else
			stop=x-unit/2;
		moy+=ABS(sound.at(x));
		moy-=ABS(sound.at(x-unit));
		
	}
	
	if(moy>WINDOW_MINIMUM*unit && isNoise)
		return false;
	
	stop=MIN(length,stop+WINDOW_MINIMUM_ECART);
	start=MAX(0    ,start-WINDOW_MINIMUM_ECART);	
	
	if(_start)
		*_start=start;
	if(_stop)
		*_stop=stop;
	return start<stop;
}

//finally doesn't give better results
/*#define HZ_TO_MEL(F)  (1127*log(1+(F)/700.0))
#define MEL_TO_HZ(M)  ( ( exp((M)/1127.0)  -1) *700 )*/
#define HZ_TO_MEL(F) (F)
#define MEL_TO_HZ(F) (F)


VoiceSignature::VoiceSignature(const Sound& sound)
{
 	static uint temp_wind=0, temp_fft=0, temp_moy=0;
 	QTime t;
 	t.start();
	
	unsigned int start , stop;
	if(!window(sound,&start,&stop))
	{
		kWarning( 1217 ) << k_funcinfo << "No voice found in the sound" << endl	;
		return;
	}
	
	temp_wind+=t.restart();

	uint length=stop-start;

	for(int wind=0; wind<WINDOW_NUMBER; wind++)
	{
		unsigned int w_start=MAX(start, start+ (int)((wind - WINDOW_SUPER)*length/WINDOW_NUMBER));
		unsigned int w_stop =MIN(stop , start+ (int)((wind+1.0+WINDOW_SUPER)*length/WINDOW_NUMBER));
		

		QVector<double> fourrier=fft(sound, w_start,w_stop);
		
		temp_fft+=t.restart();

		//MEL conversion
		double mel_start=HZ_TO_MEL(FFT_RANGE_INF);
		uint mel_stop=HZ_TO_MEL(FFT_RANGE_SUP);
		
		for(int four=0; four<FOUR_NUMBER; four++)
		{
			unsigned int wf_start=mel_start + four*(mel_stop-mel_start)/FOUR_NUMBER;
			unsigned int wf_stop=mel_start + (four+1)*(mel_stop-mel_start)/FOUR_NUMBER;

			unsigned int f_start=MEL_TO_HZ( wf_start )*fourrier.size()/sound.fs();
			unsigned int f_stop=MEL_TO_HZ( wf_stop )*fourrier.size()/sound.fs();
			unsigned int f_size=f_stop-f_start;
			
			double nb=0;
			for(uint f=f_start; f<f_stop; f++)
			{
				int freq=f*fourrier.size()/sound.fs();
				nb+=fourrier[f]*FFT_PONDERATION(freq);
			}
			nb/=(f_size);
			data[wind][four]=nb;
		}
		
		temp_moy+=t.restart();

	}
	
//	kdDebug( 1217 ) << k_funcinfo << "wind: "<< temp_wind << "  - fft: " << temp_fft << "  - moy: " << temp_moy << endl;
}



VoiceSignature::~VoiceSignature()
{
}



float VoiceSignature::diff(const VoiceSignature &s1, const VoiceSignature &s2)
{
	if(s1.isNull() || s2.isNull())
		return 1000000;
#if 0
	double result=0;
	for(int x=0;x<WINDOW_NUMBER;x++)
		for(int y=0;y<FOUR_NUMBER;y++)
	{
		double d1=s1.data[x][y]-s2.data[x][y];
		result+= d1*d1;//*pond[x][y];
	}
	return result;
#endif
	
	//DTW
	//  http://tcts.fpms.ac.be/cours/1005-08/speech/projects/2001/delfabro_henry_poitoux/
	
	const int I=WINDOW_NUMBER;
	const int J=WINDOW_NUMBER;
	double g[I+1][J+1];
	for(int f=1;f<=J;f++)
		g[0][f]=10000000;
	for(int f=1;f<=I;f++)
		g[f][0]=10000000;
	g[0][0]=0;
	for(int i=1;i<=I;i++)
		for(int j=1;j<=J;j++)
	{
		double d=0;
		for(int f=0;f<FOUR_NUMBER;f++)
		{
			double d1=s1.data[i-1][f]-s2.data[j-1][f];
			d+= d1*d1;//*pond[x][y];
		}
		d=sqrt(d);
		g[i][j]=QMIN(QMIN( g[i-1][j]+d, g[i][j-1]+d )  ,  g[i-1][j-1]+d+d  );
	}

	return g[I][J]/(I+J);	
}





int VoiceSignature::size1()
{
	return WINDOW_NUMBER;
}
		
int VoiceSignature::size2()
{
	return FOUR_NUMBER;
}

QMap<int, QMap<int, double> > VoiceSignature::pond;



void VoiceSignature::write(KConfigBase *cfg, const QString &key) const
{
	QStringList sl;
	for(int x=0;x<WINDOW_NUMBER;x++)
		for(int y=0;y<FOUR_NUMBER;y++)
	{
		sl.append( QString::number(data[x][y]) );
	}
	cfg->writeEntry(key,sl);
}

void VoiceSignature::read(KConfigBase *cfg, const QString &key)
{
	QStringList sl=cfg->readListEntry(key);
	for(int x=0;x<WINDOW_NUMBER;x++)
		for(int y=0;y<FOUR_NUMBER;y++)
	{
		data[x][y]= sl[x*FOUR_NUMBER+y].toDouble();
	}
}

}
