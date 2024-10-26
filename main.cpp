#include <iostream>
#include <iostream>
#include <fstream>
#include <string>
#include <random>
#include <ctime>

using namespace std;

#define NUM_PLAYERS 2
#define NUM_PIECES 98
#define BOARD_SIZE 15
#define NUM_PIECES_PLAYER 7
#define FILE_PATH "../files/"

struct Piece {
    string letter;
    int value = 0;
};

struct Bag {
    Piece pieces[NUM_PIECES];
    int numPieces = 0;
};

struct Player {
    string name;
    int score = 0;
    Piece pieces[NUM_PIECES_PLAYER];
    int numPieces = 0;
};

struct Board {
    Piece pieces[BOARD_SIZE][BOARD_SIZE];
};

struct Game {
    Bag bag;
    Player players[NUM_PLAYERS];
    Board board;
    int turn = 0;
};

// Game initialization
Game initializeGame();
bool isFirstTime();
Bag initializeBag();
Board initializeBoard();
void initializePlayers(Player players[]);
void changePlayersNames();
Bag shuffleBag(Bag bag);
Game assignPieces(Game game);

// Game flux
int play();
bool gameIsOver(Game game);
Game playTurn(Game game);
Game askForWord(Game game);
bool previewWord(Game game, Piece word[], int wordTam, int col, int row, char dir);
Game putWord(Game game, string word, int col, int row, char dir);

// Game rules
bool validWord(string word);
bool validPosition(Game game, Piece word[], int wordTam, int col, int row, char dir);
bool passCenter(int wordTam, int col, int row, char dir);
bool overlapDifferentLetter(Game game, Piece word[], int wordTam, int col, int row, char dir);

// Utils
int wordToPieces(string word, Piece pieces[]);
Board copyBoard(Board original);

// Menu
void Scrabble();
int menu();

//Print game
void printBag(Bag bag);
void printBoard(Board board);
void printCompactBoard(Board board);
void printPlayerPieces(Player player);
void printPlayersScores(Player players[]);
void printGameStatus(Game game);

int main() {
    setlocale(LC_ALL, "es_ES.UTF-8");
    Scrabble();
}

/***********************
 -  GAME INITIALIZATION  -
 ***********************/

Game initializeGame() {
    Game game;
    game.bag = initializeBag();
    initializePlayers(game.players);
    game.board = initializeBoard();
    game.turn = 0;
    game.bag = shuffleBag(game.bag);
    game = assignPieces(game);
    return game;
}

bool isFirstTime() {
    fstream file(string(FILE_PATH)+"players.txt");
    bool firstTime = !file.is_open();
    if(file.is_open()) {
        string name;
        getline(file, name);
        firstTime = firstTime || name.empty();
    }
    file.close();
    return firstTime;
}

Bag initializeBag() {
    Bag bag;
    bag.numPieces = 0;

    fstream file(string(FILE_PATH)+"pieces.txt", ios_base::in);
    if(!file.is_open()) {
        cout << "Error al abrir el archivo." << endl;
        return bag;
    }

    int value; // Value for letters
    string letter; // Letter
    int quantity; // Quantity of the letter
    while(!file.eof()) {
        // Get value for letters
        file >> value;
        // Get letters
        file >> letter;
        while(letter != "---") {
            file >> quantity;
            for(int i = 0; i < quantity ; i++) {
                Piece piece;
                piece.letter = letter;
                piece.value = value;
                bag.pieces[bag.numPieces] = piece;
                bag.numPieces++;
            }
            file >> letter;
        }
    }
    file.close();
    return bag;
}

Board initializeBoard() {
    Board board;
    Piece piece;
    piece.value = 0;
    piece.letter = "";
    for(int i = 0; i < BOARD_SIZE; i++) {
        for(int j = 0; j < BOARD_SIZE; j++) {
            board.pieces[i][j] = piece;
        }
    }
    board.pieces[7][7].letter = "*";
    return board;
}

void initializePlayers(Player players[]) {
    if(isFirstTime()) {
        cout << "Ups..." << endl;
        cout << "No se han ingresado los nombres de los jugadores." << endl;
        cout << "Por favor, ingrese los nombres de los jugadores." << endl;
        changePlayersNames();
    }
    string name;
    ifstream file(string(FILE_PATH)+"players.txt");
    for(int i = 0; i < NUM_PLAYERS; i++) {
        getline(file, name);
        players[i].name = name;
        players[i].score = 0;
        players[i].numPieces = 0;
    }
    file.close();
}

void changePlayersNames() {
    string name;
    ofstream file(string(FILE_PATH)+"players.txt");
    for(int i = 0; i < NUM_PLAYERS; i++) {
        cout << "Ingrese el nombre del jugador " << i+1 << ": " << endl << ">";
        getline(cin, name);
        file << name << endl;
    }
    file.close();
}

Bag shuffleBag(Bag bag) {
    srand(time(NULL));
    for(int i = 0; i < bag.numPieces; i++) {
        int random = rand() % bag.numPieces;
        Piece temp = bag.pieces[i];
        bag.pieces[i] = bag.pieces[random];
        bag.pieces[random] = temp;
    }
    return bag;
}

Game assignPieces(Game game) {
    for(int i = 0 ; i < NUM_PLAYERS; i++) {
        game.players[i].numPieces = 0;
        for(int j = 0; j < NUM_PIECES_PLAYER; j++) {
            game.players[i].pieces[j] = game.bag.pieces[game.bag.numPieces - 1];
            game.bag.numPieces--;
            game.players[i].numPieces++;
        }
    }
    return game;
}

/**************
 - GAME FLUX -
 **************/

int play() {
    Game game = initializeGame();
    while(!gameIsOver(game)) {
        printGameStatus(game);
        game = playTurn(game);
        cout << "Presione enter para continuar" << endl;
        getchar();
        system("cls");
        game.turn++;
    }
    printGameStatus(game);
    return 0;
}

bool gameIsOver(Game game) {
    return game.bag.numPieces == 0;
}

Game playTurn(Game game) {
    int playerTurn = game.turn%NUM_PLAYERS;
    printPlayerPieces(game.players[playerTurn]);

    // Ask for word
    game = askForWord(game);
    return game;
}

Game askForWord(Game game) {
    string word;
    int row;
    char dir, col;
    int piecesTam;
    Piece pieces[BOARD_SIZE];
    do {
        do {
            do {
                cout << "Ingrese la palabra a jugar: ";
                getline(cin, word);
                // word to lower
                for(int i = 0; i < word.length(); i++) {
                    word[i] = (char) tolower(word[i]);
                }
            } while(!validWord(word));

            cout << "Ingrese la posicion de la palabra (fila columna direccion): ";
            cout << "Posicion de la palabra (LETRA NUMERO ej. A 11): " << endl << ">";
            cin >> col >> row;
            col = toupper(col) - 'A';
            row--;
            cout << "Direccion de la palabra: " << endl;
            cout << " - H: Horizontal" << endl;
            cout << " - V: Vertical" << endl;
            cin >> dir;
            piecesTam = wordToPieces(word, pieces);
        } while(!validPosition(game, pieces, piecesTam, col, row, dir));
    } while(!previewWord(game, pieces, piecesTam, col, row, dir));

    putWord(game, word, col, row, dir);
    return game;
}

bool previewWord(Game game, Piece word[], int wordTam, int col, int row, char dir) {
    Board previewBoard = copyBoard(game.board);
    for (int i = 0; i < wordTam; i++) {
        if (dir == 'H') {
            previewBoard.pieces[row][col + i] = word[i];
        } else {
            previewBoard.pieces[row + i][col] = word[i];
        }
    }
    printCompactBoard(previewBoard);
    cout << "Desea jugar esta palabra? (S/N): ";
    char option;
    cin >> option;
    return option == 'S' || option == 's';
}

Game putWord(Game game, string word, int col, int row, char dir){
    // Pieces of the word
    Piece pieces[word.length()];
    int piecesTam = wordToPieces(word, pieces);

    // Letters of the word that are in the board
    string interceptedLetters[word.length()];
    int interceptedTam = 0;

    for(int i = 0 ; i < piecesTam ; i++) {
        if(game.board.pieces[row][col].letter != "") {
            interceptedLetters[interceptedTam] = game.board.pieces[row][col].letter;
            interceptedTam++;
        }
        if(dir == 'H') {
            col++;
        }
        else {
            row++;
        }
    }



    return game;
}


/**************
 - GAME RULES -
 **************/

bool validWord(string word) {
    // File extracted from https://github.com/kamilmielnik/scrabble-dictionaries
    ifstream file(string(FILE_PATH)+"fise-2.txt");
    bool exists = false;
    string line;
    while(getline(file, line)) {
        if(line == word) {
            exists = true;
            break;
        }
    }
    file.close();
    if(!exists) {
        cout << "La palabra ingresada no es valida.\n" << endl;
    }
    else {
        cout << "La palabra ingresada es valida!\n" << endl;
    }
    return exists;
}

bool validPosition(Game game, Piece word[], int wordTam, int col, int row, char dir) {
    bool isInBoard = (col >= 0 && col <= BOARD_SIZE) && (row >= 0 && row <= BOARD_SIZE);
    bool validDirection = dir == 'H' || dir == 'V';
    bool noOverFlow = (dir == 'H' && col + wordTam <= BOARD_SIZE) || (dir == 'V' && row + wordTam <= BOARD_SIZE);
    bool overlapDifferent = overlapDifferentLetter(game, word, wordTam, col, row, dir);
    bool isInCenterFirstTurn = game.turn == 0 && passCenter(wordTam, col, row, dir);


    // Show personalized error messages
    if(!isInBoard) {
        cout << "\tLa posicion ingresada no es valida." << endl;
    }
    if(!validDirection) {
        cout << "\tLa direccion ingresada no es valida." << endl;
    }
    if(!noOverFlow) {
        cout << "\tLa palabra no cabe en la posicion ingresada." << endl;
    }
    if(!isInCenterFirstTurn) {
        cout << "\tLa primera palabra debe pasar por el centro del tablero." << endl;
    }
    if(overlapDifferent) {
        cout << "\tLa palabra se superpone con una letra diferente." << endl;
    }

    return isInBoard && validDirection && noOverFlow && isInCenterFirstTurn && !overlapDifferent;
}

bool passCenter(int wordTam, int col, int row, char dir) {
    for(int i = 0 ; i < wordTam; i++) {
        if(col == BOARD_SIZE/2 && row == BOARD_SIZE/2) {
            return true;
        }
        if(dir == 'H') {
            col++;
        }
        else {
            row++;
        }
    }
    return false;
}

bool overlapDifferentLetter(Game game, Piece word[], int wordTam, int col, int row, char dir) {
    for(int i = 0; i < wordTam; i++) {
        string boardLetter;
        if(dir == 'H') {
            boardLetter = game.board.pieces[row][col + i].letter;
        }
        else {
            boardLetter = game.board.pieces[row + i][col].letter;
        }
        if((!boardLetter.empty() && boardLetter != "*") && (boardLetter != word[i].letter)) {
            return true;
        }
    }
    return false;
}


/**********
 - UTILS  -
 **********/

int wordToPieces(string word, Piece pieces[]) {
    for(int i = 0 ; i < word.length() ; i++) {
        word[i] = toupper(word[i]);
    }
    int tam = 0;
    for (int i = 0; i < word.length(); i++) {
        Piece piece;
        if (i < word.length() - 1) {
            piece.letter = word.substr(i, 2);
            if (piece.letter == "LL" || piece.letter == "CH" || piece.letter == "RR") {
                i++;
            } else {
                piece.letter = word.substr(i, 1);
            }
        } else {
            piece.letter = word[i];
        }
        pieces[tam] = piece;
        tam++;
    }
    return tam;
}

Board copyBoard(Board original) {
    Board newBoard;
    for (int i = 0; i < BOARD_SIZE; i++) {
        for (int j = 0; j < BOARD_SIZE; j++) {
            newBoard.pieces[i][j] = original.pieces[i][j];  // Copia profunda de cada pieza
        }
    }
    return newBoard;
}


/**********
 - MENU  -
 **********/

void Scrabble() {
    int option = menu();
    while(option != 4) {
        switch(option) {
            case 1:
                changePlayersNames();
                break;
            case 2:
                play();
                break;
            case 3:
                break;
            default:
                cout << "Opcion invalida." << endl;
        }
        option = menu();
    }
}

int menu() {
    if (isFirstTime()) {
        cout << "Bienvenido a SCRABBLE! Por favor, ingrese los nombres de los jugadores." << endl;
        changePlayersNames();
        system("cls");
    }

    cout << "------------------------------------" << endl;
    cout << "-   Bienvenido a SCRABBLE!        -" << endl;
    cout << "------------------------------------\n" << endl;
    string options[] = {
        "1. Cambiar nombre de jugadores",
        "2. Iniciar juego",
        "3. Mostrar registro de resultados",
        "4. Salir"
    };
    // Show menu
    cout << "---------- MENU PRINCIPAL ----------" << endl;
    for (int i = 0; i < 4; i++) {
        cout << "- " << options[i] << endl;
    }
    cout << "------------------------------------" << endl;
    cout << "Por favor, selecciona una opcion (1-4): ";

    // Read user option
    int option;
    cin >> option;
    getchar();

    return option;
}


/**********************
 -  PRINT FUNCTIONS  -
 **********************/

void printBag(Bag bag) {
    cout << "Bag: " << endl;
    for(int i = 0; i < bag.numPieces; i++) {
        cout << bag.pieces[i].letter << " ";
    }
    cout << endl;
}

void printBoard(Board board) {
    // Font should be mono-spaced for the board to be displayed correctly
    cout << "    ";
    // Print letters at the top
    for (int j = 0; j < BOARD_SIZE; j++) {
        cout << " " << (char)('A' + j) << "  ";
    }
    cout << endl;

    for (int i = 0; i < BOARD_SIZE; i++) {
        // Print row separator
        cout << "   ";
        for (int j = 0; j < BOARD_SIZE; j++) {
            cout << "+---";
        }
        cout << "+" << endl;

        // Print row label
        cout << " " << (i + 1);
        if (i + 1 < 10){
            cout << " "; // Align single digit numbers
        }

        // Print row contents
        for (int j = 0; j < BOARD_SIZE; j++) {
            cout << "| " << board.pieces[i][j].letter << " ";
            cout << board.pieces[i][j].letter;
            if(board.pieces[i][j].letter == "") {
                cout << ".";
            }
            if(board.pieces[i][j].letter.length() == 1) {
                cout << " ";
            }
            cout << " ";
        }
        cout << "|" << endl;
    }

    // Print bottom row separator
    cout << "   ";
    for (int j = 0; j < BOARD_SIZE; j++) {
        cout << "+---";
    }
    cout << "+" << endl;
}

void printCompactBoard(Board board) {
    cout << "   ";
    // Print header with letters
    for (int j = 0; j < BOARD_SIZE; j++) {
        cout << (char)('A' + j) << "  ";
    }
    cout << endl;

    // Print board contents
    for (int i = 0; i < BOARD_SIZE; i++) {
        // Print the row number
        if (i + 1 < 10) cout << " ";
        cout << i + 1 << " ";

        // Imprimir el contenido de la fila
        for (int j = 0; j < BOARD_SIZE; j++) {
            if (board.pieces[i][j].letter == "") {
                cout << ".  ";
            } else {
                cout << board.pieces[i][j].letter << " ";  // Imprimir letra
            }
            if(board.pieces[i][j].letter.length() == 1) {
                cout << " ";
            }
        }
        cout << endl;
    }
}

void printPlayerPieces(Player player) {
    cout << "------------------------------------------------------------------" << endl;
    cout << "Piezas de " << player.name << ": ";
    for (int i = 0; i < player.numPieces; i++) {
        cout << player.pieces[i].letter << "(" << player.pieces[i].value << ") ";
    }
    cout << endl;
    cout << "------------------------------------------------------------------" << endl << endl;
}

void printPlayersScores(Player players[]) {
    for (int i = 0; i < NUM_PLAYERS; i++) {
        cout << players[i].name << ": " << players[i].score << endl;
    }
}

void printGameStatus(Game game) {
    cout << "----------------------" << endl;
    cout << " Jugando " << game.players[game.turn%NUM_PLAYERS].name << endl;
    cout << "----------------------" << endl;

    // Print board status
    printCompactBoard(game.board);
    // printBoard(game.board);

    // Print player's scores
    cout << "-----------------" << endl;
    cout << "Puntajes:" << endl;
    printPlayersScores(game.players);
    // Print remaining pieces
    cout << "\nPiezas restantes: " << game.bag.numPieces << endl;
}