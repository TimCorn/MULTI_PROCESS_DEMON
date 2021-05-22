//
// SClient.cpp
// ~~~~~~~~~~
//

#include <iostream>
#include <boost/array.hpp>
#include <boost/asio.hpp>


#include <boost/lexical_cast.hpp>

#include <iostream>
#include <fstream>

////using asio::ip::tcp;






/**   Client program
 *    Get three parameters - address, port, file name to send to server.
 */
int main(int argc, char* argv[])
{
   enum { max_length =   10000  };   /** Size of buffer for reading. */
   
   std::string  RFileName;  /**  Name of file to read */
 
try
{
  
    if (argc != 4)

    {

       std::cout << "Usage: client <host> <port><file name to send>" << std::endl;

       return 1;

    }



    

    RFileName.assign(argv[3]);   /* path to file to write  */

    
    boost::system::error_code error;
      
    boost::asio::io_context io_context;
    
    boost::asio::ip::tcp::resolver resolver(io_context);
    boost::asio::ip::tcp::resolver::results_type endpoints =
    resolver.resolve(boost::asio::ip::tcp::v4(), argv[1], argv[2]);
    
     boost::asio::ip::tcp::socket socket(io_context);
     boost::asio::connect(socket, endpoints);
     
    //set local socket buffer size
    boost::asio::ip::tcp::socket::receive_buffer_size rbs(16384);
    socket.set_option(rbs);
    
    boost::asio::ip::tcp::socket::send_buffer_size sbs(16384);
    socket.set_option(sbs);
     
    //get local socket buffer size
    boost::asio::ip::tcp::socket::receive_buffer_size grbs;
    socket.get_option(grbs);

    boost::asio::ip::tcp::socket::send_buffer_size gsbs;
    socket.get_option(gsbs);    
  
    std::cout<< "grbs = "<< grbs.value() << std::endl;
    std::cout<< "gsbs = "<< gsbs.value() << std::endl;

  
    std::ifstream fin;  
    boost::array<char, max_length > rbuf;  /** array for reading file. */
 
   fin.open(RFileName, std::ios::in | std::ios::binary);  

   while( fin.good() && !fin.eof() )
   {    
      rbuf.fill(0);
      fin.read(rbuf.data(), max_length);  
      int real_size = static_cast<int>(fin.gcount() );
    
      std::cout << std::endl; 
      
      size_t len = boost::asio::write( socket, boost::asio::buffer(rbuf.data(), real_size ), error );
      len  = len;  /** to press compiler warning*/
      if(error)    {
          std::cout << "ERROR: socket.write() failed error = " << error.message() << std::endl;  
      }
    
   }/*ref. to while() */
   
    fin.close();

}
catch (std::exception& e)
{
   std::cerr << "Exception: " << e.what() << "\n";
}


  return 0;
}




