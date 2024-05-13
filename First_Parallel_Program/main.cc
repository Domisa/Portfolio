//How to run MPI natively on local linux system
//cd into the diretory with with project files
//mpirun -np 6 ./project1 easy_sample.dat sol_easy.04
//np designated processor count to use easy_sample.dat is data to crunch. sol_easy.04 is the output file

#include "game.h"
#include "utilities.h"
// Standard Includes for MPI, C and OS calls
#include <mpi.h>

// C++ standard I/O and library includes
#include <iostream>
#include <fstream>
#include <vector>
#include <string>

// C++ stadard library using statements
using std::cout ;
using std::cerr ;
using std::endl ;

using std::vector ;
using std::string ;

using std::ofstream ;
using std::ifstream ;
using std::ios ;


void Server(int argc, char *argv[]) {

//Generically used to 
MPI_Status stat;
MPI_Request request;
MPI_Request request2;

int flag;
int answers;
int results;
int flagged;

move solution[IDIM*JDIM];

// Check to make sure the server can run
  if(argc != 3) {
    cerr << "two arguments please!" << endl ;
    MPI_Abort(MPI_COMM_WORLD,-1) ;
  }

  // Input case filename 
  ifstream input(argv[1],ios::in) ;

   // Output case filename
  ofstream output(argv[2],ios::out) ;

  int count = 0;
  int NUM_GAMES = 0;

  //This is where I will find the number of games and eventually setup the chunking
  //Reads in number of games from file
  input >> NUM_GAMES ;
  
  //The difficulty here is that each row contains 25 characters. To chunk, I need multiple rows. I am choosing 5 here.
  //NUM_GAMES/5 is because the chunks each take 10 games.

  for(int i=0;i<NUM_GAMES/10;++i) {  // for each game in file...
    
    string input_string;
    string bucket = "";
    
    int chunk = 0;

    //Create a chunk
    while(chunk!=10){

      //I am hoping this block will put the whole line into the input_string, if so, then I can just repeat this five times
      //Confirmed, will repeat if left like this.-96
      input >> input_string;

      // This is just for checking if the file is not accurate. Ignore this.
      if(input_string.size() != IDIM*JDIM) {
      cerr << "something wrong in input file format!" << endl ;
      MPI_Abort(MPI_COMM_WORLD,-1) ;
      }

      bucket = input_string + bucket;

      chunk++;

  
    }
    
    // read in the initial game state from file. From the original, I multiplied by 10 to include the chunk.
    // the client will handle the combined 10 games
    unsigned char buf[IDIM*JDIM*10] ;
      for(int j=0;j<IDIM*JDIM*10;++j)
        buf[j] = bucket[j] ;
      
    
    //There is a sacrifice in efficiency here. The server can only do work in batches of ten for now

    //First receive information about an available client

    //When moving to grad, this has to be the Irecv,
    //This was a ping by the client to ask for work
    MPI_Irecv(&flagged, 1, MPI_INT, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &request);
    MPI_Test(&request, &flag, &stat);
    
    while(i < 9){
      //That would be the last chunk of the game, the server can solve these after closing the clients

      while(flag==true){
        
        MPI_Send(&buf, IDIM*JDIM*10, MPI_UNSIGNED_CHAR, stat.MPI_SOURCE, 1, MPI_COMM_WORLD);
        

        //This is for checking if there are any results waiting to be collected
        MPI_Irecv(&results, 1, MPI_INT, stat.MPI_SOURCE, 3, MPI_COMM_WORLD, &request2);
        MPI_Test(&request2, &answers, &stat);
        

        if(answers==true){
          
          //cout << results << ' ' << endl;

          if(results==1){
          int size = 0;
          unsigned char gameday[IDIM*JDIM];

          
          MPI_Recv(&solution, JDIM*IDIM, MPI_UNSIGNED_CHAR, stat.MPI_SOURCE, 2, MPI_COMM_WORLD, &stat);
          MPI_Recv(&gameday, IDIM*JDIM, MPI_UNSIGNED_CHAR, stat.MPI_SOURCE, 5, MPI_COMM_WORLD, &stat);

          cout << solution << " " << stat.MPI_SOURCE;
          output << "found solution = " << endl ;

          game_state s ;
          s.Init(gameday) ;
          s.Print(output) ;
          for(int i=0;i<size;++i) {
            s.makeMove(solution[i]) ;
            output << "-->" << endl; 
            s.Print(output) ;
          }
          output << "solved" << endl ;
          count++ ;

        } else {
          cout << "no cigar " << endl;
        }
        }
        
        MPI_Irecv(&results, 1, MPI_INT, stat.MPI_SOURCE, 3, MPI_COMM_WORLD, &request);

        MPI_Test(&request, &flag, MPI_STATUS_IGNORE);

      }

      if(flag==false){
        MPI_Test(&request, &flag, &stat);
        for(int i=0; i<10; ++i) {
          unsigned char gameday[IDIM*JDIM];

        //if flag is not true, then do work
        //form the game
          for(int j=0; i<IDIM*JDIM; ++j) {
            gameday[j] = buf[j+(i*25)];
          }

          game_state game_board ;
          game_board.Init(gameday) ;

          // If we find a solution to the game, put the results in
          // solution
          move solution[IDIM*JDIM] ;
          int size = 0 ;
          // Search for a solution to the puzzle
          bool found = depthFirstSearch(game_board,size,solution) ;

          // If the solution is found we want to output how to solve the puzzle
          // in the results file.
          if(found) {
            output << "found solution = " << endl ;
            game_state s ;
            s.Init(gameday) ;
            s.Print(output) ;
            for(int i=0;i<size;++i) {
              s.makeMove(solution[i]) ;
              output << "-->" << endl; 
              s.Print(output) ;
            }
            output << "solved" << endl ;
            count++ ;
            }
            }

    }
    }
    
  }


cout << "found " << count << " solutions" << endl ;



}
    





// Put the code for the client here
void Client(int argc, char *argv[]) {

  unsigned char buf[IDIM*JDIM*10];
  int test_drive;
  bool active=true;
  MPI_Status stat;
  int flagged;

  while(active){
    MPI_Send(&flagged, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
    
    //cout<< "Work Request sent" << endl;

    MPI_Recv(&buf, IDIM*JDIM*10, MPI_UNSIGNED_CHAR, 0, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    //cout<<"Muscle received " << endl;

    //This revealed the problem with the organized data
    //cout<< buf << endl;

    //My attempt at closing the client when it is finished. It doesnt seem to work
    if(&buf[0]==0){
      break;
    }




  //At this time, the buffer has 125 characters in it, and each game is only 25 games long.
  //Time to separate them
  
  
  for(int i=0; i < 10; ++i) {

    //cout << "help ";
    unsigned char gameday[IDIM*JDIM];


    //string results;
    for(int j = 0; j<IDIM*JDIM; ++j){

      //The idea is that for every game, all 25 characters will go to the new variable
      //The math idea is new game "i"*25 will gave the placement of char j in buf to assign
      //to index j in gameday.
      gameday[j] = buf[j+(i*25)];
      
      
      }
    //cout << endl;
    
    
    //cout << gameday << endl;
    
    // //Time to play the game
    game_state game_board ;
    game_board.Init(gameday) ;
    int count = 0;

    // // If we find a solution to the game, put the results in
    // // solution
    move solution[IDIM*JDIM] ;
    int size = 0 ;

    bool found = depthFirstSearch(game_board,size,solution) ;
    //cout << found << " " << endl;

    // // If the solution is found we want to output how to solve the puzzle
    // // in the results file.
     if(found) {
      //cout << "gottem " << endl;
      int results = 1;
      MPI_Send(&results,1, MPI_INT, 0, 3, MPI_COMM_WORLD);
      //cout << gameday << endl;
      MPI_Send(&solution, JDIM*IDIM, MPI_UNSIGNED_CHAR, 0, 2, MPI_COMM_WORLD);

      MPI_Send(&gameday, IDIM*JDIM, MPI_UNSIGNED_CHAR, 0, 5, MPI_COMM_WORLD);
      
      }

      else {
      
      //cout << "They got me, gal!" << endl;
      int results = 0;
      MPI_Send(&results, 1, MPI_INT, 0, 3, MPI_COMM_WORLD);

      }


       }  

     }
  
}  



int main(int argc, char *argv[]) {
  // This is a utility routine that installs an alarm to kill off this
  // process if it runs to long.  This will prevent jobs from hanging
  // on the queue keeping others from getting their work done.
  chopsigs_() ;
  
  // All MPI programs must call this function
  MPI_Init(&argc,&argv) ;

  
  int myId ;
  int numProcessors ;

  /* Get the number of processors and my processor identification */
  MPI_Comm_size(MPI_COMM_WORLD,&numProcessors) ;
  MPI_Comm_rank(MPI_COMM_WORLD,&myId) ;

  if(myId == 0) {
    // Processor 0 runs the server code
    get_timer() ;// zero the timer
    Server(argc,argv) ;
    // Measure the running time of the server
    
    
    
    cout << "execution time = " << get_timer() << " seconds." << endl ;
  } else {
    
    
    // all other processors run the client code.
    Client(argc, argv);
    //Client(myId)
  }

  // All MPI programs must call this before exiting
  MPI_Finalize() ;
}
