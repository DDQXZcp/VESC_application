# To successfully adapt sin/cos encoder, several files need to be modified

## 1. conf_general.h 

The most inportant!!! 

choose the right hardware version, or the vesc will brick, can use stm32 nucleo board to recover(through ST-link, in windows)

ours are: 

~~~
define HW_SOURCE "hw_410.c" // Also for 4.11 and 4.12
define HW_HEADER "hw_410.h" // Also for 4.11 and 4.12
~~~

## 2. hw_410.h

Add the following, it is copied from hw_axiom.h
~~~
// HW properties
#define HW_HAS_SIN_COS_ENCODER
~~~

## 3. encoder.c

The key function

change the amplitude to corresponding voltage level(for M3508 is 0.3-1.8)

~~~
#define SINCOS_MIN_AMPLITUDE		0.3//1.0			// sqrt(sin^2 + cos^2) has to be larger than this
#define SINCOS_MAX_AMPLITUDE		1.7//1.65		// sqrt(sin^2 + cos^2) has to be smaller than this
~~~

But the range is not fit, need manual adjustment

~~~

~~~

## utils.h 

There is a fast arctan function, which will result in the sudden drop of the M3508 motor

~~~
float utils_fast_atan2(float y, float x) {
	float abs_y = fabsf(y) + 1e-20; // kludge to prevent 0/0 condition
	float angle;

	if (x >= 0) {
		float r = (x - abs_y) / (x + abs_y);
		float rsq = r * r;
		angle = ((0.1963 * rsq) - 0.9817) * r + (M_PI / 4.0);
	} else {
		float r = (x + abs_y) / (abs_y - x);
		float rsq = r * r;
		angle = ((0.1963 * rsq) - 0.9817) * r + (3.0 * M_PI / 4.0);
	}

	if (y < 0) {
		return(-angle);
	} else {
		return(angle);
	}
}
~~~
