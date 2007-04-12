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

#include "sound.h"
#include <QtCore/QFile>
#include <QtCore/QDataStream>
#include <kdebug.h>




Sound::Sound()
{
}


Sound::~Sound()
{
}


#define READ_FROM_STREAM(FORMAT,NAME)  FORMAT NAME; stream >> NAME;
#define MAGIC(CH) { \
   stream >> magic;  \
   if( magic != ( (CH)[0] | (CH)[1]<<8 | (CH)[2]<< 16 | (CH)[3] << 24 ) ) \
   {  \
      kWarning() << k_funcinfo << "bad format " << magic << " != " << CH "\n";\
      return;\
   } }   

#define ABS(X)  ( (X>0) ? X : -X )

void Sound::load(const QString& filename)
{
	kDebug() << k_funcinfo << filename << endl;
	data=QVector<Q_INT32>();
	QFile file(filename);
	if(!file.open(IO_ReadOnly))
	{
		kWarning() << k_funcinfo <<"unable to open file" << endl;
		return;
	}
	QDataStream stream(&file);
	stream.setByteOrder( QDataStream::LittleEndian );
	Q_INT32 magic;
	
	MAGIC("RIFF");
	READ_FROM_STREAM(quint32,ChunkSize);
	MAGIC("WAVE");
	MAGIC("fmt ");
	READ_FROM_STREAM(quint32,ChunkSize2);
	READ_FROM_STREAM(Q_INT16,AudioFormat);
	READ_FROM_STREAM(Q_UINT16,NumberOfChannels);
	READ_FROM_STREAM(quint32,SampleRate);
	_fs=SampleRate;
	READ_FROM_STREAM(quint32,ByteRate);
	READ_FROM_STREAM(Q_UINT16,BlockAlign);
	READ_FROM_STREAM(Q_UINT16,BitsPerSample);
	MAGIC("data");
	READ_FROM_STREAM(QByteArray,SoundData);
	NumberOfChannels=1; //Wav i play are broken

	file.close();

	uint BytePS=BitsPerSample/8;
	uint NumberOfSamples = (SoundData.size())/(NumberOfChannels*BytePS);
	

	data.resize(NumberOfSamples);

//	kDebug() << k_funcinfo << NumberOfSamples << " samples" << endl;

	max=0;
	for(unsigned long int f=0;f<NumberOfSamples;f++)
	{
		Q_INT32 nb=0;
		for(uint k=0;k<BytePS;k++)
		{
            nb |= (SoundData[(unsigned int)(f*BytePS+k)]&0x000000FF) << (k*8);
		}
		if(nb & (1 << (BytePS*8 -1)) )
			nb = nb-(1<<BytePS*8);
		data[f]=nb;
		if(ABS(nb)>max)
		{
			max=ABS(nb);
		}
	}

/*	static int q=0;
	QString name="test" + QString::number(q++) + ".wav";
	save(name);*/

}

#define SMAGIC(CH) { stream << ( Q_INT32) ( (CH)[0] | (CH)[1]<<8 | (CH)[2]<< 16 | (CH)[3] << 24 ) ; }

void Sound::save(const QString& filename) const
{
	kDebug( 1217 ) << k_funcinfo << filename << " - " << data.size() <<  endl;
	QFile file(filename);
	if(!file.open(IO_WriteOnly))
	{
		kWarning() << k_funcinfo <<"unable to open file" << endl;
		return;
	}
	QDataStream stream(&file);
	stream.setByteOrder( QDataStream::LittleEndian );


	QByteArray SoundData(data.size()*2);
	
	for(unsigned long int f=0;f<data.size();f++)
	{
		Q_UINT16 val= (signed short int) ( (data.at(f) * ((double)(1<<13)/(signed)max)  ) );
        SoundData[ (uint)(2*f) ]=   val & 0x00FF;
        SoundData[(uint)(2*f+1)]=  (val & 0xFF00) >> 8;
		
//		kdDebug( 1217 ) << k_funcinfo << data.at(f) << " / " << max << " = " << val << "  |  " <<   SoundData[ 2*f ] << " "<< SoundData[ 2*f+1 ] <<  endl;
	}

	Q_UINT16 NumberOfChannels=2;
	quint32 SampleRate=_fs;

	SMAGIC("RIFF");
	//READ_FROM_STREAM(quint32,ChunkSize);
	stream <<  (quint32)(36+ SoundData.size());
	SMAGIC("WAVE");
	SMAGIC("fmt ");
	//READ_FROM_STREAM(quint32,ChunkSize2);
	stream <<  (quint32)(16);
	//READ_FROM_STREAM(Q_INT16,AudioFormat);
	stream <<  (Q_INT16)(1);
	//READ_FROM_STREAM(Q_UINT16,NumberOfChannels);
	stream <<  (Q_UINT16)(NumberOfChannels);
	//READ_FROM_STREAM(quint32,SampleRate);
	stream <<  (quint32)(SampleRate);
	//READ_FROM_STREAM(quint32,ByteRate);
	stream <<  (quint32)(NumberOfChannels*SampleRate*16/8);
	//READ_FROM_STREAM(Q_UINT16,BlockAlign);
	stream <<  (Q_UINT16)(16/8 *NumberOfChannels);
	//READ_FROM_STREAM(Q_UINT16,BitsPerSample);
	stream <<  (Q_UINT16)(16);
	SMAGIC("data");
	//READ_FROM_STREAM(QByteArray,SoundData);
	stream <<  SoundData;

	file.close();
	
}




#if 0
void Sound::load(const QString& filename)
{
	cout << "saout \n";
	data=QMemArray<long unsigned int>();
	static const int BUFFER_LEN = 4096;

	//code from libtunepimp
	//(wav_trm.cpp)

	FILE          *source;
	unsigned char  buffer[100], *copyBuffer;
	unsigned int   bytes;
	unsigned long  ulRIFF;
	unsigned long  ulLength;
	unsigned long  ulWAVE;
	unsigned long  ulType;
	unsigned long  ulCount;
	unsigned long  ulLimit;
	bool           haveWaveHeader = false;
	unsigned long  waveSize = 0;
	WAVEFORMAT     waveFormat;
	int            toRead;
	mb_int64_t     fileLen = 0;

	source = fopen(filename.ascii(), "rb");
	if (source == NULL)
	{
//		errorString = string("File not found");
//		fclose(source);
		cout << "File not found \n";
		return;
	}

	fseek(source, 0, SEEK_END);
	fileLen = ftell(source);
	fseek(source, 0, SEEK_SET);

	if (fread(buffer, 1, 12, source) != 12)
	{
//		errorString = string("File is too short");
		cout << "File is to short \n";
		fclose(source);
		return ;
	}

	ulRIFF = (unsigned long)(((unsigned long *)buffer)[0]);
	ulLength = (unsigned long)(((unsigned long *)buffer)[1]);
	ulWAVE = (unsigned long)(((unsigned long *)buffer)[2]);

	if(ulRIFF != MAKEFOURCC('R', 'I', 'F', 'F') ||
		  ulWAVE != MAKEFOURCC('W', 'A', 'V', 'E'))
	{
//		errorString = strdup("File is not in WAVE format");
		cout << "File is not WAVE \n";
		fclose(source);
		return ;
	}

    // Run through the bytes looking for the tags
	ulCount = 0;
	ulLimit = ulLength - 4;
	while (ulCount < ulLimit && waveSize == 0)
	{
		if (fread(buffer, 1, 8, source) != 8)
		{
//			errorString = strdup("File is too short");
			cout << "File is to short \n";
			fclose(source);
			return;
		}

		ulType   = (unsigned long)(((unsigned long *)buffer)[0]);
		ulLength = (unsigned long)(((unsigned long *)buffer)[1]);
		switch (ulType)
		{
          // format
			case MAKEFOURCC('f', 'm', 't', ' '):
				if (ulLength < sizeof(WAVEFORMAT))
				{
//					errorString = strdup("File is too short");
					cout << "File is to short \n";
					fclose(source);
					return ;
				}

				if (fread(&waveFormat, 1, ulLength, source) != ulLength)
				{
//					errorString = strdup("File is too short");
					cout << "File is to short \n";
					fclose(source);
					return ;
				}

				if (waveFormat.wFormatTag != WAVE_FORMAT_PCM)
				{
//					errorString = strdup("Unsupported WAV format");
					cout << "Unsupported WAVE \n";
					fclose(source);
					return ;
				}
				haveWaveHeader = true;

				ulCount += ulLength;
				break;

          // data
			case MAKEFOURCC('d', 'a', 't', 'a'):
				waveSize = ulLength;
				break;

			default:
				fseek(source, ulLength, SEEK_CUR);
				break;

		}
	}


	if (!haveWaveHeader)
	{
//		errorString = strdup("Could not find WAV header");
		cout << "Header nbot found \n";
		fclose(source);
		return ;
	}

	fileLen -= (mb_int64_t)ftell(source);
	fileLen /= waveFormat.nChannels;
	fileLen /= (waveFormat.nBlockAlign / waveFormat.nChannels);

	fileLen /= waveFormat.nSamplesPerSec;

	//on ne lit qu'un channel
	//waveSize=fileLen;
	data.resize(waveSize);
	unsigned long pos=0;

	cout << "Weeee "<< waveSize <<"\n";

	copyBuffer = (unsigned char*)malloc(BUFFER_LEN);
	if (copyBuffer == NULL)
	{
//		errorString = strdup("Cannot allocate buffer space.");
		return ;
	}

	for(;;)
	{
		toRead = min(waveSize, (unsigned long)BUFFER_LEN);
		if (toRead <= 0)
			break;

		bytes = fread(copyBuffer, 1, toRead, source);
		if (bytes <= 0)
			break;

		for(uint f=0;f<bytes;f+=4)
		{
			data[pos]=(((unsigned long*)copyBuffer)[f/4]);
			pos++;
		}

		waveSize -= toRead;
	}
	free(copyBuffer);
	fclose(source);

    return ;
}

#endif
