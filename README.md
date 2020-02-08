# VESC_application
Firmware modification of VESC

## Steps of making custom firmware

1. Install linux (Not recommended to use VMware, once I was burning firmware, VMware stucked and shut down)

2. choose your project location, enter command: 

$git clone https://github.com/vedderb/bldc

enter bldc folder

$make build/VESC_test.bin -j20 

The .bin name must contain "VESC" at the front

3. use VESC_tool to burn firmware

## Steps of installing VESC_tools

1. Install linux and get package from official website

2. Open it and it will fail by missing a package, use sudo apt-get xxx to install it

3. Then VESC_tool should be able to opened normally
