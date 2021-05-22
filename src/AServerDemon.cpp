//
// AServerDemon .cpp
// ~~~~~~~~~~~~~~~~~~~~~~~~~~
//


#include <boost/asio.hpp>


#include <asio/io_context.hpp>
#include <asio/ip/tcp.hpp>
#include <asio/signal_set.hpp>
#include <asio/write.hpp>
#include <boost/array.hpp>
#include <boost/bind.hpp>
#include <cstdlib>
#include <iostream>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include <boost/filesystem.hpp>



#include <syslog.h>

#include <fstream>


#include <string>






////using asio::ip::tcp;
namespace filesys = boost::filesystem;






class server
{
public:
    
  server(boost::asio::io_context& io_cont, unsigned short port)
    : io_context_(io_cont),
      signal_(io_cont, SIGCHLD),
      acceptor_(io_cont, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port)),
      socket_(io_cont)
  {
    start_signal_wait();
    start_accept();
  }
  

   /*
    * Check if given string path is of a Directory
    */
   bool checkIfDirectory(std::string filePath)
   {
       try {

           // Create a Path object from given path string
           filesys::path pathObj(filePath);
           // Check if path exists and is of a directory file
           if (filesys::exists(pathObj) && filesys::is_directory(pathObj))
               return true;
       }
       catch (filesys::filesystem_error & e)
       {
           std::cout << e.what() << std::endl;
       }
       return false;
   };/*end of checkIfDirectory() */

   
   /*
    *  Create directory tree recursively.
    *  parameters:
    *                   s -    end path
    *                   mode - attributes
    */
   int mkpath(std::string s,mode_t mode)
   {
       size_t pos=0;
       std::string dir;
       int mdret;

       if(s[s.size()-1]!='/'){
           // force trailing / so we can handle everything in loop
           s+='/';
       }
      
       while((pos=s.find_first_of('/',pos))!=std::string::npos){
           dir=s.substr(0,pos++);
           if(dir.size()==0) continue; // if leading / first time is 0 length
               boost::filesystem::path Dir(dir);
               int mkdirres=boost::filesystem::create_directory(Dir);
               if(mkdirres)     {
                   mdret=0;
               }
               else    {
                   mdret=-1;
               }    
       }/*ref. to  while((pos=s.find_first_of('/',pos))!=std::string::npos)*/

       return mdret;

   }; /*end of mkpath()  */
  
  

private:
    
  void start_signal_wait()
  {
    signal_.async_wait(boost::bind(&server::handle_signal_wait, this));
  }

  void handle_signal_wait()
  {
    // Only the parent process should check for this signal. We can determine
    // whether we are in the parent by checking if the acceptor is still open.
    if (acceptor_.is_open())
    {
      // Reap completed child processes so that we don't end up with zombies.
      int status = 0;
      while (waitpid(-1, &status, WNOHANG) > 0) {}

      start_signal_wait();
    }
  }

  void start_accept()
  {
    acceptor_.async_accept(socket_,
        boost::bind(&server::handle_accept, this, _1));
  }

  ////void handle_accept(const asio::error_code& ec)     
  void handle_accept(const boost::system::error_code& ec)
  {
    if (!ec)
    {
      // Inform the io_context that we are about to fork. The io_context cleans
      // up any internal resources, such as threads, that may interfere with
      // forking.
      io_context_.notify_fork(boost::asio::io_context::fork_prepare);

      if (fork() == 0)
      {
        // Inform the io_context that the fork is finished and that this is the
        // child process. The io_context uses this opportunity to create any
        // internal file descriptors that must be private to the new process.
        io_context_.notify_fork(boost::asio::io_context::fork_child);

        // The child won't be accepting new connections, so we can close the
        // acceptor. It remains open in the parent.
        acceptor_.close();

        // The child process is not interested in processing the SIGCHLD signal.
        signal_.cancel();
        
     
    //set local socket buffer size

    boost::asio::ip::tcp::socket::receive_buffer_size rbs(32768);
    socket_.set_option(rbs);

    boost::asio::ip::tcp::socket::send_buffer_size sbs(32768);
    socket_.set_option(sbs);

    //get local socket buffer size
    boost::asio::ip::tcp::socket::receive_buffer_size grbs;
    socket_.get_option(grbs);

    boost::asio::ip::tcp::socket::send_buffer_size gsbs;
    socket_.get_option(gsbs);    

    std::cout<< "grbs = "<< grbs.value() << std::endl;
    std::cout<< "gsbs = "<< gsbs.value() << std::endl;
   
    // set option  SO_REUSEADDR  SO_KEEPALIVE.
    boost::asio::ip::tcp::socket::reuse_address ra(true);
    socket_.set_option(ra);

    boost::asio::ip::tcp::socket::keep_alive ka(true);
    socket_.set_option(ka);

        start_read();
      }
      else
      {
        // Inform the io_context that the fork is finished (or failed) and that
        // this is the parent process. The io_context uses this opportunity to
        // recreate any internal resources that were cleaned up during
        // preparation for the fork.
        io_context_.notify_fork(boost::asio::io_context::fork_parent);

        socket_.close();
        start_accept();
      }
    }
    else
    {
      std::cout << "Accept error: " << ec.message() << std::endl;
      start_accept();
    }
  }

  void start_read()
  {
    socket_.async_read_some(boost::asio::buffer(data_),
        boost::bind(&server::handle_read, this, _1, _2));
  }/*end start_read()  */

  
  
  
  void handle_read(const boost::system::error_code& ec, std::size_t  bytes_transferred /*length*/)
  {
    if (!ec)     {
        
      std::cout << "STOP POINT handle_read(): bytes_transferred = " << bytes_transferred << std::endl; 
         
      
//// Here we will treat received data. Write it to a file.     
      pid_t curr_pid =getpid();
      char curr_pid_str[20];
      sprintf(curr_pid_str, "%d", curr_pid);
      std::string fname(curr_pid_str);
      fname = fname + ".dat";
      WFileName=ResultDir+"/"+fname;
      fout.open(WFileName, std::ios::out | std::ios::app);
        
      std::cout<<"bytes_transferred: " << bytes_transferred << std::endl;
      for(std::size_t i=0;i<bytes_transferred;i++)   {
        fout <<  data_[i]; 
      }  
      
      fout.close();  
     
      start_read();
      
    }  
    
    else if ( ec == boost::asio::error::connection_reset )       {
        std::cout << "connection_reset = " << "connection_reset" << std::endl; 
        this->io_context_.stop();
    }
    else if ( ec == boost::asio::error::eof )       {
        std::cout << "eof = " << "eof" << std::endl; 
        this->io_context_.stop();
        //exit(0);
    }
    else
    {
      std::cout << "STOP POINT handle_read() error = " << ec.message() << std::endl; 
      std::cout << "STOP POINT handle_read() err code = " << ec.category().name() << ':' << ec.value() << std::endl; 
      std::cout << "STOP POINT handle_read() typeid(this).name() =  " << typeid(this).name() << std::endl; 
      this->io_context_.stop();
    }
      
  }/*end of handle_read()  */
  
  

  void start_write(std::size_t length)
  {
    boost::asio::async_write(socket_, boost::asio::buffer(RepData, length),
        boost::bind(&server::handle_write, this, _1));
  }

  
  
  void handle_write(const boost::system::error_code& ec)
  {
    if (!ec)           {
      start_read();
    }
    else
    {
      std::cout << "STOP POINT handle_write() error = " << ec.message() << std::endl; 
      delete this; /// - ???
    }    
      
      
  }
  


    
  boost::asio::io_context& io_context_;
  boost::asio::signal_set signal_;
  boost::asio::ip::tcp::acceptor acceptor_;
  boost::asio::ip::tcp::socket socket_;
  
  enum { max_length = 10000 };
  
  boost::array<char, max_length> data_;
  
  char RepData[max_length];
  
  std::ofstream fout;     /** Descriptor of file in which do writing. */
  
 public:  
     
  std::string WFileName;  /* Name of a file in which do writing. */
  std::string  ResultDir = "SERVER_FILES"; /** Directory name to keep resulting files. */
  
};






void handler(boost::asio::io_context& this_io, boost::asio::signal_set& this_, boost::system::error_code error, int signal_number)   
{
    
  if (!error)
  {

    // A signal occurred.
     pid_t parent= getppid();   /** if ppid = 1 it is parent process. */

      
     if (signal_number == SIGTERM )    {
          syslog(LOG_INFO | LOG_USER, "handler(): pid=%d  ppid=%d got signal SIGTERM to complete !!!", getpid(), getppid() );
          this_io.boost::asio::io_context::stop();
     }
     else if (signal_number == SIGINT)      {
        // SIGINT will not treat in child proces.
        if( parent == 1)        {
          syslog(LOG_INFO | LOG_USER, "handler(): pid=%d ppid=%d got SIGINT Do Nothing !!!", getpid(), getppid() );
        }
     }     
     
     else if (signal_number == SIGHUP)      {
        syslog(LOG_INFO | LOG_USER, "handler(): pid=%d  ppid=%d got signal SIGHUP to complete !!!", getpid(), getppid() );
        this_io.boost::asio::io_context::stop(); 
     }
     
     this_.async_wait(

        boost::bind(handler,  boost::ref(this_io), boost::ref(this_), _1, _2));
  }   
  else       {
     std::cout << "STOP POINT ERROR handler() error = " << error.message() << "   pid= " << getpid() << std::endl;
     syslog(LOG_ERR | LOG_USER, "STOP POINT ERROR handler() error:  %s    pid=%d", error.message().c_str(), getpid() );
  }

}/* end of handler() */








/*
 *  Get one parameters:   port number  
 *  The name of file to write chosen from process pid.
 * 
 */
int main(int argc, char* argv[])
{
    
  try
  {
     
    if (argc != 2)
    {
      std::cout << "Usage:  <port>\n";
      return 1;
    }   
     
    boost::asio::io_context io;
    server s(io, std::atoi(argv[1]) );
    
    /** Check if result directory SERVER_FILES exist in current folder. If not create it. */  
    bool dirresult = s.checkIfDirectory(s.ResultDir);
    if(dirresult==false)     {
      /** create new directory*/
      int mkdirretval;
      std::cout << "Directory " << s.ResultDir << " not exists. Create it." << std::endl;
      mkdirretval=s.mkpath(s.ResultDir,0755);
      if(mkdirretval == 0)     {
            ;   
      }
      else       {
         std::cout << "ERROR IN  creating Directory " << s.ResultDir << std::endl;               
      }               

   }/*ref. to if(dirresult==false)  */

   else    {
         std::cout << "Directory " << s.ResultDir << "  already exists." << std::endl;    
   }
      
   
    // Register signal handlers so that the daemon may be shut down. You may
    // also want to register for other signals, such as SIGHUP to trigger a
    // re-read of a configuration file.
      
    boost::asio::signal_set signals(io, SIGINT);
    
    signals.add(SIGTERM);
    signals.add(SIGHUP);
    
   // Start an asynchronous wait for one of the signals to occur.
   signals.async_wait(
        boost::bind(handler, boost::ref(io), boost::ref(signals), _1, _2));
        
    
    
    // Inform the io_context that we are about to become a daemon. The
    // io_context cleans up any internal resources, such as threads, that may
    // interfere with forking.
    io.notify_fork(boost::asio::io_context::fork_prepare);

    // Fork the process and have the parent exit. If the process was started
    // from a shell, this returns control to the user. Forking a new process is
    // also a prerequisite for the subsequent call to setsid().
    if (pid_t pid = fork())
    {
      if (pid > 0)
      {
        // We're in the parent process and need to exit.
        //
        // When the exit() function is used, the program terminates without
        // invoking local variables' destructors. Only global variables are
        // destroyed. As the io_context object is a local variable, this means
        // we do not have to call:
        //
        //   io_context.notify_fork(asio::io_context::fork_parent);
        //
        // However, this line should be added before each call to exit() if
        // using a global io_context object. An additional call:
        //
        //   io_context.notify_fork(asio::io_context::fork_prepare);
        //
        // should also precede the second fork().
        exit(0);
      }
      else
      {
        syslog(LOG_ERR | LOG_USER, "First fork failed: %m");
        return 1;
      }
    }

    // Make the process a new session leader. This detaches it from the
    // terminal.
    setsid();

    // A process inherits its working directory from its parent. This could be
    // on a mounted filesystem, which means that the running daemon would
    // prevent this filesystem from being unmounted. Changing to the root
    // directory avoids this problem.
    
    //chdir("/");
    //
    
    // The file mode creation mask is also inherited from the parent process.
    // We don't want to restrict the permissions on files created by the
    // daemon, so the mask is cleared.
    umask(0);

    // A second fork ensures the process cannot acquire a controlling terminal.
    if (pid_t pid = fork())
    {
      if (pid > 0)
      {
        exit(0);
      }
      else
      {
        syslog(LOG_ERR | LOG_USER, "Second fork failed: %m");
        return 1;
      }
    }

    // Close the standard streams. This decouples the daemon from the terminal
    // that started it.
    close(0);
    close(1);
    close(2);

    // We don't want the daemon to have any standard input.
    if (open("/dev/null", O_RDONLY) < 0)
    {
      syslog(LOG_ERR | LOG_USER, "Unable to open /dev/null: %m");
      return 1;
    }

    // Send standard output to a log file.
    const char* output = "/tmp/asio.daemon.out";
    const int flags = O_WRONLY | O_CREAT | O_APPEND;
    ////const int flags = O_WRONLY /*| O_TRUNC*/;
    const mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;
    if (open(output, flags, mode) < 0)
    {
      syslog(LOG_ERR | LOG_USER, "Unable to open output file %s: %m", output);
      return 1; 
    }

    // Also send standard error to the same log file.
    if (dup(1) < 0)
    {
      syslog(LOG_ERR | LOG_USER, "Unable to dup output descriptor: %m");
      return 1;
    }

    
    // Inform the io_context that we have finished becoming a daemon. The
    // io_context uses this opportunity to create any internal file descriptors
    // that need to be private to the new process.
    io.notify_fork(boost::asio::io_context::fork_child);
    
    // The io_context can now be used normally.
    syslog(LOG_INFO | LOG_USER, "Daemon started");
    io.run();
    syslog(LOG_INFO | LOG_USER, "Daemon stopped");
  }
  catch (std::exception& e)
  {
    syslog(LOG_ERR | LOG_USER, "Exception: %s", e.what());
    std::cout << "Exception: " << e.what() << std::endl;
  }    
 
  
}/*end main() */
