#pragma once

struct RGB
{
	unsigned char R;
	unsigned char G;
	unsigned char B;
};

struct HSL
{
	int H;
	float S;
	float L;
};

RGB HSLToRGB(HSL hsl);
float HueToRGB(float v1, float v2, float vH);

