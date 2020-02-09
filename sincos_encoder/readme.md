# To successfully adapt sin/cos encoder, several files need to be modified

## 1. conf_general.h 

The most inportant!!! 

choose the right hardware version, or the vesc will brick. We can use stm32 nucleo board to recover(through ST-link, in windows)

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
#ifdef HW_HAS_SIN_COS_ENCODER
	case ENCODER_MODE_SINCOS: {
		float sin = ENCODER_SIN_VOLTS * sin_gain - sin_offset;// 2.2 , 0.66
		float cos = ENCODER_COS_VOLTS * cos_gain - cos_offset;
		//float sin = (ENCODER_SIN_VOLTS-0.3)*2.2;//will be enable to test motor
		//float cos = (ENCODER_COS_VOLTS-0.3)*2.2;
		float module = SQ(sin) + SQ(cos);

		if (module > SQ(SINCOS_MAX_AMPLITUDE) )	{
			// signals vector outside of the valid area. Increase error count and discard measurement
			++sincos_signal_above_max_error_cnt;
			UTILS_LP_FAST(sincos_signal_above_max_error_rate, 1.0, 1./SINCOS_SAMPLE_RATE_HZ);
			angle = last_enc_angle;
		}
		else {
			if (module < SQ(SINCOS_MIN_AMPLITUDE)) {
				++sincos_signal_below_min_error_cnt;
				UTILS_LP_FAST(sincos_signal_low_error_rate, 1.0, 1./SINCOS_SAMPLE_RATE_HZ);
				angle = last_enc_angle;
			}
			else {
				UTILS_LP_FAST(sincos_signal_above_max_error_rate, 0.0, 1./SINCOS_SAMPLE_RATE_HZ);
				UTILS_LP_FAST(sincos_signal_low_error_rate, 0.0, 1./SINCOS_SAMPLE_RATE_HZ);
				
				//float angle_tmp = (utils_fast_atan2(sin, cos) * 180.0 / M_PI+180)*4.5;
				float angle_tmp = (utils_fast_atan2(cos, sin) * 180.0 / M_PI + 170)*5;
				//float angle_tmp = utils_fast_atan2(sin, cos) * 180.0 / M_PI;
				//float angle_tmp = asin(cos)*180.0/M_PI;//will cause error,serial port error
///
				if (angle_tmp>359)
					angle_tmp = 359;
				if (angle_tmp<1)
					angle_tmp = 1;
				if((angle_tmp - angle_tmp2)> 350)
					loop_cnt--;
				if((angle_tmp - angle_tmp2)< -350)
					loop_cnt++;
				//angle_tmp = 60*loop_cnt;cannot detect encoder
///
				//angle_tmp = angle_tmp/7 + (loop_cnt%7)*360.0/7;
				UTILS_LP_FAST(angle, angle_tmp, sincos_filter_constant);
				last_enc_angle = angle;
			}
		}
		break;
	}
#endif
~~~

Key sentence is this one

~~~
float angle_tmp = (utils_fast_atan2(cos, sin) * 180.0 / M_PI + 170)*5;
~~~

But still have not figured out why the drop is not sudden but gradually 

(There are 7 rise & fall of M3508, corresponding to 7 pair of magnets)

## utils.h 

There is a fast arctan function, which should result in the sudden drop of the M3508 motor

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
