#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MIN(X, Y) (((X) < (Y)) ? (X) : (Y))

unsigned char header[] = { // 4319232 = 1184*912*4
		0x42,0x4D,/*BM*/ 0x00,0xE8,0x41,0x00,/*filesize*/ 0x00,0x00,0x00,0x00,//unused
		0x36,0x00,0x00,0x00,/*dataoffset*/ 0x28,0x00,0x00,0x00,//headersize
		0xa0,0x04,0x00,0x00,/*width*/ 0x90,0x03,0x00,0x00,//height
		0x01,0x00,/*planecount*/ 0x20,0x00,//bitcount
		0x00,0x00,0x00,0x00,/*compr*/ 0x00,0x00,0x00,0x00,//compressed image size
		0xC4,0x0E,0x00,0x00,0xC4,0x0E,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};//trash

struct ImgData { int W, H, Res; };
struct ImgData Img = { .W=1184, .H=912 };
struct ImgData Blk = { .W= 256, .H= 16 };

int tBits[] = { 21644, 23896 };//template section sizes
char * tData;//array for template stage data

const int Bits = 4;

void convert( int mode, char ** data, FILE * fstr ){
	int ofs = 0, len = Blk.W;
//	for( int lane=0; lane<Img.H/Blk.H ;){//upsidedown
	for( int lane=Img.H/Blk.H-1 ; -1<lane ;){//vertical flip
		int count = len*Bits;// printf( "%d:%d:%d ", (ofs+(lane*Blk.H)*Img.W)*Bits, ofs, len );
//		for( int line=0; line<Blk.H; line++ ){//upsidedown
		for( int line=Blk.H-1; -1<line ; line-- ){//vertical flip
			int offset = (ofs+(line+lane*Blk.H)*Img.W)*Bits;
			if( mode ) fread( *data+offset, sizeof(char), count, fstr );// blk to img
			else      fwrite( *data+offset, sizeof(char), count, fstr );// img to blk
		}
		ofs = ofs+len;
		if( Img.W <= ofs ){// printf( "\n" );
			if( len!=Blk.W ) len = Blk.W-len;
			ofs = 0; lane--;// --/++ depends on flip
		}else len = MIN( Img.W-ofs, Blk.W );
}}

int bgtemplate( char ** bgtemp ){
	FILE * bgfile = fopen( "bg12.dat", "rb" );
	if( bgfile==NULL ) printf( "Error: Make sure the executable file is in the same folder as bg12.dat file.\n" );
	else{
		*bgtemp = malloc( tBits[0]+tBits[1]*sizeof(char) );
		fread( *bgtemp, sizeof(char), tBits[0], bgfile );
		fseek( bgfile, Img.Res*Bits, SEEK_CUR );
		fread( *bgtemp+tBits[0], sizeof(char), tBits[1], bgfile );
		fclose( bgfile );
		printf( "Stage file template loaded\n" );
		return 1;
	}
	return 0;
}

int main( int argc, char* argv[] ){
	Img.Res = Img.W*Img.H;
	Blk.Res = Blk.W*Blk.H;
	if( argc<2 ) printf( "Drag and drop every file you want to convert on the executable,\nor pass the filenames as arguments.\n"
		"Image file has to be a 24bit or 32bit BMP, w1184*h912 in size.\n"
		"Converting images to stage files requires bg12.dat in the same directory as the executable.\n" );
	else{
		for( int fc=1; fc<argc; fc++ ){
			FILE * ifile = fopen( argv[fc], "rb" );
			if( ifile==NULL ) printf( "Error: Failed to open %s (check filename or path?)\n", argv[fc] );
			else{
				char * fnme = malloc( strlen( argv[fc] ) + 5 );// ".dat"|".bmp"+'\0'
				strcpy( fnme, argv[fc] );
				char * pxls = malloc( Img.Res*Bits*sizeof(char) );
				unsigned char thead[54];
				fread( thead, sizeof(char), 54, ifile );
				if( strncmp( thead, "BM", 2 ) == 0 ){ //BMP file to stage
					strcat( fnme, ".dat" );
					FILE * ofile = fopen( fnme, "wb" );
					if( ofile==NULL ) printf( "Error: Failed to create output file %s\n", fnme );
					else{
						if( tData || bgtemplate( &tData ) ){// check if template data is already loaded, try to load if not
							unsigned short dBits = thead[28]|(thead[29]<<8);//bmp bitcount
							unsigned int dOffset = thead[10]|(thead[11]<<8)|(thead[12]<<16)|(thead[13]<<24);
							fwrite( tData, sizeof(char), tBits[0], ofile );
							fseek( ifile, dOffset, SEEK_SET );
							fread( pxls, sizeof(char), Img.Res*(dBits/8), ifile );
							switch( dBits ){
								case 24:// add padding to pxls and fallthrough (don't break)
									printf( "Converting %s from 24bit to 32bit\n", argv[fc] );
									for( int i=Img.Res-1; -1<i; i-- ) memmove( pxls+i*4, pxls+i*3, 3 );
								case 32:
									for( int i=3; i<Img.Res*Bits; i+=4 ) memset( pxls+i, 0xFF, 1 );
									convert( 0, &pxls, ofile ); // ^ Some editors save BMPs with 00 on 4th byte, making the stage transparent
									break;
								default: printf( "Error: Unsupported BMP image bit depth, 24 or 32 bit required\n" );
							}
							fwrite( tData+tBits[0], sizeof(char), tBits[1], ofile );
							printf( "%s converted to %s successfully\n", argv[fc], fnme );
						}
						else printf( "Error: Failed to load template file (make sure \"bg12.dat\" is in the executable folder)\n" );
						fclose( ofile );
					}
				}
				else if( strncmp( thead, "bgmake", 6 ) == 0 ){ //Stage file
					strcat( fnme, ".bmp" );
					FILE * ofile = fopen( fnme, "wb" );
					if( ofile==NULL ) printf( "Failed to open output file %s\n", fnme );
					else{
						fwrite( header, sizeof(char), sizeof(header), ofile );
						fseek( ifile, tBits[0], SEEK_SET ); // pixel data offset
						convert( 1, &pxls, ifile );
						fwrite( pxls, sizeof(char), Img.Res*Bits, ofile );
						if( !fclose( ofile ) ) printf( "%s converted to %s successfully\n", argv[fc], fnme );
					}
				}
				else printf( "Error: Unknown file format for %s\n", argv[fc] );
				fclose( ifile );
				free( fnme ); free( pxls );
			}
		}
	free( tData ); //stage background template data
	}
	printf( "Press Enter to close." );getchar();
	return 0;
}