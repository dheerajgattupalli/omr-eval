#include <cstdio>
#include <cstring>
#include <stdio.h>
#include <algorithm>
#include <cstdlib>
#include <fstream>
#include <dirent.h>
#include <cmath>
#include <iostream>
#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
using namespace std;
using namespace cv;
Mat omr;
string input_path;
int omr_x, omr_y ;
int no_of_choices, no_of_columns, number_of_questions;
int column_startx, column_starty, cell_diff_x, cell_diff_y, block_diff_x, column_diff_y, column_diff_x; // X denotes rows, Y denotes Columns 
int id_startx, id_starty, id_diff_x, id_diff_y;
int displacement_x, displacement_y, dx, dy ;
int input_answers[50],correct_answers[50];
int get_pixel_color_value( int x, int y ) {
	x += dx ;
	y += dy ;
	Scalar Location;
	Location = omr.at<uchar>( Point( y, x ) ) ;
	return Location[0] ;
}

void rotate( Mat &src, double angle, Mat &dst ) {
	int len  = max( src.cols, src.rows ) ;
	Point2f pt( len / 2.0 , len / 2.0 ) ;
	Mat r = getRotationMatrix2D( pt, angle, 1.0 ) ;
	warpAffine( src, dst, r, Size( len, len ) ) ;
}
void angle_diff() {
	Mat src = omr ;
	Size size = src.size() ; 
	bitwise_not( src, src ) ;
	vector<Vec4i> lines ;
	HoughLinesP( src, lines, 1, CV_PI / 180, 100, 0.75 * size.width , 20 ) ;
	double angle = 0.0 ;
	int nb_lines = lines.size() ;
	for ( int i = 0 ; i < nb_lines ; ++i ) {
		angle += atan2( ( double ) lines[i][3] - lines[i][1],
				( double ) lines[i][2] - lines[i][0]) ;
	}
	angle /= nb_lines ;
	Mat dst;
	angle = ( angle * 180 ) / CV_PI  ;
	if( angle >= 0 ) 
		angle = -90 + angle ;
	else
		angle = 90 + angle ;
	rotate(src, angle , dst) ;
	bitwise_not( dst, omr ) ;
	omr_x = omr.rows ;
	omr_y = omr.cols ;
	dx = dy = 0 ;
}
void displacement() {
	angle_diff() ;
	int i, j, displacement_y = 0 ;
	int x, y, color ;
	x = omr_x / 4 ;
	j = 1 ; 
	while( j <= 3 ) {
		for( i = 10; ; ++i ) {
			color = get_pixel_color_value(x, i) ;
			if( color <= 100 ) 
				break ;
			if( i > 500 ) 
				break ;
		}
		displacement_y += i ;
		++j ;
		x += ( omr_x / 4 ) ;
	}
	displacement_y /= 3 ;
	y = displacement_y ;
	displacement_y -= 225 ;
	y = 371 + displacement_y ;
	for( i = 0; ; ++i ) {
		color = get_pixel_color_value( i, y ) ;
		if( color <= 200 ) 
			break ;
		if( i > 500 ) 
			break ;
	}
	displacement_x = i ;
	displacement_x -= 130 ;
	dx = displacement_x ;
	dy = displacement_y ;
}
void image_filter() {
	int i, j, color ;
	for( i = 0; i < omr_x ; ++i ) {
		for( j = 0; j < omr_y; ++j ) {
			color = get_pixel_color_value( i, j ) ;
			if( color >= 225 ) {
				omr.at<uchar>( Point( j, i ) ) = 255 ;
			}
		}
	}
}
void find_id() {
	int startx, starty, sx, sy, i, pi, pj ;
	startx = id_startx ;
	starty = id_starty ;
	for( i = 1; i <= 16; ++i ) {   
		sx = startx ;
		int count=0;
		sy = starty + ( i - 1 ) * ( id_diff_y + 5 ) ;
		for( pi = 1 ; pi <= id_diff_y ; ++pi ){
			for( pj = 1 ; pj <= id_diff_x ; ++pj ){ 
				if(get_pixel_color_value(sx+pj,sy+pi)<100)count++;
			}
		}
		if(count>1000)printf("1");
		else printf("0");
	}
	printf("\n");
}
void omr_evaluation( int startx, int starty ) {
	int i, j, k, pi, pj ;
	int sx, sy, f = 1, s = 1 ;  
	for( i = 1 ; i <= number_of_questions ; ++i ) { 
		sx = startx + (i - 1) * block_diff_x ;
		for( j = 1 ; j <= no_of_choices ; ++j ) {  
			int count=0;
			sy = starty + (j - 1) * ( cell_diff_y + 4 ) ;
			for( pi = 1 ; pi <= cell_diff_y ; ++pi ) { 
				for( pj = 1 ; pj <= cell_diff_x ; ++pj ) {  
					if(get_pixel_color_value(sx+pj,sy+pi)<200)
						count++;
				}
			}
			if(count>900)input_answers[i-1]=j;
		}
		if( f == ( 6 * s ) ) { 
			startx = startx - 8 ;
			s = s + 1 ;
		}
		++f ;
	}
}
void omr_read() {
	DIR *directory ;
	struct dirent *entry ;
	input_path="./input/";
	if( directory = opendir("./input") ) {
		while( entry = readdir( directory ) ) {
			if( strcmp( entry -> d_name , "Input" ) == 0 )
				continue ;
			if( strcmp( entry -> d_name, "." ) == 0 )
				continue ;
			if( strcmp( entry -> d_name, ".." ) == 0 )
				continue ;
			string name = entry -> d_name ;
			omr = imread( input_path + name , 0 ); 
			omr_x = omr.rows ;
			omr_y = omr.cols ;
			dx = dy = 0 ;
			image_filter() ;
			displacement() ;
			find_id() ;
			int startx , starty, i ;
			startx = column_startx ;
			starty = column_starty ;
			for( i = 1; i <= no_of_columns ; ++i ) { 
				omr_evaluation( startx, starty ) ;
				startx += column_diff_x ; 
				starty += column_diff_y ;
			}
			int correct=0;
			for(i=0;i<50;i++){
				printf("%d ",input_answers[i]);
				if(correct_answers[i]==input_answers[i])correct++;
			}
			printf("\nscore-->%d\n",correct);
		}
	} else {
		printf( "The given directory is not present!!\n" ) ;
	}
}
void input_read() {
	FILE *pFile;
	pFile = fopen("input.txt", "r");
	if (pFile == NULL)
		printf ("Error opening the file\n\n'");
	char foo[100] ;
	fscanf (pFile, "%d %d %s", &id_startx,&id_starty,foo);
	fscanf( pFile, "%d %s", &id_diff_x, foo ) ;
	fscanf( pFile, "%d %s", &id_diff_y, foo ) ;
	fscanf( pFile, "%d %d %s", &column_startx, &column_starty, foo ) ;
	fscanf( pFile, "%d %s", &cell_diff_x, foo ) ;
	fscanf( pFile, "%d %s", &cell_diff_y, foo ) ;
	fscanf( pFile, "%d %s", &block_diff_x, foo) ;
	fscanf( pFile, "%d %s", &no_of_choices, foo ) ;
	fscanf( pFile, "%d %s", &number_of_questions, foo ) ;
	fscanf( pFile, "%d %s", &no_of_columns, foo ) ;
	fscanf( pFile, "%d %s", &column_diff_x, foo ) ;
	fscanf( pFile, "%d %s", &column_diff_y, foo ) ;
	fclose(pFile);
}
int main() {
	int kk;
	for(kk=0;kk<50;kk++)
		correct_answers[kk]=rand()%5+1;
	try{
		input_read() ;
		omr_read() ;
	}catch(...) {
		printf( "There is error in the Input file\n" ) ;
	}
	int i;
	return 0;
}
