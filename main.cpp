#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <linux/can/raw.h>
#include <cstdlib>
#include <iostream>
#include <cstring>
#include <string>
# include<sstream>
#include <iomanip>
#include <linux/can.h>
#include <net/if.h>
#include <cpprest/asyncrt_utils.h>
#include <cpprest/http_client.h>
#include <cpprest/filestream.h>
#include <cpprest/http_listener.h>
#include <cpprest/json.h>

using namespace std;
using namespace utility;                    // Common utilities like string conversions
using namespace web;                        // Common features like URIs.
using namespace web::http;                  // Common HTTP functionality
using namespace web::http::client;          // HTTP client features
using namespace concurrency::streams;       // Asynchronous streams
using namespace web::http::experimental::listener;

json::value jsonResponse;
#define TRACE(msg)            cout << msg
void handle_get(http_request request)
{
    TRACE("\nhandle GET\n");         
    std::ifstream in("example.json");
    std::string contents((std::istreambuf_iterator<char>(in)), 
    std::istreambuf_iterator<char>());
    request.reply(status_codes::OK,  contents.c_str());             
}

/*void handle_post(http_request request)
{

    TRACE("\nhandle Post\n"); 
    request
    .extract_json()
    .then([request](pplx::task<json::value>task)
    {
 
        try
        {
            //Parsing Data From json Post Request        
            auto const jvalue=task.get();
            display_json(jvalue,"R:");
            return;
        }
        catch(http_exception const& e)
        {
          cout << e.what() << endl;
        }
    }
    ).wait();
    
}*/
int skt = socket( PF_CAN, SOCK_RAW, CAN_RAW );
struct can_filter rfilter[1];
struct ifreq ifr;
struct sockaddr_can addr;    
struct can_frame frame;


int main()
{
    ofstream myfile;    
	 
    rfilter[0].can_id   = 0x7E8;
    rfilter[0].can_mask = CAN_EFF_MASK;  

    setsockopt(skt, SOL_CAN_RAW, CAN_RAW_FILTER, rfilter, sizeof(struct can_filter) );

	 //////////////////////////////////////////

   /* Locate the interface you wish to use  Note i am using now For Testing a Virual CAN vcan0 */
 
     strcpy(ifr.ifr_name, "can0");
     ioctl(skt, SIOCGIFINDEX, &ifr); /* ifr.ifr_ifindex gets filled
                          * with that device's index */

   /* Select that CAN interface, and bind the socket to it. */
    
     addr.can_family = AF_CAN;
     addr.can_ifindex = ifr.ifr_ifindex;
     bind( skt, (struct sockaddr*)&addr, sizeof(addr) );

    /* Send a message turifi to the CAN bus */

     frame.can_id = 0x7DF;
     frame.can_dlc = 8;
     frame.data[0]=0x02;
     frame.data[1]=0x01;
     frame.data[2]=0x51;     
     write( skt, &frame, sizeof(frame) );          
     sleep(1);
     frame.data[2]=0x46;
     write( skt, &frame, sizeof(frame) );          
     sleep(1);     
     frame.data[2]=0x0C;
     write( skt, &frame, sizeof(frame) );
     sleep(1);
     frame.data[2]=0x31;
     write( skt, &frame, sizeof(frame) );
     sleep(1);
     frame.data[2]=0x0D;
     write( skt, &frame, sizeof(frame) );
  
//////////////////////////////////////////////////////
//////////////////////////////////////////////////////
//////////////////////////////////////////////////////
 http_listener listener("http://127.0.0.1:8181");

    listener.support(methods::GET, handle_get);
    //listener.support(methods::POST, handle_post);
    //listener.support(methods::PUT, handle_put);
    //listener.support(methods::DEL, handle_del);
    string WithNewlineLogFile;
    
    try
    {
        listener
            .open()
            .then([&listener]() {TRACE("\nstarting to listen\n"); })
            .wait();
        
     do
     {
    
    int bytes_read = read( skt, &frame, sizeof(frame) );   
    
     /////////////////////
    if(frame.data[2]==0x51)
    {   
   
     json::value jsonResponse;
     myfile.open ("example.json", ios::app);
     int Value=(int)frame.data[3];
     if(Value==8)
     {
     cout<<endl<<"Fule is Electric";
     jsonResponse[("Engine coolant Temperature is")]=json::value::string("Fule is Electric");
     WithNewlineLogFile=jsonResponse.serialize();      
     myfile << WithNewlineLogFile<<"\n";     
     myfile.close();  
     }
     else if (Value==1)
     {
      cout<<endl<<"Fule is Gasoline";
      jsonResponse[("Engine coolant Temperature is")]=json::value::string("Fule is Gasoline");
     WithNewlineLogFile=jsonResponse.serialize();      
     myfile << WithNewlineLogFile<<"\n";     
     myfile.close();  
     }
     else if (Value==4)
     {
      cout<<endl<<"Fule is Diesel";
      jsonResponse[("Engine coolant Temperature is")]=json::value::string("Fule is Diesel ");
     WithNewlineLogFile=jsonResponse.serialize();      
     myfile << WithNewlineLogFile<<"\n";     
     myfile.close();  
     }
            
    }
        /////////////////////   
    if(frame.data[2]==0x46)
    {              
     json::value jsonResponse;
     myfile.open ("example.json", ios::app);
     int Value=(int)frame.data[3]-40;
     cout<<endl<<"Outside Car Temperature"<<dec<<(int)frame.data[3]-40<<"°C";
     jsonResponse[("Outside Car Temperature")]=json::value::string(std::to_string(Value)+"°C");
     WithNewlineLogFile=jsonResponse.serialize();      
     myfile << WithNewlineLogFile<<"\n";     
     myfile.close();            
    }
        /////////////////////   
    if(frame.data[2]==0x0C)
    { 
    json::value jsonResponse;             
     cout<<endl<<"Engine speed rpm : "<<dec<<(((int)frame.data[3]*256)+(frame.data[4]))/4;;   
     myfile.open ("example.json", ios::app);
     int Value=(((int)frame.data[3]*256)+(frame.data[4]))/4;  
     jsonResponse[("Engine speed rpm")]=json::value::string(std::to_string(Value));
     WithNewlineLogFile=jsonResponse.serialize();
     myfile << WithNewlineLogFile<<"\n";
     myfile.close();
     
     
    }    
        /////////////////////
    if(frame.data[2]==0x2F)
    {
    json::value jsonResponse;
     cout<<endl<<"Fuel Level Input : "<<dec<<100*(int)frame.data[3]/255<<"%";
      myfile.open ("example.json", ios::app);
     int Value=100*(int)frame.data[3]/255;    
     jsonResponse[("Fuel Level Input")]=json::value::string(std::to_string(Value)+"%");
     WithNewlineLogFile=jsonResponse.serialize();
     myfile << WithNewlineLogFile<<"\n";
     myfile.close();
      
    }
    /////////////////////
    if(frame.can_id==0x7E8&&frame.data[2]==0x0D)
    {     
     json::value jsonResponse;
     cout<<endl<<"Car speed is : "<<dec<<(int)frame.data[3]<<" km/h ";
     myfile.open ("example.json", ios::app);
     int Value=(int)frame.data[3];    
     jsonResponse[("Mototr speed is")]=json::value::string(std::to_string(Value)+" km/h");
     WithNewlineLogFile=jsonResponse.serialize();
     myfile << WithNewlineLogFile<<"\n";
     myfile.close();              
    }
    if(frame.can_id==0x7E8&&frame.data[2]==0x0A)
    {
     cout<<endl<<".............Ending "<<endl;               
     break;
    }
     
    /////////////////////
      cout<<endl<<"======================================="<<endl;
     //
    }while (true);
     myfile.close();        
     listener.close();        
    }
    catch (exception const& e)
    {
        cout << e.what() << endl;
    }
    
 return 0;
}


