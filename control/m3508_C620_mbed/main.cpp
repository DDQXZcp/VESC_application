#include "mbed.h"

#include <stdint.h>
//#include <mcp_can.h>
//#include <SPI.h>
//////////////////////////////////////
#include "mbed.h"
//Serial pc(USBTX, USBRX);
char display_buffer[8];


CAN can1(PA_11, PA_12, 1000000);//500000);
CANMessage receive_msg;
#define Motor_1_RevID 0x201
#define Motor_2_RevID 0x202
#define Motor_3_RevID 0x203
#define Motor_4_RevID 0x204


#define Motor_5_RevID 0x205
#define Motor_6_RevID 0x206
#define Motor_7_RevID 0x207
#define Motor_8_RevID 0x208


uint16_t required_current = (int16_t)0;
float required_position   = 0;
uint16_t required_velocity = (int16_t)0;

//const float p_pid_kp = 1;
//const float p_pid_ki = 0.00;
//const float p_pid_kd = 0.0004;

//const float p_pid_kp = 0.1;
//const float p_pid_ki = 0.00;
//const float p_pid_kd = 0.0004;

//slow
const float p_pid_kp = 0.03;
const float p_pid_ki = 0.00;
const float p_pid_kd = 0.0004;
const float p_pid_kd_filter = 0.9;
const float dt = 0.005;

//fast
//const float p_pid_kp = 0.03;
//const float p_pid_ki = 0.00;
//const float p_pid_kd = 0.01;
//const float p_pid_kd_filter = 0.9;
//const float dt = 0.1;

//testing
//const float p_pid_kp = 0.1;
//const float p_pid_ki = 0.00;
//const float p_pid_kd = 0.9;
//const float p_pid_kd_filter = 0.9;
//const float dt = 10;

#define UTILS_LP_FAST(value, sample, filter_constant)   (value -= (filter_constant) * (value - (sample)))

int utils_truncate_number(float *number, float min, float max) {
    int did_trunc = 0;

    if (*number > max) {
        *number = max;
        did_trunc = 1;
    } else if (*number < min) {
        *number = min;
        did_trunc = 1;
    }

    return did_trunc;
}


int utils_truncate_number_abs(float *number, float max) {
    int did_trunc = 0;

    if (*number > max) {
        *number = max;
        did_trunc = 1;
    } else if (*number < -max) {
        *number = -max;
        did_trunc = 1;
    }

    return did_trunc;
}




void CAN_Send(int16_t current)    //CAN 发送 一标准帧数据
{
    CANMessage TxMessage;
    //  CanRxMsg RxMessage;
    /* transmit 1 message */
    TxMessage.id=0x200;          //SID;//0x00;       ID标示符
    //  TxMessage.ExtId=0;
    TxMessage.format=CANStandard;         //CAN_ID_EXT;//        //选择标准帧
    TxMessage.type=CANData;       //选择数据帧
    TxMessage.len=8;

    TxMessage.data[0]=current>>8;           //data1;
    TxMessage.data[1]=current;           //data2;
  
    TxMessage.data[2]=current>>8;            //data3;
    TxMessage.data[3]=current;           //data4;
    TxMessage.data[4]=current>>8;            //data5;
    TxMessage.data[5]=current;          //data6;
    TxMessage.data[6]=current>>8;            //data7;
    TxMessage.data[7]=current;          //data8;
    //TransmitMailbox = CAN_Transmit(CAN1,&TxMessage);
    
    
    can1.write(TxMessage);
  /* receive*/ 
//  RxMessage.StdId=0x00;
//  RxMessage.IDE=CAN_ID_STD;
//  RxMessage.DLC=0;
//  RxMessage.Data[0]=0x00;
//  RxMessage.Data[1]=0x00;
//  CAN_Receive(CAN1,CAN_FIFO0, &RxMessage);

  
}

/*******************************************************************************
* Function Name  : USB_LP_CAN_RX0_IRQHandler
* Description    : This function handles USB Low Priority or CAN RX0 interrupts 
*                  requests.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void RX()//CAN      中断接收程序  
{
   receive_msg.id=0x001; //ignore if receive rubbish
   can1.read(receive_msg);
    //RED_ON;
    //printf("Receivecan!\n");
    switch(receive_msg.id)
    {
        case Motor_1_RevID:
        {
    
    
                 
            uint16_t actual_local_position = (uint16_t)(receive_msg.data[0]<<8)|receive_msg.data[1];
            int16_t  actual_velocity       =  (int16_t)(receive_msg.data[2]<<8)|receive_msg.data[3];
            int16_t  actual_current        =  (int16_t)(receive_msg.data[4]<<8)|receive_msg.data[5]; 
    /*
    ////////////////Velocity//////////////////////////            
            static int16_t last_velocity = 0;
            
            static int velocity_p = 0.3;
            
            if ((required_velocity <800)  && (required_velocity > -800)) {velocity_p = 1.3;} //ok
       else if ((required_velocity <3000) && (required_velocity >-3000)) {velocity_p = 1;}
       else if ((required_velocity <5000) && (required_velocity >-5000)) {velocity_p = 1;}
       else if ((required_velocity <8000) && (required_velocity >-8000)) {velocity_p = 1;}
            
            
            if ((required_velocity - last_velocity)> 10) {CAN_Send( (uint16_t) (required_velocity - last_velocity)* velocity_p);}  //{CAN_Send( (uint16_t)  1000);}
       else if ((required_velocity - last_velocity)<-10) {CAN_Send( (uint16_t) (required_velocity - last_velocity)* velocity_p);}  //{CAN_Send( (uint16_t) -1000);}
       
            last_velocity = actual_velocity;
    ////////////////////////////////////////////////////
    */
   
            static uint16_t start_offset_position     = actual_local_position;
            
            static uint16_t last_angle     = 0;
            static int round_cnt           = 0;
            static int total_ecd           = 0;
            static float total_angle       = 0;
    
            if (actual_local_position - last_angle > 4096)
            {
                round_cnt--;
            }
            else if (actual_local_position - last_angle < -4096)
            {
                round_cnt++;
            }
            else
            { }
        //total encoder value
            total_ecd = round_cnt * 8192 + actual_local_position - start_offset_position;
      //total angle/degree
            total_angle = total_ecd * 360 / 8192;
            
            
           // if (total_angle < position) {CAN_Send((position - total_angle)*  500);}
          //  if (total_angle > position) {CAN_Send((position - total_angle)*  500);}
          
          
          
    static float i_term = 0;
    static float prev_error = 0;
    float p_term;
    float d_term;

    // Compute parameters
    float error = (required_position - total_angle);  // *-1 to invert if needed

    p_term = error * p_pid_kp;
    i_term += error * (p_pid_ki * dt);

    // Average DT for the D term when the error does not change. This likely
    // happens at low speed when the position resolution is low and several
    // control iterations run without position updates.
    // TODO: Are there problems with this approach?
    static float dt_int = 0.0;
    dt_int += dt;
    if (error == prev_error) {
        d_term = 0.0;
    } else {
        d_term = (error - prev_error) * (p_pid_kd / dt_int);
        dt_int = 0.0;
    }

    // Filter D
    static float d_filter = 0.0;
    UTILS_LP_FAST(d_filter, d_term, p_pid_kd_filter);
    d_term = d_filter;


    // I-term wind-up protection
    utils_truncate_number_abs(&p_term, 1.0);
    utils_truncate_number_abs(&i_term, 1.0 - fabsf(p_term));

    // Store previous error
    prev_error = error;

    // Calculate output
    float output = p_term + i_term + d_term;
    utils_truncate_number(&output, -1.0, 1.0);


    int16_t m_iq_set = output * 16384; //ratio to see how many percentage of max current should be applied
    
    CAN_Send( (int16_t) (m_iq_set ) );
      
          
          
          
          
    ///////////////////////////////////////////      
   //              if ((required_position - total_angle)<-10) {CAN_Send( (int16_t) (required_position - total_angle)*  50 );} 
   //         else if ((required_position - total_angle)> 10) {CAN_Send( (int16_t) (required_position - total_angle)*  50 );}
    //////////////////////////////////////////////////////     
         
            //else {CAN_Send( (uint16_t)   0);}
            
            last_angle = actual_local_position;       
            
            //sprintf(display_buffer, "Position %d", actual_local_position);
            //pc.printf(display_buffer);
            //pc.printf("\n\r");
             
            can1.read(receive_msg);
        
                 
        }
        case Motor_2_RevID:
        case Motor_3_RevID:
        case Motor_4_RevID:
        case Motor_5_RevID:
        case Motor_6_RevID:
        case Motor_7_RevID:
        case Motor_8_RevID:
        
                 
        break;
        
        default:
                 break;
    }

}
/**********************************************************************************/
/*
void get_moto_measure(moto_measure_t* ptr, CanRxMsg* hcan)
{
    
      ptr->last_angle    = ptr->angle;
    ptr->angle         = (uint16_t)(hcan->Data[0] << 8 | hcan->Data[1]);
    
    if (ptr->angle - ptr->last_angle > 4096)
        {
            ptr->round_cnt--;
           
        }
    else if (ptr->angle - ptr->last_angle < -4096)
        {
      ptr->round_cnt++;
            
        }
        else
        {
            
        }
        //total encoder value
    ptr->total_ecd = ptr->round_cnt * 8192 + ptr->angle - ptr->offset_angle;
      //total angle/degree
        ptr->total_angle = ptr->total_ecd * 360 / 8192;
}
*/
/*
void get_moto_offset(moto_measure_t* ptr, CanRxMsg* hcan)
{
    ptr->angle        = (uint16_t)(hcan->Data[0] << 8 | hcan->Data[1]);
    ptr->offset_angle = ptr->angle;
}
*/





int main() 
{
    //pc.baud(115200);
    can1.attach(&RX, CAN::RxIrq);
    
    //current = 800;

while(1)
 {
   //CAN_Send(current);
   //required_position = 100;
   //wait_ms(30);
   //required_velocity = 7000;
   /*
   for ( int i=0; i<=69; i++ ) 
   {
       required_velocity = i*100;
       wait_ms(30);
       
   }
   */
   
   
   
   for ( int i=0; i<=20; i++ ) 
   {
       required_position = i*36*19;
       wait_ms(600);
       
   }
   
   required_position = 0;
   wait_ms(1000);
   
   
   }
   
   
   
    
}
