// ************************************************************************
//
//               Demo program for labs
//
// Subject:      Computer Architectures and Parallel systems
// Author:       Petr Olivka, petr.olivka@vsb.cz, 09/2019
// Organization: Department of Computer Science, FEECS,
//               VSB-Technical University of Ostrava, CZ
//
// File:         OpenCV simulator of LCD
//
// ************************************************************************

#include <stdio.h>
#include <stdlib.h>
//#include <unistd.h>
#include <opencv2/opencv.hpp>
#include "font8x8.cpp"
#include "math.h"
#include "time.h"

//#include "fonts/font12x16_lsb.h"

#define LCD_WIDTH       320
#define LCD_HEIGHT      240
#define LCD_NAME        "Virtual LCD"

// LCD Simulator

// Virtual LCD
cv::Mat g_canvas(cv::Size(LCD_WIDTH, LCD_HEIGHT), CV_8UC3);

// Put color pixel on LCD (canvas)
void lcd_put_pixel(int t_x, int t_y, int t_rgb_565)
{
	// Transform the color from a LCD form into the OpenCV form. 
	cv::Vec3b l_rgb_888(
		(t_rgb_565 & 0x1F) << 3,
		((t_rgb_565 >> 5) & 0x3F) << 2,
		((t_rgb_565 >> 11) & 0x1F) << 3
	);
	g_canvas.at<cv::Vec3b>(t_y, t_x) = l_rgb_888; // put pixel
}

// Clear LCD
void lcd_clear()
{
	cv::Vec3b l_black(0, 0, 0);
	g_canvas.setTo(l_black);
}

// LCD Initialization 
void lcd_init()
{
	cv::namedWindow(LCD_NAME, 0);
	lcd_clear();
	cv::waitKey(1);
}


// Simple graphic interface

struct Point2D
{
	int32_t x, y;
};

struct RGB
{
	uint8_t r, g, b;
};

class GraphElement
{
public:
	// foreground and background color
	RGB fg_color, bg_color;

	// constructor
	GraphElement(RGB t_fg_color, RGB t_bg_color) :
		fg_color(t_fg_color), bg_color(t_bg_color) {}

	// ONLY ONE INTERFACE WITH LCD HARDWARE!!!
	void drawPixel(int32_t t_x, int32_t t_y) { lcd_put_pixel(t_x, t_y, convert_RGB888_to_RGB565(fg_color)); }

	// Draw graphics element
	virtual void draw() = 0;

	// Hide graphics element
	virtual void hide() { swap_fg_bg_color(); draw(); swap_fg_bg_color(); }
private:
	// swap foreground and backgroud colors
	void swap_fg_bg_color() { RGB l_tmp = fg_color; fg_color = bg_color; bg_color = l_tmp; }

	// IMPLEMENT!!!
	// conversion of 24-bit RGB color into 16-bit color format
	int convert_RGB888_to_RGB565(RGB t_color) {
		union URGB {
			struct {
				int b : 5;
				int g : 6;
				int r : 5;
			};
			short rgb565;
		} urgb;
		urgb.r = (t_color.r >> 3) & 0x1F;
		urgb.g = (t_color.g >> 2) & 0x3F;
		urgb.b = (t_color.b >> 3) & 0x1F;
		return urgb.rgb565;
	}
};


class Pixel : public GraphElement
{
public:
	// constructor
	Pixel(Point2D t_pos, RGB t_fg_color, RGB t_bg_color) : pos(t_pos), GraphElement(t_fg_color, t_bg_color) {}
	// Draw method implementation
	virtual void draw() { drawPixel(pos.x, pos.y); }
	// Position of Pixel
	Point2D pos;
};


class Circle : public GraphElement
{
public:
	// Center of circle
	Point2D center;
	// Radius of circle
	int32_t radius;

	Circle(Point2D t_center, int32_t t_radius, RGB t_fg, RGB t_bg) :
		center(t_center), radius(t_radius), GraphElement(t_fg, t_bg) {};

	void draw() {
		int x = radius - 1;
		int y = 0;
		int dx = 1;
		int dy = 1;
		int err = dx - (radius << 1);

		while (x >= y)
		{
			drawPixel(center.x + x, center.y + y);
			drawPixel(center.x + y, center.y + x);
			drawPixel(center.x - y, center.y + x);
			drawPixel(center.x - x, center.y + y);
			drawPixel(center.x - x, center.y - y);
			drawPixel(center.x - y, center.y - x);
			drawPixel(center.x + y, center.y - x);
			drawPixel(center.x + x, center.y - y);

			if (err <= 0)
			{
				y++;
				err += dy;
				dy += 2;
			}

			if (err > 0)
			{
				x--;
				dx += 2;
				err += dx - (radius << 1);
			}
		}
	}



	void drawSecondLine(int x1, int y1, int x2, int y2, int color) {
		int x, y, dx, dy, dx1, dy1, px, py, xe, ye, i;
		dx = x2 - x1;
		dy = y2 - y1;
		dx1 = fabs(dx);
		dy1 = fabs(dy);
		px = 2 * dy1 - dx1;
		py = 2 * dx1 - dy1;
		if (dy1 <= dx1) {
			if (dx >= 0) {
				x = x1;
				y = y1;
				xe = x2;
			}
			else {
				x = x2;
				y = y2;
				xe = x1;
			}
			lcd_put_pixel(x, y, color);
			for (i = 0; x < xe; i++) {
				x = x + 1;
				if (px < 0) {
					px = px + 2 * dy1;
				}
				else {
					if ((dx < 0 && dy < 0) || (dx > 0 && dy > 0)) {
						y = y + 1;
					}
					else {
						y = y - 1;
					}
					px = px + 2 * (dy1 - dx1);
				}
				lcd_put_pixel(x, y, color);
			}
		}
		else {
			if (dy >= 0) {
				x = x1;
				y = y1;
				ye = y2;
			}
			else {
				x = x2;
				y = y2;
				ye = y1;
			}
			lcd_put_pixel(x, y, color);
			for (i = 0; y < ye; i++) {
				y = y + 1;
				if (py <= 0) {
					py = py + 2 * dx1;
				}
				else {
					if ((dx < 0 && dy < 0) || (dx > 0 && dy > 0)) {
						x = x + 1;
					}
					else {
						x = x - 1;
					}
					py = py + 2 * (dx1 - dy1);
				}
				lcd_put_pixel(x, y, color);
			}
		}
	}

};

class Character : public GraphElement
{
public:
	// position of character
	Point2D pos;
	// character
	char character;



	Character(Point2D t_pos, char t_char, RGB t_fg, RGB t_bg) :
		pos(t_pos), character(t_char), GraphElement(t_fg, t_bg) {};

	/* //	FONT 12x16
	void draw() {
		// width
		for (int i = 0; i < 16; i++) {
			// height
			for (int j = 0; j < 16; j++) {
				if (font[character][i] & 1 << j) {
					drawPixel(pos.x + j, pos.y + i);
				}
			}
		}
	}
	*/

	void draw() {
		// width
		for (int i = 0; i < 8; i++) {
			// height
			for (int j = 0; j < 8; j++) {
				if (font8x8[character][i] & 1 << j) {
					drawPixel(pos.x + j, pos.y + i);
				}
			}
		}
	}

};

class Line : public GraphElement
{
public:
	// the first and the last point of line
	Point2D pos1, pos2;

	Line(Point2D t_pos1, Point2D t_pos2, RGB t_fg, RGB t_bg) :
		pos1(t_pos1), pos2(t_pos2), GraphElement(t_fg, t_bg) {}

	void drawLine()
	{
		int x0 = pos1.x;
		int y0 = pos1.y;
		int x1 = pos2.x;
		int y1 = pos2.y;

		int dx = abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
		int dy = -abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
		int err = dx + dy, e2; //error value e_xy

		for (;;) {  //loop
			drawPixel(x0, y0);
			if (x0 == x1 && y0 == y1) break;
			e2 = 2 * err;
			if (e2 >= dy) { err += dy; x0 += sx; } //e_xy+e_x > 0
			if (e2 <= dx) { err += dx; y0 += sy; } //e_xy+e_y < 0
		}
	}


	void draw() {
		int dx = pos2.x - pos1.x; //pos1 centerX=160    //pos2 x=230
		int dy = pos2.y - pos1.y; //pos1 centerY=120    //pos2 y=120
		for (int x = pos1.x; x < pos2.x; x++)
		{
			int y = pos1.y + dy * (x - pos1.x) / dx;
			drawPixel(x, y);

		}
	};



};

#define LCD_SCALING		2

void lcd_show(int wait = -1) {
	if (cv::getWindowProperty(LCD_NAME, cv::WindowPropertyFlags::WND_PROP_VISIBLE)) {
		cv::imshow(LCD_NAME, g_canvas);
	}
	else {
		cv::destroyWindow(LCD_NAME);
		lcd_init();
		cv::resizeWindow(LCD_NAME, LCD_WIDTH * LCD_SCALING, LCD_HEIGHT * LCD_SCALING);
	}

	if (wait >= 0)
		cv::waitKey(wait);
}


Point2D zahrada[60] = {};
Point2D Clock[15] = {};
Point2D Min[60] = {};
Point2D Hour[12] = {};

void ClockCoords()
{
	for (int j = 0; j < 12; j++) {
		Clock[j].x = (160 + 80 * cos(30 * (j * (3.14159 / 180))));
		Clock[j].y = (120 + 80 * sin(30 * (j * (3.14159 / 180))));
	}
}


void SecXY() {
	for (int i = 0; i < 60; i++) {
		zahrada[i].x = (160 + 90 * cos(6 * (i * (3.14159 / 180))));
		zahrada[i].y = (120 + 90 * sin(6 * (i * (3.14159 / 180))));
	}
}

void MinXY() {
	for (int i = 0; i < 60; i++) {
		Min[i].x = (160 + 75 * cos(6 * (i * (3.14159 / 180))));
		Min[i].y = (120 + 75 * sin(6 * (i * (3.14159 / 180))));
	}
}

void HourXY() {
	for (int i = 0; i < 12; i++) {
		Hour[i].x = (160 + 40 * cos(30 * (i * (3.14159 / 180))));
		Hour[i].y = (120 + 40 * sin(30 * (i * (3.14159 / 180))));
	}
}

void printNumbers() {
	char allClockNumbers[12][2] = { {'3'}, {'4'}, {'5'}, {'6'}, {'7'}, {'8'}, {'9'},{'1','0'},{'1','1'},{'1','2'}, {'1'}, {'2'} };

	for (int i = 0; i < 12; i++) {
		for (int j = 0; j < 2; j++) {
			if (allClockNumbers[i][1] == '\0') {
				Character ClockDraw(Clock[i], allClockNumbers[i][j], { 255,255,0 }, { 0,0,0 });
				ClockDraw.draw();
			} else {
				Character clockDraw({ Clock[i].x + 8 * j - 2, Clock[i].y}, allClockNumbers[i][j], { 255,255,0 }, { 0,0,0 });
				clockDraw.draw();
			}
		}
	}
}


void drawName(char brandName[]) {
	int margin = 0;

	for (int i = 0; i < 11; i++) {
		if (i <= 5) {
			Character Text({ 135 + 8 * margin, 150 }, brandName[i], { 255,100,0 }, { 0,0,0 });
			Text.draw();
		}
		else if (i > 5) {
			Character Text2({ 140 + 8 * (margin - 6), 160 }, brandName[i], { 255,100,0 }, { 0,0,0 });
			Text2.draw();
		}
		margin++;
	}
}


void drawFrame() {
	for (int i = 0; i < 5; i++)
	{
		Circle* c = new Circle({ 160,120 }, 90 + i, { 255,255,255 }, { 0,0,0 });
		c->draw();
	}
}

void drawHands(int &centerX, int &centerY, int &x, int &y, int &z) {
	Line Seconds({ centerX,centerY }, zahrada[x++], { 255,0,0 }, { 0,0,0 });
	Seconds.drawLine();

	Line Minutes({ centerX,centerY }, Min[y], { 0,255,0 }, { 0,0,0 });
	Minutes.drawLine();

	Line Hours({ centerX, centerY }, Hour[z], { 0,0,255 }, { 0,0,0 });
	Hours.drawLine();
	if (x == 60)
	{
		x = 0;
	}
	if (x == 45)
	{
		y++;

		if (y == 60)
		{
			y = 0;
			z++;
			if (z == 12)
			{
				z = 0;
			}
		}
	}
}

int main()
{
	lcd_init();                     // LCD initialization
	lcd_clear();

	int x = 46;
	int y = 55;
	int z = 5;
	int centerX = 160, centerY = 120;
	char brandName[] = { 'H', 'O', 'D', 'I', 'N', 'Y', 'C', 'A', 'S', 'I', 'O' };
	

	SecXY();
	MinXY();
	HourXY();
	ClockCoords();

	while (1) {
		lcd_clear();
		
		printNumbers();
		drawName(brandName);
		drawFrame();
		drawHands(centerX, centerY, x, y, z);
		
		lcd_show(1000);
	}
	cv::imshow(LCD_NAME, g_canvas);   // refresh content of "LCD"
	cv::waitKey(0);                   // wait for key 

}