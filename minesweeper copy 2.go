/*
 Assignment: Final Project: Minesweeper.go
#  	    Authors: Kristen Gladfelter (kgladfelter@email.arizona.edu)
#     			 Aaron Posey (acposey@email.arizona.edu)
# 	     Course: CSC 372
#    Instructor: L. McCann
#      	     TA: Tito Ferra
# 	   Due Date: December 9, 2019
#   Description: A simple command line minesweeper game written in go
#				 To win, need to reveal all space without bombs. Basic
#				 Board of 9x9 with 10 bombs 
#
#      Language: Go
#  Ex. Packages: None
#  Deficiencies: None
*/


package main
import (
	"fmt"
	"math/rand"
	"time"
	)

//Struct that represents an individual square
type Square struct{
	bomb bool
	show_square bool
	number int 
	mark_bomb bool
}

//Board size constant. Change this, change the shape of the board and number of bombs
const board_size = 9
const num_bombs = 10


/*
Print_board function. Will print a minesweeper board to the command line.
PreCondition: None
PostCondition: None
*/
func print_board(board[board_size][board_size] Square){
	fmt.Printf("\n   ")
	for i:=0; i < board_size; i ++{
		fmt.Printf(" %d ", i)
	}
	fmt.Printf("\n  ")
	for i:=0; i < board_size; i ++{
		fmt.Printf("___")
	}
	fmt.Printf("\n")
	for i := 0; i < board_size; i ++{
		fmt.Printf("%d| ", i)
		for j := 0; j < board_size; j++{
			if board[i][j].show_square == false && board[i][j].mark_bomb == false{
				fmt.Printf(" - ")
			} else if board[i][j].show_square == true && board[i][j].mark_bomb == false{
				fmt.Printf(" %d ", board[i][j].number)
			}
			if board[i][j].mark_bomb == true{
				fmt.Printf(" * ")
			}
		}
		fmt.Printf("\n")
	}
}


/*
Test_print_board. Will print a game board to command line with all 
spaces revealed for testing
PreCondition: None
PostCondition: None
*/
func test_print_board(board[board_size][board_size] Square){
	fmt.Printf("\n\n   ")
	for i:=0; i < board_size; i ++{
		fmt.Printf(" %d ", i)
	}
	fmt.Printf("\n  ")
	for i:=0; i < board_size; i ++{
		fmt.Printf("___")
	}
	fmt.Printf("\n")
	for i := 0; i < board_size; i ++{
		fmt.Printf("%d| ", i)
		for j := 0; j < board_size; j++{
			if board[i][j].show_square == false && board[i][j].bomb == false{
				fmt.Printf(" %d ", board[i][j].number)
			} else if board[i][j].show_square == true{
				fmt.Printf(" %d ", board[i][j].number)
			}
			if board[i][j].bomb == true{
				fmt.Printf(" * ")
			}
		}
		fmt.Printf("\n")
	}
}

/*Place_bombs.Seeds the random number generator and places ten bombs
On the board. It will loop if a bomb is already placed on the board 
until it finds an empty space.
PreCondition: None
PostCondition: Bombs are placed on board
*/
func place_bombs(board[board_size][board_size] Square) [board_size][board_size]Square{
	
	rand.Seed(time.Now().UnixNano())
	r := rand.Intn(board_size)
	c := rand.Intn(board_size)
	for i := 0; i < num_bombs; i ++{
		for (board[r][c].bomb == true){
			r = rand.Intn(board_size)
			c = rand.Intn(board_size)
		}
		board[r][c].bomb = true
	}
	return board
}


/*
Test_valid function. Will check to make sure i and j are in range of
the board size when placing numbers for adjacent bombs in each square
PreCondition: None
PostCondtion: None
*/
func test_valid(i int, j int) bool{
	return (i >= 0 && i < board_size && j >= 0 && j < board_size) 
}


/*
Create_board. Initiates a game board of the basic minesweeper game by
setting all values to false or zero.
PreCondition: None
PostCondition: Game board is initialized.
*/
func create_board(board[board_size][board_size] Square) [board_size][board_size]Square{
	for i := 0; i < board_size; i ++{
		for j := 0; j < board_size; j++{
			board[i][j].show_square = false
			board[i][j].bomb = false
			board[i][j].number = 0
			board[i][j].mark_bomb = false
		}
	}
	return board
}	

/*
Get_numbers. Will itterate through the game board and check for adjacent 
bombs. Will call test_valid to prevent array from going out of bounds. 
PreCondition: None
PostCondition: Numbers are set to adjacent bomb count for each square
*/
func get_numbers(board[board_size][board_size] Square) [board_size][board_size]Square{
	for i := 0; i < board_size; i ++{
		for j := 0; j < board_size; j ++{
			counter := 0

			if board[i][j].bomb == false{
				if (test_valid(i-1,j-1) == true && board[i-1][j-1].bomb == true){
					counter += 1
				}
				if (test_valid(i, j-1) == true && board[i][j-1].bomb == true){
					counter += 1
				}
				if (test_valid(i+1, j-1) == true && board[i+1][j-1].bomb == true){
					counter += 1
				}
				if (test_valid(i-1, j) == true && board[i-1][j].bomb == true){
					counter += 1
				}
				if (test_valid(i-1, j+1) == true && board[i-1][j+1].bomb == true){
					counter += 1
				}
				if (test_valid(i, j+1) == true && board[i][j+1].bomb == true){
					counter += 1
				}
				if (test_valid(i+1, j) == true && board[i+1][j].bomb == true){
					counter += 1
				}
				if (test_valid(i+1, j+1) == true && board[i+1][j+1].bomb == true){
					counter += 1
				}
				board[i][j].number = counter
			}

		}
	}

	return board
}


/*
Boom. Print statement for when you lose
PreCondition: None
PostCondition: None
*/
func boom(){
	fmt.Printf("BBBBBBBBBBBBBBBBB                                                               !!!   !!!   !!! \n")
    fmt.Printf("B::::::::::::::::B                                                             !!:!! !!:!! !!:!!\n")
    fmt.Printf("B::::::BBBBBB:::::B                                                            !:::! !:::! !:::!\n")
    fmt.Printf("BB:::::B     B:::::B                                                           !:::! !:::! !:::!\n")
    fmt.Printf("  B::::B     B:::::B   ooooooooooo      ooooooooooo      mmmmmmm    mmmmmmm    !:::! !:::! !:::!\n")
    fmt.Printf("  B::::B     B:::::B oo:::::::::::oo  oo:::::::::::oo  mm:::::::m  m:::::::mm  !:::! !:::! !:::!\n")
    fmt.Printf("  B::::BBBBBB:::::B o:::::::::::::::oo:::::::::::::::om::::::::::mm::::::::::m !:::! !:::! !:::!\n")
    fmt.Printf("  B:::::::::::::BB  o:::::ooooo:::::oo:::::ooooo:::::om::::::::::::::::::::::m !:::! !:::! !:::!\n")
    fmt.Printf("  B::::BBBBBB:::::B o::::o     o::::oo::::o     o::::om:::::mmm::::::mmm:::::m !:::! !:::! !:::!\n")
    fmt.Printf("  B::::B     B:::::Bo::::o     o::::oo::::o     o::::om::::m   m::::m   m::::m !:::! !:::! !:::!\n")
    fmt.Printf("  B::::B     B:::::Bo::::o     o::::oo::::o     o::::om::::m   m::::m   m::::m !!:!! !!:!! !!:!!\n")
    fmt.Printf("  B::::B     B:::::Bo::::o     o::::oo::::o     o::::om::::m   m::::m   m::::m  !!!   !!!   !!! \n")
    fmt.Printf("BB:::::BBBBBB::::::Bo:::::ooooo:::::oo:::::ooooo:::::om::::m   m::::m   m::::m                  \n")
    fmt.Printf("B:::::::::::::::::B o:::::::::::::::oo:::::::::::::::om::::m   m::::m   m::::m  !!!   !!!   !!! \n")
    fmt.Printf("B::::::::::::::::B   oo:::::::::::oo  oo:::::::::::oo m::::m   m::::m   m::::m !!:!! !!:!! !!:!!\n")
    fmt.Printf("BBBBBBBBBBBBBBBBB      ooooooooooo      ooooooooooo   mmmmmm   mmmmmm   mmmmmm  !!!   !!!   !!! \n")
}


/*
TODO
*/
func clear_board(board[board_size][board_size] Square, row int, col int) [board_size][board_size]Square{

	return board
}


/*
Main function. Will call to initialize board and ask user for input in 
order to play minesweeper. Will keep count of all spaces cleared and
check each square that user entered to see if they hit a bomb or not. 
Will determine if you win or lose.
PreCondition: None
PostCondition: None
*/
func main(){

	fmt.Printf("\nBegin Minesweeper!\n")

	//Create a new game board to work with	
	var board[board_size][board_size] Square
	var counter_space int = 0
	
	//Initialize the board
	board = create_board(board)
	board = place_bombs(board)
	board = get_numbers(board)

	for{
		//Print board each time
		print_board(board)

		//Test board so you don't go crazy, has all the mines and numbers 
		//That can be printed. Comment out for a real game
		test_print_board(board)

		//variables to hold user input
		var row int
		var col int
		var is_bomb rune

		//Get values from user
		fmt.Printf("\nEnter row: ")
		fmt.Scanf("%d", &row)

		fmt.Printf("Enter column: ")
		fmt.Scanf("%d", &col)

		fmt.Printf("Mark as bomb? (Y/N): ")
		fmt.Scanf("%c\r", &is_bomb)

		//Checks to make sure all input is valid
		if (row < 0 || row > board_size || col < 0 || col > board_size ||(int(is_bomb) != 89 && int(is_bomb) != 78)) {
			fmt.Printf("Invalid Entry. Try again. \n")

		} else if board[row][col].bomb == true && int(is_bomb) == 78{
			//If you hit a bomb and dont mark it as a bomb, you lose
			boom()
			fmt.Printf("Game Over!\n")
			break

		} else if board[row][col].bomb == false{
			//If you want a square to be uncovered and its not a bomb, will reveal number
			//of possible neighboring bombs and increment the counter_space
			board[row][col].show_square = true
			counter_space += 1

			///////TODO
			//Should recurse through list if number is zero and reveal square
			if board[row][col].number == 0{
				board = clear_board(board, row, col)
			}
		} 


		//If you want to mark a certain position as a bomb, it will print *
		//at that spot
		if int(is_bomb) == 89{
			board[row][col].mark_bomb = true
		}

		//If all spaces have been cleared and no bombs were hit, you win
		if counter_space == board_size * board_size - num_bombs{
			print_board(board)
			fmt.Printf("You win!\n")
			break
		}

	}
	
}



