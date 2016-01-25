/*
OpenCV 2.4.7 or else edition is needed
opencv_core247.dll
opencv_highgui247.dll
opencv_imgproc247.dll

haven't finish...
by J.S.Y.Chenkerymiowstone
*/

# include <stdio.h>
# include <math.h>
# include <sys/time.h>
# include "opencv/highgui.h"
# include "opencv/cv.h"

# define r0		20
# define r1		30
# define r2		18
# define winx	480
# define winy	640
# define bound	20
# define wall	130
# define DT		0.03
# define dampw	0.8
# define damph	1.0
# define vmax	1000 

# define cosd(x) cos(3.1415926*(x)/180.0)
# define sind(x) sin(3.1415926*(x)/180.0)
//# define max(x,y)	( (x>y) ? x : y )
//# define max(x,y)	( (x<y) ? x : y )
# define pix2coo(x)	((x)-bound-r0)
# define coo2pix(x)	((x)+bound+r0)
# define Abs(x)		( (x<0) ? (-(x)) : (x) )
# define sqr(x)		((x)*(x))

void easymouse(int event, int x, int y, int flag, void *imgv);
void preprocessing(char *windowname, IplImage **base, IplImage **fr, IplImage **fr0);
void initialize(struct timeval *past);
void easyplot(IplImage *fr, IplImage *fr0);
void easyupdate(struct timeval *past);
void user_update();
void com_update();
void ball_update(float dt);
int check_wall_reflection(float dt, float *w_progress);
int check_handle_collision(float dt, float *progress, CvPoint2D32f uv, CvPoint2D32f cv);
int collide_moment(float rpx, float rpy, float pr, float rvx, float rvy, float dt, float *progress);
CvPoint2D32f collide_velocity_change(float rpx, float rpy, float rvx, float rvy);
void velocity_correction(int w_flag, float w_progress, int h_flag, float h_progress, CvPoint2D32f uv, CvPoint2D32f cv, float *dt);

CvPoint2D32f mp, upc, up0c, cpc, cp0c, gpc, bpc, bp0c, bv;
int click, com_change_goal;
int score[2];

CvSize winsz = cvSize(winx, winy);
CvScalar white	= CV_RGB(255, 255, 255);
CvScalar red 	= CV_RGB(250,  80,  80);
CvScalar yellow	= CV_RGB(250, 230, 120);
CvScalar blue	= CV_RGB(110, 150, 220);
CvScalar green	= CV_RGB(120, 240,  80);

int boundw = winx-2*(bound+r0);
int boundh = winy-2*(bound+r0);
int boundx1 = r1-r0;
int boundx2 = boundw+r0-r1;
int boundy1 = boundh/2+r1;
int boundy2 = boundh+r0-r1;
int wallc = wall-r0;

int main()
{
	char windowname[] = "Table Hockey Little Game!";
	IplImage *base, *fr, *fr0;
	preprocessing(windowname, &base, &fr, &fr0);
	
	struct timeval past;
	initialize(&past);
	do {
		cvCopy(base, fr, NULL);
		//printf("%5d%5d%5d\n", mp.x, mp.y, click);
		//printf("%20.6f\n", dt);
		easyupdate(&past);
		easyplot(fr, fr0);
		cvShowImage(windowname, fr);
	} while( cvWaitKey(3)!=27 );
	
	cvDestroyWindow(windowname);
	cvReleaseImage(&base);
	cvReleaseImage(&fr);
	cvReleaseImage(&fr0);
	return 0;
}

void easymouse(int event, int x, int y, int flag, void *imgv)
{
	mp = cvPoint2D32f(x, y);
	click = (event==CV_EVENT_LBUTTONDOWN);
}


void preprocessing(char *windowname, IplImage **base, IplImage **fr, IplImage **fr0)
{
	srand(time(NULL));
	// point postion
	mp  = cvPoint2D32f(-100.0, -100.0);
	upc = cvPoint2D32f(-100.0, -100.0);
	cpc = cvPoint2D32f(boundw/2, winx/2-bound-wall);
	
	// image
	*base = cvCreateImage(winsz, 8, 3);
	*fr   = cvCreateImage(winsz, 8, 3);
	*fr0  = cvCreateImage(winsz, 8, 3);
	cvNamedWindow(windowname, 1);
	
	// mouse call
	cvSetMouseCallback(windowname, easymouse, (void *)*base);

	// red bound
	int sep = 8, thk1 = 2, thk2 = 7;
	CvScalar bcolor = red;
	CvPoint p1 = cvPoint(bound,bound), p2 = cvPoint(bound,winy/2-sep);
	cvLine(*base, p1, p2, bcolor, thk2, CV_AA, 0);
	cvLine(*base, p1, p2,  white, thk1, CV_AA, 0);
	p2 = cvPoint(bound+wall,bound);
	cvLine(*base, p1, p2, bcolor, thk2, CV_AA, 0);
	cvLine(*base, p1, p2,  white, thk1, CV_AA, 0);
	// yellow bound
	bcolor = yellow;
	p1 = cvPoint(winx-bound,bound), p2 = cvPoint(winx-bound,winy/2-sep);
	cvLine(*base, p1, p2, bcolor, thk2, CV_AA, 0);
	cvLine(*base, p1, p2,  white, thk1, CV_AA, 0);
	p2 = cvPoint(winx-bound-wall,bound);
	cvLine(*base, p1, p2, bcolor, thk2, CV_AA, 0);
	cvLine(*base, p1, p2,  white, thk1, CV_AA, 0);
	// blue bound
	bcolor = blue;
	p1 = cvPoint(bound,winy-bound), p2 = cvPoint(bound,winy/2+sep);
	cvLine(*base, p1, p2, bcolor, thk2, CV_AA, 0);
	cvLine(*base, p1, p2,  white, thk1, CV_AA, 0);
	p2 = cvPoint(bound+wall,winy-bound);
	cvLine(*base, p1, p2, bcolor, thk2, CV_AA, 0);
	cvLine(*base, p1, p2,  white, thk1, CV_AA, 0);
	// green bound
	bcolor = green;
	p1 = cvPoint(winx-bound,winy-bound), p2 = cvPoint(winx-bound,winy/2+sep);
	cvLine(*base, p1, p2, bcolor, thk2, CV_AA, 0);
	cvLine(*base, p1, p2,  white, thk1, CV_AA, 0);
	p2 = cvPoint(winx-bound-wall,winy-bound);
	cvLine(*base, p1, p2, bcolor, thk2, CV_AA, 0);
	cvLine(*base, p1, p2,  white, thk1, CV_AA, 0);

	p1 = cvPoint(winx-bound-2*sep,winy/2), p2 = cvPoint(bound+2*sep,winy/2);
	cvLine(*base, p1, p2, white, 1, CV_AA, 0);
	
	thk1 = 1;
	p1 = cvPoint(winx/2,winy/2);
	cvCircle(*base, p1, winx/2-bound-wall, white, thk1, CV_AA, 0);
	p1 = cvPoint(winx/2,bound);
	cvCircle(*base, p1, winx/2-bound-wall, white, thk1, CV_AA, 0);
	p1 = cvPoint(winx/2,winy-bound);
	cvCircle(*base, p1, winx/2-bound-wall, white, thk1, CV_AA, 0);
	
	thk1 = 5;
	p1 = cvPoint(winx-bound-winx/7,winy/2);
	cvCircle(*base, p1, 33, CV_RGB(0,0,0), -1, CV_AA, 0);
	cvCircle(*base, p1, 33, white, thk1, CV_AA, 0);
	p1 = cvPoint(winx-bound-winx/7-sep,winy/2-2*sep);
	p2 = cvPoint(winx-bound-winx/7-sep,winy/2+2*sep);
	cvLine(*base, p1, p2,  white, thk1, CV_AA, 0);
	p1 = cvPoint(winx-bound-winx/7+sep,winy/2-2*sep);
	p2 = cvPoint(winx-bound-winx/7+sep,winy/2+2*sep);
	cvLine(*base, p1, p2,  white, thk1, CV_AA, 0);
}


void initialize(struct timeval *past)
{
	gettimeofday(past, NULL);
	gpc = cpc;
	com_change_goal = 0;
	bpc = cvPoint2D32f(boundw+r0-winx/7,boundh/2);
	bv.x = 0;// 2004;
	bv.y = 0;//1000;
}


void easyplot(IplImage *fr, IplImage *fr0)
{
	int rmean = 0.5*(r1+r2), rthick = r1-r2;
	CvPoint up, cp, bp;
	
	up.x = coo2pix(upc.x);
	up.y = coo2pix(upc.y);
	cvCircle(fr, up, rmean,   red, rthick+2, CV_AA, 0);
	cvCircle(fr, up, rmean, white, rthick-4, CV_AA, 0);
	
	cp.x = coo2pix(cpc.x);
	cp.y = coo2pix(cpc.y);
	cvCircle(fr, cp, rmean, green, rthick+2, CV_AA, 0);
	cvCircle(fr, cp, rmean, white, rthick-4, CV_AA, 0);
	
	bp.x = coo2pix(bpc.x);
	bp.y = coo2pix(bpc.y);
	cvCircle(fr, bp, r0,  white, -1, CV_AA, 0);
	cvCircle(fr, bp, r0,   blue, 3, CV_AA, 0);
	// blur processing
	cvSmooth(fr, fr, CV_BLUR, 15, 15, 0.0, 0.0);
	cvAddWeighted(fr0, 0.3, fr, 1.4, 0.0, fr);
	
	cvCopy(fr, fr0);
}


float time_interval(struct timeval *past)
{
	struct timeval now;
	float dt;
	gettimeofday(&now, NULL);
	dt = (now.tv_sec-past->tv_sec) + (now.tv_usec-past->tv_usec)/1000000.0;
	*past = now;
	return dt;
}

void easyupdate(struct timeval *past)
{
	float dt = time_interval(past);
	user_update();
	com_update();
	ball_update(dt);
}


void user_update()
{
	int outx1, outx2, outy1, outy2, gap = 20, R = r1+r0;
	float shift, dist;
	up0c = upc;
	upc.x  = pix2coo(mp.x);
	upc.y  = pix2coo(mp.y);
	outx1 = (upc.x<boundx1);
	outx2 = (upc.x>boundx2);
	outy1 = (upc.y<boundy1);
	outy2 = (upc.y>boundy2);
	if(outx1+outx2+outy1+outy2>0) {
		if      (outx1) upc.x = boundx1;
		else if (outx2) upc.x = boundx2;
		if      (outy1) upc.y = boundy1;
		else if (outy2) upc.y = boundy2;
	}
	if(bpc.y>boundh/2+r0){
		if(bpc.y<boundh-gap) {
			if(Abs(upc.y-bpc.y)<gap) {
				if     (       bpc.x<gap && upc.x<       r1+gap) upc.x =        r1+gap;
				else if(boundw-bpc.x<gap && upc.x>boundw-r1-gap) upc.x = boundw-r1-gap;
			}
		}
		else {
			if(bpc.x<wallc) {
				if(bpc.x>gap) {
					if(Abs(upc.x-bpc.x)<gap && upc.y>boundh-r1-gap) upc.y = boundh-r1-gap;
				}
				else {
					dist = sqrt(sqr(upc.x)+sqr(upc.y-boundh));
					if((int)dist<R) {
						upc.x =          R*(upc.x       )/dist;
						upc.y = boundh + R*(upc.y-boundh)/dist;
					}
				}
			}
			else if(bpc.x>boundw-wallc) {
				if(bpc.x<boundw-gap) {
					if(Abs(upc.x-bpc.x)<gap && upc.y>boundh-r1-gap) upc.y = boundh-r1-gap;
				}
				else {
					dist = sqrt(sqr(upc.x-boundw)+sqr(upc.y-boundh));
					if((int)dist<R) {
						upc.x = boundw + R*(upc.x-boundw)/dist;
						upc.y = boundh + R*(upc.y-boundh)/dist;
					}
				}
			}
			
		}
	}
}


void com_update()
{
	float thresholdy, track_rate = 0.3;
	int through;
	cp0c = cpc;
	if(com_change_goal && bv.y<=0) {
		com_change_goal = 0;
		thresholdy = boundh/2-r1;
		if(thresholdy>bpc.y) thresholdy = bpc.y;
		gpc.y = r1+rand()%((int)thresholdy-r1);
		if(bv.y==0) gpc.x = bpc.x;
		else {
			gpc.x = bpc.x + bv.x*(bpc.y-gpc.y)/(-bv.y);
			through = gpc.x/boundw;
			gpc.x = Abs( (through+through%2)*boundw-gpc.x );
			gpc.y -= 15;
		}
	}
	else if(bpc.y>boundh/2 && bv.y>=0) {
		gpc = cvPoint2D32f(boundw/2, winx/2-bound-wall);
	}
	cpc.x += track_rate*(gpc.x-cpc.x);
	cpc.y += track_rate*(gpc.y-cpc.y);
}


void ball_update(float dt)
{
	
	int through, sign, w_flag, h_flag = 0;
	float ddt, w_progress, h_progress;
	CvPoint2D32f uv, cv;
	
	//dt = DT;
	
	// record
	bp0c = bpc;
	// velocity of handle
	uv.x = (upc.x-up0c.x)/dt;
	uv.y = (upc.y-up0c.y)/dt;
	cv.x = (cpc.x-cp0c.x)/dt;
	cv.y = (cpc.y-cp0c.y)/dt;
	
	int count = 0;
	while(1) {
		// predict ball position
		bpc.x = bp0c.x + bv.x*dt;
		bpc.y = bp0c.y + bv.y*dt;
		w_flag = check_wall_reflection(dt, &w_progress);
		h_flag = check_handle_collision(dt, &h_progress, uv, cv);
		if     (w_flag+h_flag==0) break;
		else velocity_correction(w_flag, w_progress, h_flag, h_progress, uv, cv, &dt);
		count++;
		if(count>1000) {
			break;
			bv.x = 0;
			bv.y = 0;
		}
		com_change_goal = h_flag+w_flag;
	}
	if(count) {
		float norm;
		norm = sqrt(sqr(bv.x)+sqr(bv.y));
		if(norm>vmax) {
			norm = vmax/norm;
			bv.x *= norm;
			bv.y *= norm;
		}
		printf("\n%d", count);
	}

}


int check_wall_reflection(float dt, float *w_progress)
{
	int outx1, outx2, outy1, outy2;
	float interceptx;
	outx1 = (bpc.x<0     );
	outx2 = (bpc.x>boundw);
	outy1 = (bpc.y<0     );
	outy2 = (bpc.y>boundh);
	if     (outx1+outx2+outy1+outy2==0) return 0;
	else if(outx1+outx2-outy1-outy2==1) {
		if(outx1) *w_progress =         bp0c.x /(bp0c.x-bpc.x);
		else      *w_progress = (boundw-bp0c.x)/(bpc.x-bp0c.x);
		return 1;
	}
	else {
		if(outy1) *w_progress =         bp0c.y /(bp0c.y-bpc.y); 
		else	  *w_progress = (boundh-bp0c.y)/(bpc.y-bp0c.y);
		interceptx = *w_progress*bpc.x + (1-*w_progress)*bp0c.x;
		if(interceptx<0 || interceptx>boundw) {
			if(outx1) *w_progress = bp0c.x/(bp0c.x - bpc.x);
			else      *w_progress = (boundw-bp0c.x)/(bpc.x-bp0c.x);
			return 1;
		}
		else if(interceptx<wallc || interceptx>boundw-wallc) {
			return 2;
		}
		else {
			if(outy1) {
				if     ( collide_moment(       bp0c.x-wallc, bp0c.y+r0, 0, bv.x, bv.y, dt, w_progress) ) return 3;
				else if( collide_moment(bp0c.x-boundw+wallc, bp0c.y+r0, 0, bv.x, bv.y, dt, w_progress) ) return 4;
				else return 0;
			}
			else {
				if     ( collide_moment(       bp0c.x-wallc, bp0c.y-boundh-r0, 0, bv.x, bv.y, dt, w_progress) ) return 5;
				else if( collide_moment(bp0c.x-boundw+wallc, bp0c.y-boundh-r0, 0, bv.x, bv.y, dt, w_progress) ) return 6;
				else return 0;
			}
		}
	}
}


int check_handle_collision(float dt, float *progress, CvPoint2D32f uv, CvPoint2D32f cv)
{
	int u_flag, c_flag;
	float u_progress, c_progress;
	u_flag = collide_moment(bp0c.x-up0c.x, bp0c.y-up0c.y, r1, bv.x-uv.x, bv.y-uv.y, dt, &u_progress);
	c_flag = collide_moment(bp0c.x-cp0c.x, bp0c.y-cp0c.y, r1, bv.x-cv.x, bv.y-cv.y, dt, &c_progress);
	switch(u_flag+c_flag) {
		case 0:
			return 0;
		case 1: {
			if(u_flag) {
				*progress = u_progress;
				return 1;
			}
			else {
				*progress = c_progress;
				return 2;
			}
		}
		case 2: {
			if(u_progress<c_progress) {
				*progress = u_progress;
				return 1;
			}
			else {
				*progress = c_progress;
				return 2;
			}
		}
	}
}

int collide_moment(float rpx, float rpy, float pr, float rvx, float rvy, float dt, float *progress)
{
	// (rpx+rvx*dt*progress)^2 + (rpy+rvy*dt*progress)^2 = (pr+r0)^2
	// solve "progress", ( 0 < progress < 1 )
	float a, betta, gamma, delta, root1, root2;
	a = sqr(dt)*(sqr(rvx)+sqr(rvy));
	betta = dt*(rvx*rpx+rvy*rpy) / a;				// betta = b/(2a)
	gamma = ( sqr(rpx)+sqr(rpy)-sqr(pr+r0) ) / a;	// gamma = c/a
	delta = sqr(betta)-gamma;
	if(delta<0) return 0;
	else {
		delta = sqrt(delta);
		root1 = -betta+delta;
		root2 = -betta-delta;
		if(root1>=0 && root2>=0 && root2<=1) {
			*progress = root2;
			return 1;
		}
		else return 0;
	}
}

CvPoint2D32f collide_velocity_change(float rpx, float rpy, float rvx, float rvy)
{
	CvPoint2D32f mirrv;
	float mirrx, mirry, norm, proj;
	// "mirr" is the direction perpendicular to the relative vector "rp"
	// "mirrv" is the vector symmetric to relative velocity with direction of "mirr"
	mirrx =  rpy;
	mirry = -rpx;
	norm = sqrt(sqr(rpx)+sqr(rpy));
	mirrx /= norm;
	mirry /= norm;
	proj = (mirrx*rvx+mirry*rvy);
	mirrx *= proj;
	mirry *= proj;
	mirrv.x = 2*mirrx-rvx;
	mirrv.y = 2*mirry-rvy;
	return mirrv;
}


void velocity_correction(int w_flag, float w_progress, int h_flag, float h_progress, CvPoint2D32f uv, CvPoint2D32f cv, float *dt)
{
	float progress, ddt;
	
	if(w_flag && h_flag) {
		if(w_progress<h_progress) {
			progress = w_progress;
			h_flag = 0;
		}
		else {
			progress = h_progress;
			w_flag = 0;
		}
	}
	else {
		if(w_flag) progress = w_progress;
		else	   progress = h_progress;
	}
	// position interpolation
	ddt = *dt*progress;
	bp0c.x += bv.x*ddt;
	bp0c.y += bv.y*ddt;
	up0c.x += uv.x*ddt;
	up0c.y += uv.y*ddt;
	cp0c.x += cv.x*ddt;
	cp0c.y += cv.y*ddt;
	*dt *= 1-progress;
	if(w_flag) {
		bv.x *= dampw;
		bv.y *= dampw;
		switch(w_flag) {
			case 1:
				bv.x = -bv.x;
				break;
			case 2:
				bv.y = -bv.y;
				break;
			case 3:
				bv = collide_velocity_change(bp0c.x-wallc,        bp0c.y+r0, bv.x, bv.y);
				break;
			case 4:
				bv = collide_velocity_change(bp0c.x-boundw+wallc, bp0c.y+r0, bv.x, bv.y);
				break;
			case 5:
				bv = collide_velocity_change(bp0c.x-wallc,        bp0c.y-boundh-r0, bv.x, bv.y);
				break;
			case 6:
				bv = collide_velocity_change(bp0c.x-boundw+wallc, bp0c.y-boundh-r0, bv.x, bv.y);
				break;
		}
	}
	else {
		bv.x *= damph;
		bv.y *= damph;
		switch(h_flag) {
			case 1: {
				bv = collide_velocity_change(bp0c.x-up0c.x, bp0c.y-up0c.y, bv.x-uv.x, bv.y-uv.y);
				bv.x += uv.x;
				bv.y += uv.y;
				break;
			}
			case 2: {
				bv = collide_velocity_change(bp0c.x-cp0c.x, bp0c.y-cp0c.y, bv.x-cv.x, bv.y-cv.y);
				bv.x += cv.x;
				bv.y += cv.y;
				break;
			}
		}
	}
}
