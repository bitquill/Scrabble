#include <iostream>
#include <iostream>
#include <fstream>
#include <string>
#include <random>
#include <ctime>
#include <iomanip>

using namespace std;

#define NUM_PLAYERS 2
#define NUM_PIECES 98
#define BOARD_SIZE 15
#define NUM_PIECES_PLAYER 7
#define FILE_PATH "../files/"

// Estructuras
// Pieza: representa una pieza con su letra y valor
struct Piece {
    string letter;
    int value = 0;
};

// Bolsa: representa la bolsa con las piezas
struct Bag {
    Piece pieces[NUM_PIECES];
    int numPieces = 0;
};

// Jugador: representa un jugador con su nombre, puntaje y piezas
struct Player {
    string name;
    int score = 0;
    Piece pieces[NUM_PIECES_PLAYER];
    int numPieces = 0;
};

// Tablero: representa el tablero con las piezas
struct Board {
    Piece pieces[BOARD_SIZE][BOARD_SIZE];
};

// Juego: representa el juego con la bolsa, jugadores y tablero
struct Game {
    Bag bag;
    Player players[NUM_PLAYERS];
    Board board;
    int turn = 0;
    int skippedTurns = 0;
};

// Registro: representa el registro de las partidas
struct Registry {
    string name;
    int wins;
    int losses;
};

// Inicialización del juego
Game initializeGame(); // Inicializa el juego
bool isFirstTime(); // Verifica si es la primera vez que se ejecuta el juego
Bag initializeBag(); // Inicializa la bolsa con las piezas a partir de un archivo
Board initializeBoard(); // Inicializa el tablero eliminando todas las piezas y colocando el centro
void initializePlayers(Player players[]); // Inicializa los jugadores con sus nombres y puntajes a partir de un archivo
void changePlayersNames(); // Cambia los nombres de los jugadores
Bag shuffleBag(Bag bag); // Mezcla las piezas de la bolsa
Game assignPieces(Game game); // Le da a cada jugador las piezas iniciales

// Flujo del juego
int play(); // entrada al juego
bool gameIsOver(Game game); // Verifica si el juego ha terminado
Game playTurn(Game game); // Juega un turno
Game askForWord(Game game); // Pide la palabra al jugador
Game skipTurn(Game game); // Salta el turno del jugador si no tiene jugadas posibles y cambia todas sus piezas
bool previewWord(Game game, Piece word[], int wordTam, int col, int row, char dir); // Muestra la palabra que se va a jugar
Game putWord(Game game, string word, int col, int row, char dir); // Pone la palabra en el tablero y calcula los puntos
int wordPoints(Game game, int start, int col, int row, char dir); // Calcula los puntos de la palabra
int calculateAdyacentPoints(Game game, Piece pivots[], int wordTam, int col, int row, char dir); // Calcula los puntos de las palabras adyacentes
int getAdyacentWord(Game game, Piece adyacent[], int col, int row, char dir); // Obtiene la palabra adyacente
Game givePlayerPieces(Game game); // Da las piezas gastadas al jugador si es posible

// Registro de partidas
void saveResults(Registry playersRegistry[]); // Guarda los resultados de la partida en un archivo
void showResults(); // Muestra los resultados de las partidas guardadas
int readRegistry(Registry playersRegistry[]); // Lee los resultados de las partidas guardadas

// Game rules
bool validWord(string word); // Verifica si la palabra es válida
bool validMove(Game game, Piece word[], int wordTam, int col, int row, char dir); // Verifica si la jugada es válida
bool overFlow(int wordTam, char dir, int col, int row); // Verifica si la palabra se sale del tablero
bool passCenter(int wordTam, int col, int row, char dir); // Verifica si la palabra pasa por el centro del tablero
bool overlapDifferentLetter(Game game, Piece word[], int wordTam, int col, int row, char dir); // Verifica si la palabra se superpone con una letra diferente
bool playerHasPieces(Game game, Piece word[], int wordTam, int col, int row, char dir); // Verifica si el jugador tiene las piezas necesarias
bool validAdyacentWords(Game game, Piece word[], int wordTam, int col, int row, char dir); // Verifica si las palabras adyacentes son válidas
bool validAdyacent(Game game, string letter, int col, int row, char dir); // Verifica si la palabra adyacente es válida
bool wordIsConnected(Game game, int wordTam, int col, int row, char dir); // Verifica si la palabra está conectada a otra palabra

// Utils
int wordToPieces(string word, Piece pieces[]); // Convierte la palabra en piezas (teniendo en cuenta las letras especiales RR, LL, CH)
Board copyBoard(Board original); // Copia el tablero en uno nuevo para hacer cambios sin modificar el original
int pieceInArray(Piece pieces[], int piecesTam, Piece piece); // Verifica si una pieza está en un arreglo
int removePiece(Piece pieces[], int piecesTam, Piece piece); // Elimina una pieza de un arreglo

// Menu
void Scrabble(); // Inicia el menu de juego
int menu(); // Muestra el menu y lee la opción del usuario

//Print game
void printBag(Bag bag); // Imprime la bolsa
void printBoard(Board board); // Imprime el tablero
void printCompactBoard(Board board); // Imprime el tablero de forma compacta
void printPlayerPieces(Player player); // Imprime las piezas del jugador
void printPlayersScores(Player players[]); // Imprime los puntajes de los jugadores
void gameHeader(Game game); // Imprime el encabezado del juego
void printGameStatus(Game game); // Imprime el estado del juego

int main() {
    setlocale(LC_ALL, "es_ES.UTF-8");
    Scrabble(); // Inicia el juego
}

/***********************
 -  GAME INITIALIZATION  -
 ***********************/

Game initializeGame() {
    Game game;
    // Inicializar el juego
    game.bag = initializeBag(); // Inicializar la bolsa
    initializePlayers(game.players); // Inicializar los jugadores
    game.board = initializeBoard(); // Inicializar el tablero
    game.turn = 0; // Inicializar el turno
    game.skippedTurns = 0; // Inicializar los turnos saltados
    game.bag = shuffleBag(game.bag); // Mezclar la bolsa antes de asignar las piezas
    game = assignPieces(game); // Asignar las piezas a los jugadores
    return game;
}

bool isFirstTime() {
    fstream file(string(FILE_PATH)+"players.txt");
    bool firstTime = !file.is_open(); // si no se puede abrir el archivo, es la primera vez
    if(file.is_open()) { // si se puede abrir el archivo, verificar si está vacío
        string name;
        getline(file, name);
        firstTime = firstTime || name.empty(); // si el archivo está vacío, es la primera vez
    }
    file.close();
    return firstTime;
}

Bag initializeBag() {
    Bag bag;
    bag.numPieces = 0;
    // Leer piezas del archivo
    fstream file(string(FILE_PATH)+"pieces.txt", ios_base::in);
    if(!file.is_open()) {
        cout << "Error al abrir el archivo." << endl;
        return bag;
    }

    int value; // Valor de la letra
    string letter; // Letra
    int quantity; // cantidad de piezas por letra
    while(!file.eof()) {
        // Obtener el valor de las letras que se van a leer
        file >> value;
        // Leer las letras
        file >> letter;
        while(letter != "---") {
            // Leer la cantidad de piezas de la letra
            file >> quantity;
            for(int i = 0; i < quantity ; i++) { // Agregar las piezas repetidas por letra a la bolsa
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
    // Inicializa el tablero con piezas vacías
    for(int i = 0; i < BOARD_SIZE; i++) {
        for(int j = 0; j < BOARD_SIZE; j++) {
            board.pieces[i][j] = piece;
        }
    }
    // Coloca el centro del tablero
    board.pieces[BOARD_SIZE/2][BOARD_SIZE/2].letter = "*";
    return board;
}

void initializePlayers(Player players[]) {
    // Verificar si es la primera vez que se ejecuta el juego
    if(isFirstTime()) {
        cout << "Ups..." << endl;
        cout << "No se han ingresado los nombres de los jugadores." << endl;
        cout << "Por favor, ingrese los nombres de los jugadores." << endl;
        changePlayersNames();
    }
    // Leer los nombres de los jugadores del archivo
    string name;
    ifstream file(string(FILE_PATH)+"players.txt");
    for(int i = 0; i < NUM_PLAYERS; i++) {
        getline(file, name);
        players[i].name = name;
        players[i].score = 0; // Inicializar el puntaje
        players[i].numPieces = 0; // Inicializar el número de piezas
    }
    file.close();
}

void changePlayersNames() {
    // Cambiar los nombres de los jugadores y los guarda en un archivo
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
    // Mezclar las piezas de la bolsa
    srand(time(NULL));
    // Intercambia cada posición con una posición aleatoria
    for(int i = 0; i < bag.numPieces; i++) {
        int random = rand() % bag.numPieces;
        Piece temp = bag.pieces[i];
        bag.pieces[i] = bag.pieces[random];
        bag.pieces[random] = temp;
    }
    return bag;
}

Game assignPieces(Game game) {
    // Asignar las piezas a los jugadores
    for(int i = 0 ; i < NUM_PLAYERS; i++) {
        game.players[i].numPieces = 0;
        for(int j = 0; j < NUM_PIECES_PLAYER; j++) {
            // toma la última pieza de la bolsa y la asigna al jugador
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
    // Inicializar el juego
    Game game = initializeGame();
    while(!gameIsOver(game)) { // Mientras el juego no haya terminado
        printGameStatus(game); // Imprimir el estado del juego
        game = playTurn(game); // Jugar un turno
        cout << "Presione enter para continuar..." << endl;
        getchar();
    }
    // Cuando el juego ha terminado
    printGameStatus(game); // Imprimir el estado del juego
    // Verifica el ganador
    int maxScore = 0;
    int winner = 0;
    for(int i = 0; i < NUM_PLAYERS; i++) {
        if(game.players[i].score > maxScore) {
            maxScore = game.players[i].score;
            winner = i;
        }
    }
    // Crea el registro de la partida y lo guarda en el archivo
    Registry registry[NUM_PLAYERS];
    for(int i = 0 ; i < NUM_PLAYERS ; i++){
        registry[i].name = game.players[i].name;
        if(i == winner){
            registry[i].wins = 1;
            registry[i].losses = 0;
        }
        else{
            registry[i].wins = 0;
            registry[i].losses = 1;
        }
    }
    saveResults(registry);

    // Imprime el ganador
    cout << "------------------------------------" << endl;
    cout << "El juego ha terminado!" << endl;
    cout << "------------------------------------" << endl;
    cout << "El ganador es " << game.players[winner].name << " con " << game.players[winner].score << " puntos!" << endl;
    cout << "------------------------------------" << endl;
    cout << "Presione enter para continuar..." << endl;
    getchar();

    return 0;
}

bool gameIsOver(Game game) {
    bool skippedTurns = game.skippedTurns == (NUM_PLAYERS * 2); // Si se han saltado todos los turnos
    bool playerHasNoPieces = false; // Si algún jugador no tiene piezas
    // Verifica si algún jugador no tiene piezas
    for(int i = 0; i < NUM_PLAYERS; i++) {
        if(game.players[i].numPieces == 0) {
            playerHasNoPieces = true;
        }
    }
    // Si se han saltado todos los turnos o algún jugador no tiene piezas
    return (playerHasNoPieces && game.bag.numPieces == 0) || skippedTurns;
}

Game playTurn(Game game) {
    int playerTurn = game.turn%NUM_PLAYERS; // Turno del jugador actual
    printPlayerPieces(game.players[playerTurn]); // Imprimir las piezas del jugador
    // Pide la palabra al jugador
    game = askForWord(game);
    // Da las piezas al jugador que uso en el turno
    game = givePlayerPieces(game);
    // Siguiente turno
    game.turn++;
    return game;
}

Game askForWord(Game game) {
    // Pide la palabra al jugador
    string word;
    int row;
    char dir, col;
    int piecesTam;
    Piece pieces[BOARD_SIZE];
    // Turno del jugador actual
    int playerTurn = game.turn%NUM_PLAYERS;
    printPlayerPieces(game.players[playerTurn]);
    do { // Mientras el jugador no acepte poner la palabra
        do { // Mientras la jugada no sea válida
            do { // Mientras la palabra no sea válida
                printGameStatus(game);
                printPlayerPieces(game.players[playerTurn]);
                cout << "Ingrese la palabra a jugar (presione enter para saltar turno y cambiar todas sus fichas): ";
                getline(cin, word);
                if(word == "") { // si no se ingresa palabra, saltar turno
                    cout << "Saltando turno..." << endl;
                    cout << "Presione enter para continuar..." << endl;
                    getchar();
                    printGameStatus(game);
                    game = skipTurn(game);
                    return game;
                }
                cout << endl << "------------------------------------" << endl;
            } while(!validWord(word));

            // Pedir la posición de la palabra
            cout << "Ingrese la posicion de la palabra (LETRA NUMERO DIRECCION ej. A 11 V): " << endl;
            cout << " [H] Horizontal" << endl;
            cout << " [V] Vertical" << endl;
            cout << ">";
            cin >> col >> row >> dir;
            getchar();
            col = toupper(col) - 'A';
            row--;
            // Convertir la palabra en piezas
            piecesTam = wordToPieces(word, pieces);
        } while(!validMove(game, pieces, piecesTam, col, row, dir));
        // reinicia los turnos saltados a 0
        game.skippedTurns = 0;
    } while(!previewWord(game, pieces, piecesTam, col, row, dir));
    // Poner la palabra en el tablero
    game = putWord(game, word, col, row, dir);
    return game;
}

Game skipTurn(Game game) {
    // determina el jugador que salto el turno
    int playerTurn = game.turn%NUM_PLAYERS;
    int playerPieces = 0; // cantidad de piezas del jugador
    // Cambia todas las piezas del jugador
    for(int i = 0 ; i < game.players[playerTurn].numPieces && game.bag.numPieces ; i++) {
        game.players[playerTurn].pieces[i] = game.bag.pieces[game.bag.numPieces - 1];
        game.bag.numPieces--;
        playerPieces++;
    }
    game.players[playerTurn].numPieces = playerPieces;
    game.skippedTurns++; // aumenta la cantidad de turnos saltados
    return game;
}

bool previewWord(Game game, Piece word[], int wordTam, int col, int row, char dir) {
    gameHeader(game);
    // Copia el tablero para no modificar el original
    Board previewBoard = copyBoard(game.board);
    // Coloca la palabra en el tablero de acuerdo a la dirección de forma temporal
    for (int i = 0; i < wordTam; i++) {
        if (dir == 'H') {
            previewBoard.pieces[row][col + i] = word[i];
        } else {
            previewBoard.pieces[row + i][col] = word[i];
        }
    }
    // Imprime el tablero con la palabra
    printCompactBoard(previewBoard);
    // Pregunta si desea jugar la palabra
    cout << endl << "Desea jugar esta palabra? (S/N): ";
    char option;
    cin >> option;
    getchar();
    return option == 'S' || option == 's';
}

Game putWord(Game game, string word, int col, int row, char dir){
    // Posición inicial de la palabra
    int initialCol = col;
    int initialRow = row;

    // Piezas de la palabra
    Piece pieces[word.length()];
    int piecesTam = wordToPieces(word, pieces);

    // Jugador en turno
    int playerTurn = game.turn%NUM_PLAYERS;
    Player player = game.players[playerTurn];

    // Pivotes de la palabra: piezas que estaban en el tablero antes de poner la palabra
    Piece pivots[piecesTam];

    // Poner la palabra en el tablero
    for(int i = 0 ; i < piecesTam ; i++) {
        Piece boardPiece = game.board.pieces[row][col];
        if(boardPiece.letter == "" || boardPiece.letter == "*") { // Verifica si la casilla está vacía
            // Obtener la pieza del jugador
            int playerPiecePos = pieceInArray(player.pieces, player.numPieces, pieces[i]);
            boardPiece = player.pieces[playerPiecePos];
            // Eliminar la pieza del portafichas del jugador
            player.numPieces = removePiece(player.pieces, player.numPieces, boardPiece);
            pivots[i].letter = "";
        }
        else {
            // Guardar la pieza que estaba en el tablero como pivote
            pivots[i] = boardPiece;
        }
        // Coloca la pieza en el tablero
        game.board.pieces[row][col] = boardPiece;

        if(dir == 'H') {
            col++;
        }
        else {
            row++;
        }
    }
    // Determina el inicio de la palabra
    int start;
    if(dir == 'H') {
        start = initialCol;
    }
    else {
        start = initialRow;
    }

    // Calcula los puntos de la palabra
    cout << "------------------------------------" << endl;
    cout << "PALABRA: " << word << endl;
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
    // Determina el inicio de la palabra de acuerdo con la dirección
    int points = 0;
    if(dir == 'H'){
        col = start;
    }
    else{
        row = start;
    }
    // Llega al final de la palabra calculando los puntos
    while(col < BOARD_SIZE && row < BOARD_SIZE && game.board.pieces[row][col].letter != ""){
        cout << game.board.pieces[row][col].letter << "+" << game.board.pieces[row][col].value << "  ";
        points += game.board.pieces[row][col].value; // Suma el valor de la letra
        if(dir == 'H'){
            col++; // Siguiente columna si dir es horizontal
        }
        else{
            row++; // Siguiente fila si dir es vertical
        }
    }
    cout << endl;
    return points; // Retorna los puntos de la palabra
}

int calculateAdyacentPoints(Game game, Piece pivots[], int wordTam, int col, int row, char dir){
    int points = 0; // Puntos de las palabras adyacentes
    for(int i = 0 ; i < wordTam ; i++){
        Piece adyacent[BOARD_SIZE]; // Piezas adyacentes
        int wordStart = 0;
        // Si la pieza no es un pivote
        if(pivots[i].letter == ""){
            // calcula los puntos de la palabra adyacente de acuerdo con la dirección
            if(dir == 'V'){
                // derecha e izquierda de la pieza para buscar la palabra adyacente y calcular los puntos
                if((col > 0 && game.board.pieces[row][col-1].letter != "") || (col < BOARD_SIZE-1 && game.board.pieces[row][col+1].letter != "")){
                    wordStart = getAdyacentWord(game, adyacent, col, row, 'H'); // Obtiene la palabra adyacente
                    points += wordPoints(game, wordStart, col, row, 'H'); // Calcula los puntos de la palabra adyacente
                }
            }
            else{
                // arriba y abajo de la pieza para buscar la palabra adyacente y calcular los puntos
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
    // Una vez detectada la palabra adyacente la arma y calcula los puntos
    int inf, sup, start;
    int adyacentTam = 0;
    // si la dirección es horizontal se busca la palabra adyacente en la columna
    // subiendo hasta encontrar una casilla vacía y bajando hasta encontrar una casilla vacía
    // formando el rango en el que se en cuentra la palabra para despues armarla y calcular los puntos
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
    cout << "------------------------------------" << endl;
    cout << "ADAYACENTE FORMADA: ";
    for(int i = 0 ; i < adyacentTam ; i++){
        cout << adyacent[i].letter;
    }
    cout << endl;
    return start;
}

Game givePlayerPieces(Game game){
    // Da las piezas gastadas al jugador si es posible
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
    // Diccionario extraido de https://github.com/kamilmielnik/scrabble-dictionaries
    ifstream file(string(FILE_PATH)+"fise-2.txt");
    bool exists = false;
    string line;
    // Busca en todas las letras del diccionario la que se ingresó
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
        cout << "La palabra " << word << " no es valida." << endl;
        cout << "------------------------------------" << endl;
        cout << "Presione enter para continuar..." << endl;
        getchar();
    }
    else {
        cout << "La palabra " << word << " es valida!" << endl;
        cout << "------------------------------------" << endl;
    }
    return exists;
}

bool validMove(Game game, Piece word[], int wordTam, int col, int row, char dir) {
    // Verifica si la jugada es válida según las reglas del juego
    bool isInBoard = (col >= 0 && col <= BOARD_SIZE) && (row >= 0 && row <= BOARD_SIZE); // Verifica si la posición está en el tablero
    bool validDirection = dir == 'H' || dir == 'V'; // Verifica si la dirección es válida
    // Show personalized error messages
    if(!isInBoard) {
        cout << "\tLa posicion ingresada no es valida." << endl;
    }
    if(!validDirection) {
        cout << "\tLa direccion ingresada no es valida." << endl;
    }

    bool noOverFlow = !overFlow(wordTam, dir, col, row); // Verifica si la palabra se sale del tablero
    bool noOverlapDifferent = !overlapDifferentLetter(game, word, wordTam, col, row, dir); // Verifica si la palabra se superpone con una letra diferente
    bool isInCenterFirstTurn = game.turn == 0 && passCenter(wordTam, col, row, dir) || game.turn != 0; // Verifica si la palabra pasa por el centro del tablero en el primer turno
    bool isConnected = wordIsConnected(game, wordTam, col, row, dir) && game.turn != 0 || game.turn == 0; // Verifica si la palabra está conectada a otra palabra
    bool hasNeededPieces = playerHasPieces(game, word, wordTam, col, row, dir); // Verifica si el jugador tiene las piezas necesarias
    bool validAdyacent = validAdyacentWords(game, word, wordTam, col, row, dir); // Verifica si las palabras adyacentes son válidas

    bool validMove = isInBoard && validDirection && noOverFlow && isInCenterFirstTurn && noOverlapDifferent &&
                     hasNeededPieces && validAdyacent && isConnected; // Verifica si la jugada es válida según las reglas del juego
    if(!validMove){
        cout << "------------------------------------" << endl;
        cout << "Esta palabra no se puede poner" << endl;
        cout << "------------------------------------" << endl;
        cout << "Presione enter para continuar..." << endl;
        getchar();
        printGameStatus(game);
    } else {
        cout << "------------------------------------" << endl;
        cout << "Esta palabra se puede poner" << endl;
        cout << "------------------------------------" << endl;
        cout << "Presione enter para continuar..." << endl;
        getchar();
    }
    return validMove;
}

bool overFlow(int wordTam, char dir, int col, int row){
    // Verifica si la palabra se sale del tablero
    if((dir == 'H' && col + wordTam <= BOARD_SIZE) || (dir == 'V' && row + wordTam <= BOARD_SIZE)){
        return false;
    }
    cout << "\tLa palabra no cabe en la posicion ingresada." << endl;
    return true;
}

bool passCenter(int wordTam, int col, int row, char dir) {
    // Verifica si alguna de las letras de la palabra pasa por el centro del tablero
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
    // Verifica si la palabra tiene como pivote una letra distinta a la que debe tener
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
    // Verifica si el jugador tiene las piezas necesarias para poner la palabra
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
        if (dir == 'V') {
            // Mira izquierda y derecha de la palabra para verificar si tiene adyacentes
            if ((col > 0 && game.board.pieces[row][col - 1].letter != "") ||
                (col < BOARD_SIZE - 1 && game.board.pieces[row][col + 1].letter != "")) {
                // cuando encuentra la adyacente verifica si es válida
                if (!validAdyacent(game, word[i].letter, col, row, 'H')) {
                    cout << "La palabra no tiene adyacentes validas" << endl;
                    return false;
                }
            }
            row++;
        } else {
            // Mira arriba y abajo de la palabra para verificar si tiene adyacentes
            if ((row > 0 && game.board.pieces[row - 1][col].letter != "") ||
                (row < BOARD_SIZE - 1 && game.board.pieces[row + 1][col].letter != "")) {
                // cuando encuentra la adyacente verifica si es válida
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

    // si la dirección es horizontal se busca la palabra adyacente en la columna
    // subiendo hasta encontrar una casilla vacía y bajando hasta encontrar una casilla vacía
    // formando el rango en el que se en cuentra la palabra para despues armarla y verificar si es valida en el diccionario
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
    cout << endl << "------------------------------------" << endl;
    cout << "ADYACENTE " << adyacent << endl;
    return validWord(adyacent);
}

bool wordIsConnected(Game game, int wordTam, int col, int row, char dir) {
    // Verifica si la palabra está conectada a otra palabra
    // Esto es: tiene al menos una letra en común con otra palabra
    // o está conectada a otra palabra formando una palabra válida
    for(int i = 0 ; i < wordTam ; i++) {
        if(game.board.pieces[row][col].letter != "" ||
           (row < BOARD_SIZE-1 && game.board.pieces[row+1][col].letter != "") ||
           (row > 0 && game.board.pieces[row-1][col].letter != "") ||
           (col < BOARD_SIZE-1 && game.board.pieces[row][col+1].letter != "") ||
           (col > 0 && game.board.pieces[row][col-1].letter != "")
           ) {
            return true;
        }
        if(dir == 'H') {
            col++;
        }
        else {
            row++;
        }
    }
    cout << "La palabra no esta conectada a otra palabra" << endl;
    return false;
}

/**********
 - UTILS  -
 **********/

int wordToPieces(string word, Piece pieces[]) {
    // Convierte la palabra en piezas teniendo en cuenta las letras especiales
    // LL, CH, RR
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
    // Copia el tablero
    Board newBoard;
    for (int i = 0; i < BOARD_SIZE; i++) {
        for (int j = 0; j < BOARD_SIZE; j++) {
            newBoard.pieces[i][j] = original.pieces[i][j];  // Copia profunda de cada pieza
        }
    }
    return newBoard;
}

int pieceInArray(Piece pieces[], int piecesTam, Piece piece){
    // Verifica si la pieza está en el arreglo
    for(int i = 0 ; i < piecesTam ; i++){
        if(pieces[i].letter == piece.letter){
            return i;
        }
    }
    return -1;
}

int removePiece(Piece pieces[], int piecesTam, Piece piece){
    // Elimina una pieza del arreglo del portafichas del jugador
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
 - Registry -
 **********/

void saveResults(Registry registry[]) {
    Registry playersRegistry[100];
    int numRegistries = readRegistry(playersRegistry);

    // Busca los jugadores y actualiza sus victorias y derrotas
    for (int i = 0 ; i < NUM_PLAYERS ; i++) {
        bool found = false;
        for (int j = 0 ; j < numRegistries ; j++) {
            if (registry[i].name == playersRegistry[j].name) {
                playersRegistry[j].wins += registry[i].wins;
                playersRegistry[j].losses += registry[i].losses;
                found = true;
            }
        }
        if (!found) {
            playersRegistry[numRegistries] = registry[i];
            numRegistries++;
        }
    }

    ofstream file(string(FILE_PATH) + "registry.txt");
    for (int i = 0; i < numRegistries; i++) {
        file << playersRegistry[i].name << endl;
        file << playersRegistry[i].wins << " " << playersRegistry[i].losses << endl;
    }

    file.close();
}

void showResults() {
    Registry playersRegistry[100];
    int numRegistries = readRegistry(playersRegistry);
    // print the registries
    cout << "------------------------------------" << endl;
    for(int i = 0; i < numRegistries; i++) {
        cout << "Jugador: " << playersRegistry[i].name << endl;
        cout << "Victorias: " << playersRegistry[i].wins << endl;
        cout << "Derrotas: " << playersRegistry[i].losses << endl;
        cout << "------------------------------------" << endl;
    }
}

int readRegistry(Registry playersRegistry[]) {
    int numRegistries = 0;
    ifstream file(string(FILE_PATH) + "registry.txt");
    if (!file.is_open()) {
        cout << "No hay registros de partidas" << endl;
        return 0;
    }

    string name;
    while (getline(file, name)) {
        Registry registry;
        registry.name = name;
        file >> registry.wins >> registry.losses;
        file.ignore();
        playersRegistry[numRegistries++] = registry;
    }

    file.close();
    return numRegistries;
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
                showResults();
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
        
    }
    
    cout << "------------------------------------" << endl;
    cout << "-       Bienvenido a SCRABBLE!     -" << endl;
    cout << "------------------------------------\n" << endl;
    string options[] = {
            "[1] Cambiar nombre de jugadores",
            "[2] Iniciar juego",
            "[3] Mostrar registro de resultados",
            "[4] Salir"
    };
    // Show menu
    for (int i = 0; i < 4; i++) {
        cout << "  " << options[i] << endl;
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
    int maxNameLength = 0;
    for (int i = 0; i < NUM_PLAYERS; i++) {
        if (players[i].name.length() > maxNameLength) {
            maxNameLength = players[i].name.length();
        }
    }

    // Print player's scores
    cout << "\t PUNTAJES" << endl;
    cout << "-------------------------------" << endl;
    for (int i = 0; i < NUM_PLAYERS; i++) {
        cout << "   " << setw(maxNameLength) << left << players[i].name << setw(5) << right << players[i].score << endl;
    }
    cout << "-------------------------------" << endl;
}

void gameHeader(Game game) {
    cout << "-------------------------------" << endl;
    cout << " Turno " << game.turn+1 << endl;
    cout << " Jugando " << game.players[game.turn%NUM_PLAYERS].name << endl;
    cout << " Piezas restantes: " << game.bag.numPieces << endl;
    cout << "-------------------------------" << endl;
}

void printGameStatus(Game game) {
    
    gameHeader(game);
    printPlayersScores(game.players);
    printCompactBoard(game.board);
}