#include "mbed.h"
#include "DHT.h"
#include "Pulse.h"
#include "RangeFinder.h"
#include "WakeUp.h"


RangeFinder rf(D12, 10, 5800, 100000); //(Pin, duree du pulse en us, distance [m] = pulse width [us] / 5800 [us / m] , temps de fonctionnement)

DigitalOut led(LED1);
Serial serial(D1, D0);
Serial pc(USBTX, USBRX);
DHT HTSensor(D10,22);
AnalogIn battery(A1);


int main()
{

    pc.printf("Starting...\n\r\n\r");

    float d;
    int dInt;
    
    float temperature;
    int tInt;
    float humidity;
    int hInt;
    float b;
    int bInt;
    int cpt = 0;
  
      
    // Depending on the waterway
    float sensor_limit = 4.0*100.0;
    float danger_limit = 50.0;
    int danger_time = 15;
    int default_time = 30;
    int sending_interval = 3600; //every hour (1h = 3600 s)
    
    while (1)   
    {   
        //reduced battery level measurement (between 0 V et 1.85 V) 
        b = battery.read()*3.3f;
        bInt = (int)(b*100/3.3f);
        
        /*pc.printf("Voltage = %f volts\r\n\r\n", b);
        pc.printf("Voltage code = %d percentage\r\n\r\n", bInt);*/
                 
        //Humidity and temperature measurement
        
        HTSensor.readData();
        wait_us(500);
        
        temperature = HTSensor.ReadTemperature(KELVIN) - 273,15;
        tInt = (int) (temperature + 40);
        humidity = HTSensor.ReadHumidity();
        hInt = (int) humidity;
        
        //level measurement
        d = rf.read_m();
        d = d*100;
        if (d < 0)  
        {
            d = 0;
            //Timeout Error
            WakeUp::set(default_time); //time in second
            deepsleep();
            cpt += default_time; 

        } 
        else if (d >= sensor_limit) 
        {
            // Seeed's sensor has a maximum range of 4m, it returns
            // something like 7m if the ultrasound pulse isn't reflected.
            d = sensor_limit;
            WakeUp::set(default_time); //time in second
            deepsleep();
            cpt += Temps_default; 
        }
        else if (d <= danger_limit) //Inondation
        {
            dInt = (int)d;
            //Data sending
            serial.printf("AT$SF=%04x%02x%02x%02x\r\n",dInt,tInt,hInt, bInt);
            
            WakeUp::set(danger_time); //time in second
            deepsleep();
            cpt += danger_time;
        }
        else
        {            
            //Data sending            
            WakeUp::set(default_time); //time in second
            deepsleep();
            cpt += default_time;
        }
        
        if (cpt >= sending_interval)
        {
            dInt = (int)d;
            serial.printf("AT$SF=%04x%02x%02x%02x\r\n",dInt,tInt,hInt, bInt);
            cpt = 0;
        }

    }
}