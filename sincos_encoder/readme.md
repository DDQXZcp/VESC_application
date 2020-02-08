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
