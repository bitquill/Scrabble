#include <iostream>
#include <iostream>
#include <fstream>
#include <string>
#include <random>
#include <ctime>

using namespace std;

#define NUM_PLAYERS 4
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
int wordPoints(Game game, int start, int col, int row, char dir);
int calculateAdyacentPoints(Game game, Piece pivots[], int wordTam, int col, int row, char dir);
int getAdyacentWord(Game game, Piece adyacent[], int col, int row, char dir);
Game givePlayerPieces(Game game);

// Game rules
bool validWord(string word);
bool validMove(Game game, Piece word[], int wordTam, int col, int row, char dir);
bool overFlow(int wordTam, char dir, int col, int row);
bool passCenter(int wordTam, int col, int row, char dir);
bool overlapDifferentLetter(Game game, Piece word[], int wordTam, int col, int row, char dir);
bool playerHasPieces(Game game, Piece word[], int wordTam, int col, int row, char dir);
bool validAdyacentWords(Game game, Piece word[], int wordTam, int col, int row, char dir);
bool validAdyacent(Game game, string letter, int col, int row, char dir);

// Utils
int wordToPieces(string word, Piece pieces[]);
Board copyBoard(Board original);
int pieceInArray(Piece pieces[], int piecesTam, Piece piece);
int removePiece(Piece pieces[], int piecesTam, Piece piece);

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
    // play();
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
    // Give player pieces if possible
    game = givePlayerPieces(game);
    // Next turn
    game.turn++;
    return game;
}

Game askForWord(Game game) {
    string word;
    int row;
    char dir, col;
    int piecesTam;
    Piece pieces[BOARD_SIZE];
    bool valid = false;
    do {
        do {
            do {
                cout << "Ingrese la palabra a jugar (presione enter para saltar su turno): ";
                getline(cin, word);
                if(word == "") {
                    return game;
                }
            } while(!validWord(word));

            cout << "Ingrese la posicion de la palabra (LETRA NUMERO DIRECCION ej. A 11 V): " << endl;
            cout << " [H] Horizontal" << endl;
            cout << " [V] Vertical" << endl;
            cout << ">";
            cin >> col >> row >> dir;
            getchar();
            col = toupper(col) - 'A';
            row--;
            piecesTam = wordToPieces(word, pieces);
        } while(!validMove(game, pieces, piecesTam, col, row, dir));
    } while(!previewWord(game, pieces, piecesTam, col, row, dir));

    game = putWord(game, word, col, row, dir);
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
    getchar();
    return option == 'S' || option == 's';
}

Game putWord(Game game, string word, int col, int row, char dir){
    int initialCol = col;
    int initialRow = row;

    // Pieces of the word
    Piece pieces[word.length()];
    int piecesTam = wordToPieces(word, pieces);

    // Player pieces
    int playerTurn = game.turn%NUM_PLAYERS;
    Player player = game.players[playerTurn];

    // Pivots
    Piece pivots[piecesTam];

    for(int i = 0 ; i < piecesTam ; i++) {
        Piece boardPiece = game.board.pieces[row][col];
        if(boardPiece.letter == "" || boardPiece.letter == "*") { // Check if piece in board is empty
            // Get player piece
            int playerPiecePos = pieceInArray(player.pieces, player.numPieces, pieces[i]);
            boardPiece = player.pieces[playerPiecePos];
            // Remove piece from user
            player.numPieces = removePiece(player.pieces, player.numPieces, boardPiece);
            pivots[i].letter = "";
        }
        else {
            pivots[i] = boardPiece;
        }
        // Put piece in board
        game.board.pieces[row][col] = boardPiece;

        if(dir == 'H') {
            col++;
        }
        else {
            row++;
        }
    }
    int start;
    if(dir == 'H') {
        start = initialCol;
    }
    else {
        start = initialRow;
    }
    int points = wordPoints(game, start, initialCol, initialRow, dir);
    int adyacentPoints = calculateAdyacentPoints(game, pivots, piecesTam, initialCol, initialRow, dir);
    cout << "------------------------------------" << endl;
    cout << "Puntos ganados con la palabra: " << points << endl;
    cout << "Puntos con palabras adyacentes: " << adyacentPoints << endl;
    player.score += points + adyacentPoints;
    cout << "Puntos totales: " << player.score << endl;
    cout << "------------------------------------" << endl;

    game.players[playerTurn] = player;
    return game;
}

int wordPoints(Game game, int start, int col, int row, char dir){
    int points = 0;
    if(dir == 'H'){
        col = start;
    }
    else{
        row = start;
    }
    while(col < BOARD_SIZE && row < BOARD_SIZE && game.board.pieces[row][col].letter != ""){
        cout << "Letra: " << game.board.pieces[row][col].letter << " Valor: " << game.board.pieces[row][col].value << endl;
        points += game.board.pieces[row][col].value;
        if(dir == 'H'){
            col++;
        }
        else{
            row++;
        }
    }
    return points;
}

int calculateAdyacentPoints(Game game, Piece pivots[], int wordTam, int col, int row, char dir){
    int points = 0;
    for(int i = 0 ; i < wordTam ; i++){
        Piece adyacent[BOARD_SIZE];
        int wordStart = 0;
        if(pivots[i].letter == ""){
            if(dir == 'V'){
                if((col > 0 && game.board.pieces[row][col-1].letter != "") || (col < BOARD_SIZE-1 && game.board.pieces[row][col+1].letter != "")){
                    wordStart = getAdyacentWord(game, adyacent, col, row, 'H');
                    points += wordPoints(game, wordStart, col, row, 'H');
                }
            }
            else{
                if((row > 0 && game.board.pieces[row-1][col].letter != "") || (row < BOARD_SIZE-1 && game.board.pieces[row+1][col].letter != "")){
                    wordStart = getAdyacentWord(game, adyacent, col, row, 'V');
                    points += wordPoints(game, wordStart, col, row, 'V');
                }
            }
        }
        if(dir == 'H'){
            col++;
        }
        else{
            row++;
        }
    }
    return points;
}

int getAdyacentWord(Game game, Piece adyacent[], int col, int row, char dir){
    int inf, sup, start;
    int adyacentTam = 0;
    if(dir == 'H'){
        inf = col;
        sup = col;
        while(inf > 0 && game.board.pieces[row][inf-1].letter != ""){
            inf--;
        }
        while(sup < BOARD_SIZE-1 && game.board.pieces[row][sup+1].letter != ""){
            sup++;
        }
        start = inf;
        while(inf <= sup){
            adyacent[adyacentTam] = game.board.pieces[row][inf];
            adyacentTam++;
            inf++;
        }
    }
    else {
        inf = row;
        sup = row;
        while(inf > 0 && game.board.pieces[inf-1][col].letter != ""){
            inf--;
        }
        while(sup < BOARD_SIZE-1 && game.board.pieces[sup+1][col].letter != ""){
            sup++;
        }
        start = inf;
        while(inf <= sup){
            adyacent[adyacentTam] = game.board.pieces[inf][col];
            adyacentTam++;
            inf++;
        }
    }
    cout << "Adyacente: ";
    for(int i = 0 ; i < adyacentTam ; i++){
        cout << adyacent[i].letter;
    }
    cout << endl;
    return start;
}

Game givePlayerPieces(Game game){
    int playerTurn = game.turn%NUM_PLAYERS;
    Player player = game.players[playerTurn];
    for(int i = player.numPieces ; i < NUM_PIECES_PLAYER && game.bag.numPieces > 0 ; i++){
        player.pieces[i] = game.bag.pieces[game.bag.numPieces - 1];
        game.bag.numPieces--;
        player.numPieces++;
    }
    game.players[playerTurn] = player;
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
        for(int i = 0; i < word.length(); i++) {
            word[i] = (char) tolower(word[i]);
        }
        if(line == word) {
            exists = true;
            break;
        }
    }
    file.close();
    if(!exists) {
        cout << "La palabra " << word << " no es valida.\n" << endl;
    }
    else {
        cout << "La palabra " << word << " es valida!\n" << endl;
    }
    return exists;
}

bool validMove(Game game, Piece word[], int wordTam, int col, int row, char dir) {
    bool isInBoard = (col >= 0 && col <= BOARD_SIZE) && (row >= 0 && row <= BOARD_SIZE);
    bool validDirection = dir == 'H' || dir == 'V';
    // Show personalized error messages
    if(!isInBoard) {
        cout << "\tLa posicion ingresada no es valida." << endl;
    }
    if(!validDirection) {
        cout << "\tLa direccion ingresada no es valida." << endl;
    }

    bool noOverFlow = !overFlow(wordTam, dir, col, row);
    bool noOverlapDifferent = !overlapDifferentLetter(game, word, wordTam, col, row, dir);
    bool isInCenterFirstTurn = game.turn == 0 && passCenter(wordTam, col, row, dir) || game.turn != 0;
    bool hasNeededPieces = playerHasPieces(game, word, wordTam, col, row, dir);
    bool validAdyacent = validAdyacentWords(game, word, wordTam, col, row, dir);

    bool validMove = isInBoard && validDirection && noOverFlow && isInCenterFirstTurn && noOverlapDifferent && hasNeededPieces && validAdyacent;
    if(!validMove){
        cout << "Esta palabra no se puede poner" << endl;
    }
    return validMove;
}

bool overFlow(int wordTam, char dir, int col, int row){
    if((dir == 'H' && col + wordTam <= BOARD_SIZE) || (dir == 'V' && row + wordTam <= BOARD_SIZE)){
        return false;
    }
    else{
        cout << "\tLa palabra no cabe en la posicion ingresada." << endl;
        return true;
    }
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
    cout << "\tLa primera palabra debe pasar por el centro del tablero." << endl;
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
            cout << "\tLa palabra se superpone con una letra diferente." << endl;
            return true;
        }
    }
    return false;
}

bool playerHasPieces(Game game, Piece word[], int wordTam, int col, int row, char dir){
    int playerTurn = game.turn%NUM_PLAYERS;

    int numPieces = game.players[playerTurn].numPieces;
    Piece piecesCopy[numPieces];
    for(int i = 0 ; i < numPieces ; i++){
        piecesCopy[i] = game.players[playerTurn].pieces[i];
    }

    for(int i = 0 ; i < wordTam ; i++){
        if((game.board.pieces[row][col].letter == "" || game.board.pieces[row][col].letter == "*")){
            if(pieceInArray(piecesCopy, numPieces, word[i]) == -1){
                cout << "El jugador no tiene la letra " << word[i].letter << " necesaria para formar la palabra" << endl;
                return false;
            }
            else{
                numPieces = removePiece(piecesCopy, numPieces, word[i]);
            }
        }
        if(dir == 'H'){
            col++;
        }
        else{
            row++;
        }
    }
    return true;
}

bool validAdyacentWords(Game game, Piece word[], int wordTam, int col, int row, char dir){
    for(int i = 0 ; i < wordTam ; i++){
        // If the piece is not the pivot
        if (dir == 'V') {
            if ((col > 0 && game.board.pieces[row][col - 1].letter != "") ||
                (col < BOARD_SIZE - 1 && game.board.pieces[row][col + 1].letter != "")) {
                if (!validAdyacent(game, word[i].letter, col, row, 'H')) {
                    cout << "La palabra no tiene adyacentes validas" << endl;
                    return false;
                }
            }
            row++;
        } else {
            if ((row > 0 && game.board.pieces[row - 1][col].letter != "") ||
                (row < BOARD_SIZE - 1 && game.board.pieces[row + 1][col].letter != "")) {
                if (!validAdyacent(game, word[i].letter, col, row, 'V')) {
                    cout << "La palabra no tiene adyacentes validas" << endl;
                    return false;
                }
            }
            col++;
        }
    }
    return true;
}

bool validAdyacent(Game game, string letter, int col, int row, char dir){
    int inf, sup;
    string adyacent = "";
    if(dir == 'H'){
        inf = col;
        sup = col;
        while(inf > 0 && game.board.pieces[row][inf-1].letter != ""){
            inf--;
        }
        while(sup < BOARD_SIZE-1 && game.board.pieces[row][sup+1].letter != ""){
            sup++;
        }
        while(inf <= sup){
            adyacent += game.board.pieces[row][inf].letter;
            if(inf == col && (game.board.pieces[row][inf].letter == "" || game.board.pieces[row][inf].letter == "*")){
                adyacent += letter;
            }
            inf++;
        }
    }
    else {
        inf = row;
        sup = row;
        while(inf > 0 && game.board.pieces[inf-1][col].letter != ""){
            inf--;
        }
        while(sup < BOARD_SIZE-1 && game.board.pieces[sup+1][col].letter != ""){
            sup++;
        }
        while(inf <= sup){
            adyacent += game.board.pieces[inf][col].letter;
            if(inf == row && (game.board.pieces[inf][col].letter == "" || game.board.pieces[inf][col].letter == "*")){
                adyacent += letter;
            }
            inf++;
        }
    }
    cout << "Adyacente: " << adyacent << endl;
    return validWord(adyacent);
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

int pieceInArray(Piece pieces[], int piecesTam, Piece piece){
    for(int i = 0 ; i < piecesTam ; i++){
        if(pieces[i].letter == piece.letter){
            return i;
        }
    }
    return -1;
}

int removePiece(Piece pieces[], int piecesTam, Piece piece){
    int piecePos = pieceInArray(pieces, piecesTam, piece);
    if(piecePos == -1){
        cout << "El jugador no posee la pieza con la letra " << piece.letter << endl;
        return piecesTam;
    }

    piecesTam--;
    for(int i = piecePos ; i < piecesTam ; i++){
        pieces[i] = pieces[i+1];
    }

    return piecesTam;
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